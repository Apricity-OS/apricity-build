function assert(x) {
    if (!x) {
        throw new Error("assertion failed");
    }
    return x;
}
function as(cons, obj) {
    if (obj instanceof (cons)) {
        return obj;
    }
    throw new Error("Object " + obj + " is the wrong type");
}
/// <reference path="common.ts" />
var Logging;
(function (Logging) {
    // used elsewhere in the extension to enable additional safety
    // checks that "should never happen". Set to `true` when SHELLSHAPE_DEBUG=true|1|all
    Logging.PARANOID = false;
    var Lib = imports.misc.extensionUtils.getCurrentExtension().imports.lib;
    var log4js = Lib.log4javascript.log4javascript;
    function getLogger(name) {
        return log4js.getLogger(name);
    }
    Logging.getLogger = getLogger;
    ;
    function init(main) {
        var GLib = imports.gi.GLib;
        var root_logger = log4js.getLogger("shellshape");
        var GjsAppender = Lib.log4javascript_gjs_appender.init(log4js);
        var appender = new GjsAppender();
        appender.setLayout(new log4js.PatternLayout("%-5p: %m"));
        var shellshape_debug = GLib.getenv("SHELLSHAPE_DEBUG");
        var root_level = log4js.Level.INFO;
        root_logger.addAppender(appender);
        if (shellshape_debug) {
            var FileAppender = Lib.log4javascript_file_appender.init(log4js);
            if (main === true) {
                // only the main process should write shellshape.log
                // (prefs.js is loaded in a separate process, and we don't
                // want that to overwrite the real logs)
                var fileAppender = new FileAppender("/tmp/shellshape.log");
                fileAppender.setLayout(new log4js.PatternLayout("%d{HH:mm:ss,SSS} %-5p [%c]: %m"));
                root_logger.addAppender(fileAppender);
            }
            if (shellshape_debug == "true" || shellshape_debug == "all" || shellshape_debug == "1") {
                root_level = log4js.Level.DEBUG;
                Logging.PARANOID = true;
                root_logger.info("set log level DEBUG for shellshape.*");
                var NotificationAppender = function NotificationAppender() {
                };
                NotificationAppender.prototype = new log4js.Appender();
                NotificationAppender.prototype.layout = new log4js.PatternLayout("%c: %m");
                NotificationAppender.prototype.threshold = log4js.Level.ERROR;
                NotificationAppender.prototype.append = function (loggingEvent) {
                    var formattedMessage = FileAppender.getFormattedMessage(this, loggingEvent);
                    imports.ui.main.notify(formattedMessage);
                };
                var notificationAppender = new NotificationAppender();
                root_logger.addAppender(notificationAppender);
            }
            else {
                var debug_topics = shellshape_debug.split(",");
                debug_topics.map(function (topic) {
                    var log_name = "shellshape." + topic;
                    var logger = log4js.getLogger(log_name);
                    logger.setLevel(log4js.Level.DEBUG);
                    root_logger.info("set log level DEBUG for " + log_name);
                });
            }
        }
        root_logger.setLevel(root_level);
    }
    Logging.init = init;
})(Logging || (Logging = {}));
/// <reference path="common.ts" />
/// <reference path="logging.ts" />
// provides a few static utility methods for
// functions without access ot internal extension state
var Util;
(function (Util) {
    Util.log = Logging.getLogger('shellshape');
    // Utility function over GObject.connect(). Keeps track
    // of each added connection in `owner.bound_signals`,
    // for later cleanup in disconnect_tracked_signals().
    // Also logs any exceptions that occur.
    function connect_and_track(owner, subject, name, cb, after) {
        var method = after ? 'connect_after' : 'connect';
        owner.bound_signals.push({
            subject: subject,
            binding: subject[method](name, function () {
                var t = this;
                try {
                    return cb.apply(t, arguments);
                }
                catch (e) {
                    Util.log.error("Uncaught error in " + name + " signal handler: " + e + "\n" + e.stack);
                    throw e;
                }
            })
        });
    }
    Util.connect_and_track = connect_and_track;
    // Disconnect all tracked signals from the given object.
    // Used for reverting signals bound via `connect_and_track()`
    function disconnect_tracked_signals(owner, subject) {
        if (arguments.length > 1 && !subject) {
            throw new Error("disconnect_tracked_signals called with null subject");
        }
        var count = 0;
        for (var i = owner.bound_signals.length - 1; i >= 0; i--) {
            var sig = owner.bound_signals[i];
            if (subject == null || subject === sig.subject) {
                sig.subject.disconnect(sig.binding);
                // delete signal
                owner.bound_signals.splice(i, 1);
                count++;
            }
        }
        if (count > 0) {
            Util.log.debug("disconnected " + count + " listeners from " + owner + (subject == null ? "" : (" on " + subject)));
        }
    }
    Util.disconnect_tracked_signals = disconnect_tracked_signals;
    var _shell_version;
    function shell_version_gte(major, minor) {
        if (_shell_version == null) {
            var ver_string = imports.misc.config.PACKAGE_VERSION;
            _shell_version = ver_string.split('.').slice(0, 2).map(function (i) {
                return parseInt(i, 10);
            });
            if (_shell_version.length !== 2)
                throw new Error("can't parse shell version: " + ver_string);
            this.log.error("Parsed shell version: " + _shell_version.join("//"));
        }
        var required = [major, minor];
        for (var i = 0; i < required.length; i++) {
            if (_shell_version[i] > required[i]) {
                return true;
            }
            if (_shell_version[i] < required[i]) {
                return false;
            }
        }
        return true;
    }
    Util.shell_version_gte = shell_version_gte;
})(Util || (Util = {}));
/// <reference path="common.ts" />
/// <reference path="logging.ts" />
var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};
var Tiling;
(function (Tiling) {
    // external symbols (may or may not exist in a given env)
    Tiling.BORDER_RESIZE_INCREMENT = 0.05;
    Tiling.WINDOW_ONLY_RESIZE_INCREMENT = Tiling.BORDER_RESIZE_INCREMENT * 2;
    var Axis = {
        other: function (axis) {
            if (axis === 'y') {
                return 'x';
            }
            else {
                return 'y';
            }
        }
    };
    var j = function (s) {
        return JSON.stringify(s);
    };
    var HALF = 0.5;
    var STOP = '_stop_iter';
    Tiling.ArrayUtil = {
        divide_after: function (num, items) {
            return [items.slice(0, num), items.slice(num)];
        },
        moveItem: function (array, start, end) {
            var removed;
            removed = array.splice(start, 1)[0];
            array.splice(end, 0, removed);
            return array;
        }
    };
    var contains = function (arr, item) {
        return arr.indexOf(item) !== -1;
    };
    Tiling.get_mouse_position = function () {
        throw "override get_mouse_position()";
    };
    var Tile = (function () {
        function Tile() {
        }
        Tile.copy_rect = function (rect) {
            return {
                pos: {
                    x: rect.pos.x,
                    y: rect.pos.y
                },
                size: {
                    x: rect.size.x,
                    y: rect.size.y
                }
            };
        };
        Tile.split_rect = function (rect, axis, ratio, padding) {
            var new_rect, new_size_a, new_size_b;
            padding || (padding = 0);
            // log("#split_rect: splitting rect of " + j(rect) + " along the " + axis + " axis with ratio " + ratio)
            if (ratio > 1 || ratio < 0) {
                throw "invalid ratio: " + ratio + " (must be between 0 and 1)";
            }
            new_size_a = Math.round(rect.size[axis] * ratio);
            new_size_b = rect.size[axis] - new_size_a;
            padding = Math.round(Math.min(new_size_a / 2, new_size_b / 2, padding));
            // log("effective padding is " + padding)
            new_rect = Tile.copy_rect(rect);
            rect = Tile.copy_rect(rect);
            rect.size[axis] = new_size_a - padding;
            new_rect.size[axis] = new_size_b - padding;
            new_rect.pos[axis] += new_size_a + padding;
            // log("rect copy: " + j(rect))
            // log("new_rect: " + j(new_rect))
            return [rect, new_rect];
        };
        Tile.add_diff_to_rect = function (rect, diff) {
            return {
                pos: Tile.point_add(rect.pos, diff.pos),
                size: Tile.point_add(rect.size, diff.size)
            };
        };
        Tile.ensure_rect_exists = function (rect) {
            rect.size.x = Math.max(1, rect.size.x);
            rect.size.y = Math.max(1, rect.size.y);
            return rect;
        };
        Tile.zero_rect = function (rect) {
            return rect.pos.x === 0 && rect.pos.y === 0 && rect.size.x === 0 && rect.size.y === 0;
        };
        Tile.intersect = function (a, b) {
            if (a.pos.x + a.size.x < b.pos.x || a.pos.y + a.size.y < b.pos.y || b.pos.x + b.size.x < a.pos.x || b.pos.y + b.size.y < a.pos.y)
                return null;
            var xpos = Math.max(a.pos.x, b.pos.x);
            var ypos = Math.max(a.pos.y, b.pos.y);
            var w = Math.min(a.pos.x + a.size.x, b.pos.x + b.size.x) - xpos;
            var h = Math.min(a.pos.y + a.size.y, b.pos.y + b.size.y) - ypos;
            return {
                pos: { x: xpos, y: ypos },
                size: { x: w, y: h }
            };
        };
        Tile.shrink = function (rect, border_px) {
            return {
                pos: {
                    x: rect.pos.x + border_px,
                    y: rect.pos.y + border_px
                },
                size: {
                    x: Math.max(0, rect.size.x - (2 * border_px)),
                    y: Math.max(0, rect.size.y - (2 * border_px))
                }
            };
        };
        Tile.minmax = function (a, b) {
            return [Math.min(a, b), Math.max(a, b)];
        };
        Tile.midpoint = function (a, b) {
            var max, min, _ref;
            _ref = this.minmax(a, b), min = _ref[0], max = _ref[1];
            return Math.round(min + ((max - min) / 2));
        };
        Tile.within = function (val, a, b) {
            var mm = this.minmax(a, b);
            var min = mm[0];
            var max = mm[1];
            // log("val #{val} within #{min},#{max}? #{val > min && val < max}")
            return val > min && val < max;
        };
        Tile.move_rect_within = function (original_rect, bounds) {
            // log("moving #{j original_rect} to be within #{j bounds}")
            var extent, max, min, rect;
            min = Math.min;
            max = Math.max;
            rect = Tile.copy_rect(original_rect);
            rect.size.x = min(rect.size.x, bounds.size.x);
            rect.size.y = min(rect.size.y, bounds.size.y);
            rect.pos.x = max(rect.pos.x, bounds.pos.x);
            rect.pos.y = max(rect.pos.y, bounds.pos.y);
            extent = function (rect, axis) {
                return rect.pos[axis] + rect.size[axis];
            };
            rect.pos.x -= max(0, extent(rect, 'x') - extent(bounds, 'x'));
            rect.pos.y -= max(0, extent(rect, 'y') - extent(bounds, 'y'));
            return {
                pos: this.point_diff(original_rect.pos, rect.pos),
                size: this.point_diff(original_rect.size, rect.size)
            };
        };
        Tile.point_diff = function (a, b) {
            return {
                x: b.x - a.x,
                y: b.y - a.y
            };
        };
        Tile.point_add = function (a, b) {
            return {
                x: a.x + b.x,
                y: a.y + b.y
            };
        };
        Tile.rect_center = function (rect) {
            return {
                x: this.midpoint(rect.pos.x, rect.pos.x + rect.size.x),
                y: this.midpoint(rect.pos.y, rect.pos.y + rect.size.y)
            };
        };
        Tile.point_is_within = function (point, rect) {
            return this.within(point.x, rect.pos.x, rect.pos.x + rect.size.x) && this.within(point.y, rect.pos.y, rect.pos.y + rect.size.y);
        };
        Tile.point_eq = function (a, b) {
            return a.x === b.x && a.y === b.y;
        };
        Tile.rect_eq = function (a, b) {
            return (Tile.point_eq(a.pos, b.pos) && Tile.point_eq(a.size, b.size));
        };
        Tile.joinRects = function (a, b) {
            var pos, size, sx, sy;
            pos = {
                x: Math.min(a.pos.x, b.pos.x),
                y: Math.min(a.pos.y, b.pos.y)
            };
            sx = Math.max((a.pos.x + a.size.x) - pos.x, (b.pos.x + b.size.x) - pos.x);
            sy = Math.max((a.pos.y + a.size.y) - pos.y, (b.pos.y + b.size.y) - pos.y);
            size = {
                x: sx,
                y: sy
            };
            return {
                pos: pos,
                size: size
            };
        };
        return Tile;
    })();
    Tiling.Tile = Tile;
    var TileCollection = (function () {
        function TileCollection() {
            this.items = [];
            this.log = Logging.getLogger("shellshape.tiling.TileCollection");
            this.is_visible = function (item) {
                return !item.is_minimized();
            };
            this.is_minimized = function (item) {
                return item.is_minimized();
            };
            this.is_active = function (item) {
                return item.is_active();
            };
            this.most_recently_minimized = function (f) {
                var filtered, sorted;
                filtered = this.filter(this.is_minimized, this.items);
                if (filtered.length > 0) {
                    sorted = filtered.sort(function (a, b) {
                        return b.minimized_order - a.minimized_order;
                    });
                    f(sorted[0]);
                }
            };
            // provide ready-bound versions of any functions we need to use for filters:
            this.is_visible_and_untiled = Lang.bind(this, this._is_visible_and_untiled);
            this.is_tiled = Lang.bind(this, this._is_tiled);
        }
        TileCollection.prototype._is_visible_and_untiled = function (item) {
            return (!this.is_tiled(item)) && this.is_visible(item);
        };
        TileCollection.prototype._is_tiled = function (item) {
            return item.managed && this.is_visible(item);
        };
        TileCollection.prototype.sort_order = function (item) {
            if (this.is_tiled(item)) {
                return 0;
            }
            else if (this.is_visible(item)) {
                return 1;
            }
            else {
                return 2;
            }
        };
        TileCollection.prototype.sorted_with_indexes = function () {
            var self = this;
            var items_and_indexes = [];
            var ts = function () {
                return "" + this.item + "@" + this.index;
            };
            for (var index = 0; index < this.items.length; index++) {
                items_and_indexes.push({
                    item: this.items[index],
                    index: index,
                    toString: ts
                });
            }
            // this.log.debug("\nSORTING: #{j items_and_indexes}")
            var sorted = items_and_indexes.slice().sort(function (a, b) {
                var ordera, orderb;
                ordera = self.sort_order(a.item);
                orderb = self.sort_order(b.item);
                if (ordera === orderb) {
                    return a.index - b.index;
                }
                else {
                    // ensure a stable sort by using index position for equivalent windows
                    return ordera - orderb;
                }
            });
            // this.log.debug("sorted: #{items_and_indexes}\n    to: #{sorted}")
            return sorted;
        };
        TileCollection.prototype._wrap_index = function (idx, length) {
            while (idx < 0) {
                idx += length;
            }
            while (idx >= length) {
                idx -= length;
            }
            return idx;
        };
        TileCollection.prototype.filter = function (f, items) {
            var rv = [];
            for (var i = 0; i < items.length; i++) {
                var item = items[i];
                if (f(item)) {
                    rv.push(item);
                }
            }
            return rv;
        };
        TileCollection.prototype.select_cycle = function (diff) {
            var cycled = this._with_active_and_neighbor_when_filtered(this.is_visible, diff, function (active, neighbor) {
                neighbor.item.activate();
            });
            if (!cycled) {
                // no active window - just select the first visible window if there is one
                var filtered = this.filter(this.is_visible, this.items);
                if (filtered.length > 0) {
                    filtered[0].activate();
                }
            }
            return cycled;
        };
        TileCollection.prototype.sorted_view = function (filter) {
            return this.filter(function (obj) {
                return filter(obj.item);
            }, this.sorted_with_indexes());
        };
        TileCollection.prototype._with_active_and_neighbor_when_filtered = function (filter, diff, cb) {
            var self = this;
            var filtered = this.sorted_view(filter);
            var filtered_active_idx = this._index_where(filtered, function (obj) {
                return self.is_active(obj.item);
            });
            if (filtered_active_idx === null) {
                this.log.debug("active tile not found");
                return false;
            }
            var new_idx = this._wrap_index(filtered_active_idx + diff, filtered.length);
            this.log.debug("active tile found at index " + filtered_active_idx + ", neighbor idx = " + new_idx);
            cb(filtered[filtered_active_idx], filtered[new_idx]);
            return true;
        };
        TileCollection.prototype.cycle = function (diff) {
            // only one of these will have any effect, as the active tile is either tiled or untiled
            var self = this;
            var done = this._with_active_and_neighbor_when_filtered(this.is_tiled, diff, function (active, neighbor) {
                self.swap_at(active.index, neighbor.index);
            });
            if (!done) {
                self._with_active_and_neighbor_when_filtered(self.is_visible_and_untiled, diff, function (active, neighbor) {
                    self.swap_at(active.index, neighbor.index);
                });
            }
        };
        TileCollection.prototype._index_where = function (elems, cond) {
            for (var i = 0; i < elems.length; i++) {
                if (cond(elems[i])) {
                    return i;
                }
            }
            return null;
        };
        TileCollection.prototype._wrap_index_until = function (initial, offset, length, condition) {
            var index;
            index = initial;
            while (true) {
                index = this._wrap_index(index + offset, length);
                if (index === initial) {
                    // break cycle in single-element list
                    return initial;
                }
                else if (condition(index)) {
                    return index;
                }
            }
        };
        TileCollection.prototype.swap_at = function (idx1, idx2) {
            // @log.debug("swapping items at index #{idx1} and #{idx2}")
            var _orig;
            _orig = this.items[idx2];
            this.items[idx2] = this.items[idx1];
            return this.items[idx1] = _orig;
        };
        TileCollection.prototype.contains = function (item) {
            return this.indexOf(item) !== -1;
        };
        TileCollection.prototype.indexOf = function (item) {
            var id, idx, _this = this;
            id = item.id();
            idx = -1;
            this.each(function (tile, _idx) {
                if (tile.id() === id) {
                    _this.log.debug("found id " + id);
                    idx = _idx;
                    return STOP;
                }
                return null;
            });
            return idx;
        };
        TileCollection.prototype.push = function (item) {
            if (this.contains(item)) {
                return;
            }
            this.items.push(item);
        };
        TileCollection.prototype.each = function (f) {
            for (var i = 0; i < this.items.length; i++) {
                var ret = f(this.items[i], i);
                if (ret === STOP) {
                    return true;
                }
            }
            return false;
        };
        TileCollection.prototype.each_tiled = function (f) {
            this._filtered_each(this.is_tiled, f);
        };
        TileCollection.prototype._filtered_each = function (filter, f) {
            this.each(function (tile, idx) {
                if (filter(tile)) {
                    f(tile, idx);
                }
            });
        };
        TileCollection.prototype.active = function (f) {
            var self = this;
            this.each(function (item, idx) {
                if (self.is_active(item)) {
                    f(item, idx);
                    return STOP;
                }
                return null;
            });
        };
        TileCollection.prototype.for_layout = function () {
            // log.debug("tiles = #{@items}, filtered = #{@filter(@is_tiled, @items)}")
            return this.filter(this.is_tiled, this.items);
        };
        TileCollection.prototype.remove_at = function (idx) {
            return this.items.splice(idx, 1);
        };
        TileCollection.prototype.insert_at = function (idx, item) {
            return this.items.splice(idx, 0, item);
        };
        TileCollection.prototype.main = function (f) {
            var self = this;
            self.each(function (tile, idx) {
                if (self.is_tiled(tile)) {
                    f(tile, idx);
                    return STOP;
                }
                return null;
            });
        };
        return TileCollection;
    })();
    Tiling.TileCollection = TileCollection;
    var BaseSplit = (function () {
        function BaseSplit(axis) {
            this.log = Logging.getLogger("shellshape.tiling.BaseSplit");
            this.ratio = HALF;
            this.axis = axis;
        }
        BaseSplit.prototype.adjust_ratio = function (diff) {
            this.ratio = Math.min(1, Math.max(0, this.ratio + diff));
        };
        BaseSplit.prototype.save_last_rect = function (rect) {
            // log.debug("last_size changed from #{@last_size} -> #{rect.size[@axis]}")
            this.last_size = rect.size[this.axis];
        };
        BaseSplit.prototype.maintain_split_position_with_rect_difference = function (diff) {
            var unwanted_addition;
            unwanted_addition = this.ratio * diff;
            this.last_size += diff;
            this.log.debug("adjusting by " + (-unwanted_addition) + " to accommodate for rect size change from " + (this.last_size - diff) + " to " + this.last_size);
            this.adjust_ratio_px(-unwanted_addition);
        };
        BaseSplit.prototype.adjust_ratio_px = function (diff) {
            var current_px, new_px, new_ratio;
            this.log.debug("adjusting ratio " + this.ratio + " by " + diff + " px");
            if (diff === 0) {
                return;
            }
            current_px = this.ratio * this.last_size;
            this.log.debug("current ratio makes for " + current_px + " px (assuming last size of " + this.last_size);
            new_px = current_px + diff;
            this.log.debug("but we want " + new_px);
            new_ratio = new_px / this.last_size;
            if (!Tile.within(new_ratio, 0, 1)) {
                throw "failed ratio: " + new_ratio;
            }
            this.log.debug("which makes a new ratio of " + new_ratio);
            this.ratio = new_ratio;
        };
        return BaseSplit;
    })();
    Tiling.BaseSplit = BaseSplit;
    var Split = (function (_super) {
        __extends(Split, _super);
        function Split() {
            _super.apply(this, arguments);
        }
        Split.prototype.layout_one = function (rect, windows, padding) {
            var first_window, remaining, window_rect, _ref;
            this.save_last_rect(rect);
            first_window = windows.shift();
            if (windows.length === 0) {
                first_window.set_rect(rect);
                return [{}, []];
            }
            _ref = Tile.split_rect(rect, this.axis, this.ratio, padding), window_rect = _ref[0], remaining = _ref[1];
            first_window.set_rect(window_rect);
            return [remaining, windows];
        };
        Split.prototype.toString = function () {
            return "Split with ratio " + this.ratio;
        };
        return Split;
    })(BaseSplit);
    Tiling.Split = Split;
    var LayoutState = (function () {
        function LayoutState(bounds, tiles) {
            this.bounds = assert(bounds);
            this.tiles = tiles || new TileCollection();
            this.splits = {
                'x': {
                    main: new MultiSplit('x', 1),
                    minor: {
                        left: [],
                        right: []
                    }
                },
                'y': {
                    main: new MultiSplit('y', 1),
                    minor: {
                        left: [],
                        right: []
                    }
                }
            };
        }
        LayoutState.prototype.empty_copy = function () {
            return new LayoutState(this.bounds);
        };
        return LayoutState;
    })();
    Tiling.LayoutState = LayoutState;
    var MultiSplit = (function (_super) {
        __extends(MultiSplit, _super);
        function MultiSplit(axis, primary_windows) {
            _super.call(this, axis);
            this.log = Logging.getLogger("shellshape.tiling.MultiSplit");
            this.primary_windows = primary_windows;
        }
        MultiSplit.prototype.split = function (bounds, windows, padding) {
            var left_rect, left_windows, right_rect, right_windows, _ref, _ref1, _ref2;
            this.save_last_rect(bounds);
            // log.debug("mainsplit: dividing #{windows.length} after #{@primary_windows} for bounds #{j bounds}")
            _ref = this.partition_windows(windows), left_windows = _ref[0], right_windows = _ref[1];
            if (left_windows.length > 0 && right_windows.length > 0) {
                _ref1 = Tile.split_rect(bounds, this.axis, this.ratio, padding), left_rect = _ref1[0], right_rect = _ref1[1];
            }
            else {
                // only one side wil actually be laid out...
                _ref2 = [bounds, bounds], left_rect = _ref2[0], right_rect = _ref2[1];
            }
            return [[left_rect, left_windows], [right_rect, right_windows]];
        };
        MultiSplit.prototype.partition_windows = function (windows) {
            return Tiling.ArrayUtil.divide_after(this.primary_windows, windows);
        };
        MultiSplit.prototype.in_primary_partition = function (idx) {
            // @log.debug("on left? #{idx}, #{@primary_windows} == #{idx < @primary_windows}")
            return idx < this.primary_windows;
        };
        return MultiSplit;
    })(BaseSplit);
    Tiling.MultiSplit = MultiSplit;
    var BaseLayout = (function () {
        function BaseLayout(name, state) {
            this.padding = 0;
            this.log = Logging.getLogger("shellshape.tiling." + name);
            this.state = assert(state);
            this.tiles = state.tiles;
        }
        BaseLayout.prototype.toString = function () {
            return "[object BaseLayout]";
        };
        BaseLayout.prototype.layout = function (accommodate_window) {
            throw new Error("To be overridden");
        };
        BaseLayout.prototype.each = function (func) {
            return this.tiles.each(func);
        };
        BaseLayout.prototype.contains = function (win) {
            return this.tiles.contains(win);
        };
        BaseLayout.prototype.tile_for = function (win, func) {
            var self = this;
            if (!win) {
                self.log.warn("Layout.tile_for(null)");
                return false;
            }
            return this.tiles.each(function (tile, idx) {
                if (tile.window === win) {
                    func(tile, idx);
                    return STOP;
                }
                // self.log.warn("Layout.tile_for called on missing window: " + win);
                return null;
            });
        };
        BaseLayout.prototype.managed_tile_for = function (win, func) {
            // like @tile_for, but ignore floating windows
            var self = this;
            return this.tile_for(win, function (tile, idx) {
                if (self.tiles.is_tiled(tile)) {
                    func(tile, idx);
                }
            });
        };
        BaseLayout.prototype.tile = function (win) {
            var self = this;
            this.tile_for(win, function (tile) {
                tile.tile();
                self.layout();
            });
        };
        BaseLayout.prototype.select_cycle = function (offset) {
            return this.tiles.select_cycle(offset);
        };
        BaseLayout.prototype.add = function (win, active_win) {
            var self = this;
            var found, tile;
            if (this.contains(win)) {
                return false;
            }
            tile = new TiledWindow(win, this.state);
            found = this.tile_for(active_win, function (active_tile, active_idx) {
                self.tiles.insert_at(active_idx + 1, tile);
                self.log.debug("spliced " + tile + " into tiles at idx " + (active_idx + 1));
            });
            if (!found) {
                // no active tile, just add the new window at the end
                this.tiles.push(tile);
            }
            return true;
        };
        // spread(wins:Window[]) {
        // 	// move all windows in `wins` to minimize overlaps with other (tiled) windows
        // 	var self = this;
        // 	wins.forEach(function(win:Window) {
        // 		self.managed_tile_for(win, function(subject) {
        // 			self._each_tiled(function(avoid) {
        // 				if(subject === avoid) return;
        // 				// if there is overlay, move `subject` as much as it can / needs to to minimize overlap
        // 				
        // 			})
        // 		});
        // 	});
        // }
        BaseLayout.prototype.restore_original_positions = function () {
            // Sets all window positions back to original states.
            // NOTE: does _not_ actually release tiles, because
            // we may want to resume this state when the extension
            // gets re-enabled
            this.tiles.each_tiled(function (tile) {
                tile.window.move_resize(tile.original_rect);
            });
        };
        BaseLayout.prototype.active_tile = function (fn) {
            return this.tiles.active(fn);
        };
        BaseLayout.prototype.cycle = function (diff) {
            this.tiles.cycle(diff);
            return this.layout();
        };
        BaseLayout.prototype.minimize_window = function () {
            return this.active_tile(function (tile, idx) {
                return tile.minimize();
            });
        };
        BaseLayout.prototype.unminimize_last_window = function () {
            return this.tiles.most_recently_minimized(function (win) {
                // TODO: this is a little odd...
                //       we do a relayout() as a result of the unminimize, and this
                //       is the only way to make sure we don't activate the previously
                //       active window.
                return TiledWindow.with_active_window(win, function () {
                    win.unminimize();
                });
            });
        };
        BaseLayout.prototype.untile = function (win) {
            var self = this;
            this.tile_for(win, function (tile) {
                tile.release();
                self.layout();
            });
        };
        BaseLayout.prototype.on_window_killed = function (win) {
            var self = this;
            return this.tile_for(win, function (tile, idx) {
                self.tiles.remove_at(idx);
                self.layout();
            });
        };
        BaseLayout.prototype.toggle_maximize = function () {
            var self = this;
            var active = null;
            this.active_tile(function (tile, idx) {
                active = tile;
            });
            if (active === null) {
                this.log.debug("active == null");
            }
            if (active === null) {
                return;
            }
            this.each(function (tile) {
                if (tile === active) {
                    self.log.debug("toggling maximize for " + tile);
                    tile.toggle_maximize();
                }
                else {
                    tile.unmaximize();
                }
            });
        };
        BaseLayout.prototype.on_window_moved = function (win) {
            return this.on_window_resized(win);
        };
        BaseLayout.prototype.on_window_resized = function (win) {
            var self = this;
            var found = this.tile_for(win, function (tile, idx) {
                tile.update_original_rect();
                self.layout();
            });
            if (!found) {
                this.log.warn("couldn't find tile for window: " + win);
            }
        };
        // all the actions that are specific to an actual tiling layout are NOOP'd here,
        // so the keyboard handlers don't have to worry whether it's a valid thing to call
        BaseLayout.prototype.on_split_resize_start = function (win) {
            return null;
        };
        BaseLayout.prototype.adjust_splits_to_fit = function (win) {
            return null;
        };
        BaseLayout.prototype.add_main_window_count = function (i) {
            return null;
        };
        BaseLayout.prototype.adjust_main_window_area = function (diff) {
            return null;
        };
        BaseLayout.prototype.adjust_current_window_size = function (diff) {
            return null;
        };
        BaseLayout.prototype.scale_current_window = function (amount, axis) {
            return null;
        };
        BaseLayout.prototype.adjust_split_for_tile = function (opts) {
            return null;
        };
        BaseLayout.prototype.activate_main_window = function () {
            return null;
        };
        BaseLayout.prototype.swap_active_with_main = function () {
            return null;
        };
        return BaseLayout;
    })();
    Tiling.BaseLayout = BaseLayout;
    var FloatingLayout = (function (_super) {
        __extends(FloatingLayout, _super);
        function FloatingLayout(state) {
            _super.call(this, 'FloatingLayout', state);
        }
        FloatingLayout.prototype.toString = function () {
            return "[object FloatingLayout]";
        };
        FloatingLayout.prototype.layout = function (accommodate_window) {
            var self = this;
            this.each(function (tile) {
                self.log.debug("resetting window state...");
                tile.resume_original_state();
                return tile.layout();
            });
            // now don't bother laying out anything again!
            this.layout = function (accommodate_window) {
            };
        };
        return FloatingLayout;
    })(BaseLayout);
    Tiling.FloatingLayout = FloatingLayout;
    var FullScreenLayout = (function (_super) {
        __extends(FullScreenLayout, _super);
        function FullScreenLayout(state) {
            _super.call(this, 'FullScreenLayout', state);
        }
        FullScreenLayout.prototype.toString = function () {
            return "[object FullScreenLayout]";
        };
        FullScreenLayout.prototype.layout = function (accommodate_window) {
            this.each(function (tile) {
                return tile.window.maximize();
            });
            return this.layout;
        };
        return FullScreenLayout;
    })(BaseLayout);
    Tiling.FullScreenLayout = FullScreenLayout;
    var BaseTiledLayout = (function (_super) {
        __extends(BaseTiledLayout, _super);
        // private split_resize_start_rect: Rect = null
        function BaseTiledLayout(name, axis, state) {
            _super.call(this, name, state);
            // TODO: remove need for these instance vars
            this.main_axis = axis;
            this.bounds = state.bounds;
            this.main_split = state.splits[this.main_axis].main;
            this.splits = state.splits[this.main_axis].minor;
        }
        BaseTiledLayout.prototype.toString = function () {
            return "[object BaseTiledLayout]";
        };
        BaseTiledLayout.prototype._each_tiled = function (func) {
            return this.tiles.each_tiled(func);
        };
        BaseTiledLayout.prototype.layout = function (accommodate_window) {
            this.bounds.update();
            var padding = this.padding;
            var layout_windows = this.tiles.for_layout();
            this.log.debug("laying out " + layout_windows.length + " windows");
            if (accommodate_window != null) {
                this._change_main_ratio_to_accommodate(accommodate_window, this.main_split);
            }
            var _ref = this.main_split.split(this.bounds, layout_windows, padding);
            var left = _ref[0];
            var right = _ref[1];
            // @log.debug("split screen into rect #{j left[0]} | #{j right[0]}")
            this._layout_side.apply(this, left.concat([this.splits.left, accommodate_window, padding]));
            this._layout_side.apply(this, right.concat([this.splits.right, accommodate_window, padding]));
        };
        BaseTiledLayout.prototype._layout_side = function (rect, windows, splits, accommodate_window, padding) {
            var accommodate_idx, axis, bottom_split, extend_to, other_axis, previous_split, split, top_splits, window, zip, _i, _len, _ref, _ref1, _ref2, _results;
            axis = Axis.other(this.main_axis);
            extend_to = function (size, array, generator) {
                var _results;
                _results = [];
                while (array.length < size) {
                    _results.push(array.push(generator()));
                }
                return _results;
            };
            zip = function (a, b) {
                var i;
                return (function () {
                    var _i, _ref, _results;
                    _results = [];
                    for (i = _i = 0, _ref = Math.min(a.length, b.length); 0 <= _ref ? _i < _ref : _i > _ref; i = 0 <= _ref ? ++_i : --_i) {
                        _results.push([a[i], b[i]]);
                    }
                    return _results;
                })();
            };
            extend_to(windows.length, splits, function () {
                return new Split(axis);
            });
            // @log.debug("laying out side with rect #{j rect}, windows #{windows.length} and splits #{splits.length}")
            if (accommodate_window != null) {
                accommodate_idx = windows.indexOf(accommodate_window);
                if (accommodate_idx !== -1) {
                    top_splits = splits.slice(0, accommodate_idx);
                    bottom_split = splits[accommodate_idx];
                    if (accommodate_idx === windows.length - 1) {
                        bottom_split = void 0;
                    }
                    other_axis = Axis.other(this.main_axis);
                    this._change_minor_ratios_to_accommodate(accommodate_window, top_splits, bottom_split);
                }
            }
            previous_split = null;
            _ref = zip(windows, splits);
            _results = [];
            for (_i = 0, _len = _ref.length; _i < _len; _i++) {
                _ref1 = _ref[_i], window = _ref1[0], split = _ref1[1];
                window.top_split = previous_split;
                _ref2 = split.layout_one(rect, windows, padding), rect = _ref2[0], windows = _ref2[1];
                window.ensure_within(this.bounds);
                window.bottom_split = windows.length > 0 ? split : null;
                _results.push(previous_split = split);
            }
            return _results;
        };
        BaseTiledLayout.prototype.add_main_window_count = function (i) {
            this.main_split.primary_windows += i;
            return this.layout();
        };
        BaseTiledLayout.prototype.adjust_main_window_area = function (diff) {
            this.main_split.adjust_ratio(diff);
            return this.layout();
        };
        BaseTiledLayout.prototype.adjust_current_window_size = function (diff) {
            var self = this;
            return this.active_tile(function (tile) {
                self.adjust_split_for_tile({
                    tile: tile,
                    diff_ratio: diff,
                    axis: Axis.other(self.main_axis)
                });
                self.layout();
            });
        };
        BaseTiledLayout.prototype.scale_current_window = function (amount, axis) {
            var bounds = this.bounds;
            this.active_tile(function (tile) {
                tile.scale_by(amount, axis);
                tile.center_window();
                tile.ensure_within(bounds);
                tile.layout();
            });
        };
        BaseTiledLayout.prototype.adjust_split_for_tile = function (opts) {
            var adjust, axis, diff_px, diff_ratio, tile;
            axis = opts.axis, diff_px = opts.diff_px, diff_ratio = opts.diff_ratio, tile = opts.tile;
            adjust = function (split, inverted) {
                if (diff_px != null) {
                    split.adjust_ratio_px(inverted ? -diff_px : diff_px);
                }
                else {
                    split.adjust_ratio(inverted ? -diff_ratio : diff_ratio);
                }
            };
            if (axis === this.main_axis) {
                adjust(this.main_split, !this.main_split.in_primary_partition(this.tiles.indexOf(tile)));
            }
            else {
                if (tile.bottom_split != null) {
                    adjust(tile.bottom_split, false);
                }
                else if (tile.top_split != null) {
                    adjust(tile.top_split, true);
                }
            }
        };
        BaseTiledLayout.prototype.activate_main_window = function () {
            var _this = this;
            this.tiles.main(function (win) {
                win.activate();
            });
        };
        BaseTiledLayout.prototype.swap_active_with_main = function () {
            var _this = this;
            this.tiles.active(function (tile, idx) {
                _this.tiles.main(function (main_tile, main_idx) {
                    _this.tiles.swap_at(idx, main_idx);
                    _this.layout();
                });
            });
        };
        BaseTiledLayout.prototype.on_window_moved = function (win) {
            var self = this;
            this.tile_for(win, function (tile, idx) {
                var moved;
                moved = false;
                if (tile.managed) {
                    moved = self._swap_moved_tile_if_necessary(tile, idx);
                }
                if (!moved) {
                    tile.update_offset();
                }
                self.layout();
            });
        };
        // on_split_resize_start(win) {
        //	 TODO: this is never called in mutter
        //	 this.split_resize_start_rect = Tile.copy_rect(this.tiles[this.indexOf(win)].window_rect());
        //	 return this.log.debug("starting resize of split.. " + (j(this.split_resize_start_rect)));
        // }
        BaseTiledLayout.prototype.on_window_resized = function (win) {
            var self = this;
            this.managed_tile_for(win, function (tile, idx) {
                var diff;
                // TODO: uncomment when on_split_resize_start is used
                // if (self.split_resize_start_rect != null) {
                //	 diff = Tile.point_diff(self.split_resize_start_rect.size, tile.window_rect().size);
                //	 self.log.debug("split resized! diff = " + (j(diff)));
                //	 if (diff.x !== 0) {
                //		 self.adjust_split_for_tile({
                //			 tile: tile,
                //			 diff_px: diff.x,
                //			 axis: 'x'
                //		 });
                //	 }
                //	 if (diff.y !== 0) {
                //		 self.adjust_split_for_tile({
                //			 tile: tile,
                //			 diff_px: diff.y,
                //			 axis: 'y'
                //		 });
                //	 }
                //	 self.split_resize_start_rect = null;
                // } else {
                tile.update_offset();
                // }
                self.layout();
                return true;
            });
        };
        BaseTiledLayout.prototype.adjust_splits_to_fit = function (win) {
            var self = this;
            this.managed_tile_for(win, function (tile, idx) {
                if (!self.tiles.is_tiled(tile))
                    return;
                self.layout(tile);
            });
        };
        BaseTiledLayout.prototype._change_main_ratio_to_accommodate = function (tile, split) {
            var left, right, _ref;
            _ref = split.partition_windows(this.tiles.for_layout()), left = _ref[0], right = _ref[1];
            if (contains(left, tile)) {
                this.log.debug("LHS adjustment for size: " + (j(tile.offset.size)) + " and pos " + (j(tile.offset.pos)));
                split.adjust_ratio_px(tile.offset.size[this.main_axis] + tile.offset.pos[this.main_axis]);
                tile.offset.size[this.main_axis] = -tile.offset.pos[this.main_axis];
            }
            else if (contains(right, tile)) {
                this.log.debug("RHS adjustment for size: " + (j(tile.offset.size)) + " and pos " + (j(tile.offset.pos)));
                split.adjust_ratio_px(tile.offset.pos[this.main_axis]);
                tile.offset.size[this.main_axis] += tile.offset.pos[this.main_axis];
                tile.offset.pos[this.main_axis] = 0;
            }
            this.log.debug("After main_split accommodation, tile offset = " + (j(tile.offset)));
        };
        BaseTiledLayout.prototype._change_minor_ratios_to_accommodate = function (tile, above_splits, below_split) {
            var axis, bottom_offset, diff_px, diff_pxes, i, offset, proportion, size_taken, split, split_size, split_sizes, top_offset, total_size_above, _i, _j, _k, _len, _ref, _ref1;
            offset = tile.offset;
            axis = Axis.other(this.main_axis);
            top_offset = offset.pos[axis];
            bottom_offset = offset.size[axis];
            if (above_splits.length > 0) {
                // TODO: this algorithm seems needlessly involved. Figure out if there's a cleaner
                //       way of doing it
                this.log.debug("ABOVE adjustment for offset: " + (j(offset)) + ", " + top_offset + " diff required across " + above_splits.length);
                diff_pxes = [];
                split_sizes = [];
                total_size_above = 0;
                for (_i = 0, _len = above_splits.length; _i < _len; _i++) {
                    split = above_splits[_i];
                    split_size = split.last_size * split.ratio;
                    split_sizes.push(split_size);
                    total_size_above += split_size;
                }
                for (i = _j = 0, _ref = above_splits.length; 0 <= _ref ? _j < _ref : _j > _ref; i = 0 <= _ref ? ++_j : --_j) {
                    proportion = split_sizes[i] / total_size_above;
                    diff_pxes.push(proportion * top_offset);
                }
                this.log.debug("diff pxes for above splits are: " + (j(diff_pxes)));
                size_taken = 0;
                for (i = _k = 0, _ref1 = above_splits.length; 0 <= _ref1 ? _k < _ref1 : _k > _ref1; i = 0 <= _ref1 ? ++_k : --_k) {
                    split = above_splits[i];
                    diff_px = diff_pxes[i];
                    split.maintain_split_position_with_rect_difference(-size_taken);
                    size_taken += diff_px;
                    split.adjust_ratio_px(diff_px);
                }
                tile.offset.pos[axis] = 0;
                if (below_split != null) {
                    this.log.debug("MODIFYING bottom to accomodate top_px changes == " + top_offset);
                    // TODO: seems a pretty hacky place to do it..
                    below_split.maintain_split_position_with_rect_difference(-top_offset);
                }
                else {
                    tile.offset.size[axis] += top_offset;
                }
            }
            else {
                bottom_offset += top_offset;
            }
            if (below_split != null) {
                this.log.debug("BELOW adjustment for offset: " + (j(offset)) + ", bottom_offset = " + bottom_offset);
                this.log.debug("before bottom minor adjustments, offset = " + (j(tile.offset)));
                below_split.adjust_ratio_px(bottom_offset);
                tile.offset.size[axis] -= bottom_offset;
            }
            this.log.debug("After minor adjustments, offset = " + (j(tile.offset)));
        };
        BaseTiledLayout.prototype._swap_moved_tile_if_necessary = function (tile, idx) {
            var self = this;
            var moved = false;
            if (this.tiles.is_tiled(tile)) {
                var mouse_pos = Tiling.get_mouse_position();
                this._each_tiled(function (swap_candidate, swap_idx) {
                    var target_rect;
                    target_rect = Tile.shrink(swap_candidate.rect, 20);
                    if (swap_idx === idx) {
                        return null;
                    }
                    if (Tile.point_is_within(mouse_pos, target_rect)) {
                        self.log.debug("swapping idx " + idx + " and " + swap_idx);
                        self.tiles.swap_at(idx, swap_idx);
                        moved = true;
                        return STOP;
                    }
                    return null;
                });
            }
            return moved;
        };
        return BaseTiledLayout;
    })(BaseLayout);
    Tiling.BaseTiledLayout = BaseTiledLayout;
    var VerticalTiledLayout = (function (_super) {
        __extends(VerticalTiledLayout, _super);
        function VerticalTiledLayout(state) {
            _super.call(this, 'VerticalTiledLayout', 'x', state);
        }
        VerticalTiledLayout.prototype.toString = function () {
            return "[object VerticalTiledLayout]";
        };
        return VerticalTiledLayout;
    })(BaseTiledLayout);
    Tiling.VerticalTiledLayout = VerticalTiledLayout;
    var HorizontalTiledLayout = (function (_super) {
        __extends(HorizontalTiledLayout, _super);
        function HorizontalTiledLayout(state) {
            _super.call(this, 'HorizontalTiledLayout', 'y', state);
        }
        HorizontalTiledLayout.prototype.toString = function () {
            return "[object HorizontalTiledLayout]";
        };
        return HorizontalTiledLayout;
    })(BaseTiledLayout);
    Tiling.HorizontalTiledLayout = HorizontalTiledLayout;
    var TiledWindow = (function () {
        function TiledWindow(win, state) {
            this.maximized = false;
            this.managed = false;
            this._was_minimized = false;
            this.minimized_order = 0;
            this.update_original_rect = function () {
                this.original_rect = this.window.rect();
                this.log.debug("window " + this + " remembering new rect of " + (JSON.stringify(this.original_rect)));
            };
            this.log = Logging.getLogger("shellshape.tiling.TiledWindow");
            this.window = win;
            this.bounds = state.bounds;
            this.maximized = false;
            this.managed = false;
            this._was_minimized = false;
            this.minimized_order = 0;
            this.rect = {
                pos: {
                    x: 0,
                    y: 0
                },
                size: {
                    x: 0,
                    y: 0
                }
            };
            this.update_original_rect();
            this.reset_offset();
        }
        TiledWindow.with_active_window = function (win, f) {
            var _old = TiledWindow.active_window_override;
            TiledWindow.active_window_override = win;
            try {
                f();
            }
            finally {
                TiledWindow.active_window_override = _old;
            }
        };
        TiledWindow.prototype.id = function () {
            return this.window.id();
        };
        TiledWindow.prototype.resume_original_state = function () {
            this.reset_offset();
            this.rect = Tile.copy_rect(this.original_rect);
            this.log.debug("window " + this + " resuming old rect of " + (JSON.stringify(this.rect)));
        };
        TiledWindow.prototype.tile = function () {
            if (this.managed) {
                this.log.debug("resetting offset for window " + this);
            }
            else {
                this.managed = true;
                this.window.set_tile_preference(true);
                this.original_rect = this.window.rect();
            }
            this.reset_offset();
        };
        TiledWindow.prototype.reset_offset = function () {
            this.offset = {
                pos: {
                    x: 0,
                    y: 0
                },
                size: {
                    x: 0,
                    y: 0
                }
            };
        };
        TiledWindow.prototype.toString = function () {
            return "<\#TiledWindow of " + this.window.toString() + ">";
        };
        TiledWindow.prototype.update_offset = function () {
            var rect, win;
            rect = this.rect;
            win = this.window.rect();
            this.offset = {
                pos: Tile.point_diff(rect.pos, win.pos),
                size: Tile.point_diff(rect.size, win.size)
            };
            this.log.debug("updated tile offset to " + (j(this.offset)));
        };
        TiledWindow.prototype.toggle_maximize = function () {
            if (this.maximized) {
                this.unmaximize();
            }
            else {
                this.maximize();
            }
        };
        TiledWindow.prototype.is_minimized = function () {
            var min;
            min = this.window.is_minimized();
            if (min && !this._was_minimized) {
                // the window with the highest minimise order is the most-recently minimized
                this.minimized_order = TiledWindow.minimized_counter++;
            }
            this._was_minimized = min;
            return min;
        };
        TiledWindow.prototype.maximize = function () {
            if (!this.maximized) {
                this.maximized = true;
                this.update_offset();
                this.layout();
            }
        };
        TiledWindow.prototype.unmaximize = function () {
            if (this.maximized) {
                this.maximized = false;
                if (!this.managed) {
                    this.log.debug("unmaximize caused layout()");
                }
                this.layout();
            }
        };
        TiledWindow.prototype.unminimize = function () {
            this.window.unminimize();
        };
        TiledWindow.prototype.minimize = function () {
            this.window.minimize();
        };
        TiledWindow.prototype._resize = function (size) {
            this.rect.size = {
                x: size.x,
                y: size.y
            };
        };
        TiledWindow.prototype._move = function (pos) {
            this.rect.pos = {
                x: pos.x,
                y: pos.y
            };
        };
        TiledWindow.prototype.set_rect = function (r) {
            // log("offset rect to " + j(@offset))
            // @log.debug("tile has new rect: " + j(r))
            this._resize(r.size);
            this._move(r.pos);
            this.layout();
        };
        TiledWindow.prototype.ensure_within = function (screen_rect) {
            var change_required, combined_rect;
            combined_rect = Tile.add_diff_to_rect(this.rect, this.offset);
            change_required = Tile.move_rect_within(combined_rect, screen_rect);
            if (!Tile.zero_rect(change_required)) {
                this.log.debug("moving tile " + (j(change_required)) + " to keep it onscreen");
                this.offset = Tile.add_diff_to_rect(this.offset, change_required);
                this.layout();
            }
        };
        TiledWindow.prototype.center_window = function () {
            var tile_center = Tile.rect_center(this.rect);
            var window_center = Tile.rect_center(this.window.rect());
            var movement_required = Tile.point_diff(window_center, tile_center);
            this.offset.pos = Tile.point_add(this.offset.pos, movement_required);
        };
        TiledWindow.prototype.layout = function () {
            var is_active;
            if (TiledWindow.active_window_override) {
                is_active = TiledWindow.active_window_override === this;
            }
            else {
                is_active = this.is_active();
            }
            this.window.move_resize(this.active_rect());
            if (is_active) {
                this.window.activate_before_redraw("layout");
            }
        };
        TiledWindow.prototype.active_rect = function () {
            // returns the currently active rect for the window, including
            //  - maximize state
            //  - non-zero rect
            //  - tile rect + user-controlled offset
            var rect = (this.maximized ? Tile.shrink(this.bounds, 20) : Tile.add_diff_to_rect(this.rect, this.offset));
            return Tile.ensure_rect_exists(rect);
        };
        TiledWindow.prototype.scale_by = function (amount, axis) {
            var window_rect = this.window.rect();
            if (axis != null) {
                this._scale_by(amount, axis, window_rect);
            }
            else {
                // scale in both directions
                this._scale_by(amount, 'x', window_rect);
                this._scale_by(amount, 'y', window_rect);
            }
        };
        TiledWindow.prototype._scale_by = function (amount, axis, window_rect) {
            var current_dim, diff_px, new_dim;
            current_dim = window_rect.size[axis];
            diff_px = amount * current_dim;
            new_dim = current_dim + diff_px;
            this.offset.size[axis] += diff_px;
            this.offset.pos[axis] -= diff_px / 2;
        };
        TiledWindow.prototype.release = function () {
            this.set_rect(this.original_rect);
            this.managed = false;
            this.window.set_tile_preference(false);
        };
        TiledWindow.prototype.activate = function () {
            this.window.activate();
        };
        TiledWindow.prototype.is_active = function () {
            return this.window.is_active();
        };
        TiledWindow.minimized_counter = 0;
        TiledWindow.active_window_override = null;
        return TiledWindow;
    })();
    Tiling.TiledWindow = TiledWindow;
})(Tiling || (Tiling = {}));
/// <reference path="common.ts" />
var Indicator;
(function (Indicator) {
    var Lang = imports.lang;
    var PanelMenu = imports.ui.panelMenu;
    var PopupMenu = imports.ui.popupMenu;
    var St = imports.gi.St;
    var Clutter = imports.gi.Clutter;
    var Shell = imports.gi.Shell;
    var Main = imports.ui.main;
    var Ext = imports.misc.extensionUtils.getCurrentExtension();
    var Gio = imports.gi.Gio;
    var _indicator;
    // A BIT HACKY: add the shellshape icon directory to the current theme's search path,
    // as this seems to be the only way to get symbolic icons loading properly.
    (function () {
        var theme = imports.gi.Gtk.IconTheme.get_default();
        var icon_dir = Ext.dir.get_child('data').get_child('icons');
        if (icon_dir.query_exists(null)) {
            global.log("adding icon dir: " + icon_dir.get_path());
            theme.append_search_path(icon_dir.get_path());
        }
        else {
            global.log("no icon dir found at " + icon_dir.get_path() + " - assuming globally installed");
        }
    })();
    var PopupImageMenuItem = (function () {
        function PopupImageMenuItem(label, icon, params) {
            this._init.apply(this, arguments);
        }
        PopupImageMenuItem.prototype._init = function (text, iconName, params) {
            PopupMenu.PopupBaseMenuItem.prototype._init.call(this, params);
            this.label = new St.Label({
                text: text
            });
            this._icon = new St.Icon({
                style_class: 'system-status-icon'
            });
            this.actor.add(this._icon, { align: St.Align.START });
            this.actor.add(this.label);
            this.setIcon(iconName);
        };
        PopupImageMenuItem.prototype.setIcon = function (name) {
            this._icon.icon_name = name;
        };
        return PopupImageMenuItem;
    })();
    ;
    PopupImageMenuItem.prototype.__proto__ = PopupMenu.PopupBaseMenuItem.prototype;
    var ShellshapeIndicator = (function () {
        function ShellshapeIndicator(ext) {
            this.bound_signals = [];
            this._init.apply(this, arguments);
        }
        ShellshapeIndicator.enable = function (ext) {
            _indicator = new ShellshapeIndicator(ext);
            Main.panel.addToStatusArea('shellshape-indicator', _indicator);
        };
        ShellshapeIndicator.disable = function () {
            _indicator.disable();
            _indicator = undefined;
        };
        ShellshapeIndicator.prototype._init = function (ext) {
            var self = this;
            this.log = Logging.getLogger("shellshape.indicator");
            this.ext = ext;
            PanelMenu.Button.prototype._init.call(this, 0.0, 'Shellshape Layout', false);
            // create menu
            this.menu_entries = [
                {
                    label: 'Floating',
                    layout: Tiling.FloatingLayout,
                    icon: 'window-tile-floating-symbolic'
                },
                {
                    label: 'Vertical',
                    layout: Tiling.VerticalTiledLayout,
                    icon: 'window-tile-vertical-symbolic'
                },
                {
                    label: 'Horizontal',
                    layout: Tiling.HorizontalTiledLayout,
                    icon: 'window-tile-horizontal-symbolic'
                },
                {
                    label: 'Full Screen',
                    layout: Tiling.FullScreenLayout,
                    icon: 'window-tile-full-symbolic'
                }
            ];
            var items = new PopupMenu.PopupMenuSection();
            (function () {
                for (var i = 0; i < self.menu_entries.length; i++) {
                    (function (item_props) {
                        var item = new PopupImageMenuItem(item_props.label, item_props.icon);
                        items.addMenuItem(item);
                        item.connect('activate', function () {
                            self.log.debug("callback for [" + item_props.label + "] received by " + self);
                            self._set_active_item(item_props);
                            self._current_workspace().set_layout(item_props.layout);
                        });
                    })(self.menu_entries[i]);
                }
            })();
            items.addMenuItem(new PopupMenu.PopupSeparatorMenuItem());
            (function () {
                var item = new PopupMenu.PopupMenuItem("Shellshape Settings");
                item.connect('activate', function () {
                    var uuid = "shellshape@gfxmonk.net";
                    var appSys = Shell.AppSystem.get_default();
                    var app = appSys.lookup_app('gnome-shell-extension-prefs.desktop');
                    var info = app.get_app_info();
                    var timestamp = global.display.get_current_time_roundtrip();
                    info.launch_uris(['extension:///' + uuid], global.create_app_launch_context(timestamp, -1));
                });
                items.addMenuItem(item);
            })();
            this.menu.addMenuItem(items);
            var default_entry = this.menu_entries[0];
            this.icon = new St.Icon({
                icon_name: default_entry.icon,
                style_class: 'system-status-icon'
            });
            this.actor.get_children().forEach(function (c) {
                c.destroy();
            });
            this.actor.add_actor(this.icon);
            this.actor.connect('scroll-event', Lang.bind(this, this._scroll_event));
            this._workspaceChanged(null, null, global.screen.get_active_workspace_index());
            Util.connect_and_track(this, global.screen, 'workspace-switched', Lang.bind(this, this._workspaceChanged), true);
            Util.connect_and_track(this, this.ext, 'layout-changed', Lang.bind(this, this._update_indicator));
        };
        ShellshapeIndicator.prototype.disable = function () {
            Util.disconnect_tracked_signals(this);
            this.destroy(); // indicator method
        };
        ShellshapeIndicator.prototype.toString = function () {
            return "<ShellshapeIndicator>";
        };
        ShellshapeIndicator.prototype._scroll_event = function (actor, event) {
            var self = this;
            var direction = event.get_scroll_direction();
            var diff = 0;
            if (direction == Clutter.ScrollDirection.DOWN) {
                diff = 1;
            }
            else if (direction == Clutter.ScrollDirection.UP) {
                diff = -1;
            }
            else {
                return;
            }
            this._active_item(function (item, idx) {
                var new_item = self.menu_entries[idx + diff];
                if (new_item == null)
                    return;
                self._set_active_item(new_item);
                self._current_workspace().set_layout(new_item.layout);
            });
        };
        ShellshapeIndicator.prototype._set_active_item = function (item) {
            this.icon.set_icon_name(item.icon);
        };
        ShellshapeIndicator.prototype._workspaceChanged = function (meta_screen, old_index, new_index) {
            this._update_indicator();
        };
        ShellshapeIndicator.prototype._active_item = function (cb) {
            // find the active menu item for the current layout on the current workspace
            var layout_cls = this._current_workspace().active_layout;
            for (var i = 0; i < this.menu_entries.length; i++) {
                var entry = this.menu_entries[i];
                if (entry.layout == layout_cls) {
                    cb.call(this, entry, i);
                    break;
                }
            }
        };
        ShellshapeIndicator.prototype._update_indicator = function () {
            var self = this;
            var item_props = null;
            this._active_item(function (item) {
                self._set_active_item(item);
            });
        };
        ShellshapeIndicator.prototype._current_workspace = function () {
            return this.ext.current_workspace();
        };
        return ShellshapeIndicator;
    })();
    Indicator.ShellshapeIndicator = ShellshapeIndicator;
    ShellshapeIndicator.prototype.__proto__ = PanelMenu.Button.prototype;
})(Indicator || (Indicator = {}));
/// <reference path="common.ts" />
/// <reference path="logging.ts" />
/// <reference path="tiling.ts" />
/// <reference path="util.ts" />
var MutterWindow;
(function (MutterWindow) {
    var Main = imports.ui.main;
    var Lang = imports.lang;
    var Meta = imports.gi.Meta;
    var Shell = imports.gi.Shell;
    var WindowProperties = (function () {
        function WindowProperties() {
        }
        WindowProperties.id = function (w) {
            if (!w || !w.get_stable_sequence) {
                Util.log.error("Non-window object: " + w);
                return null;
            }
            return w.get_stable_sequence();
        };
        WindowProperties.is_resizeable = function (w) {
            return w.resizeable;
        };
        WindowProperties.window_type = function (w) {
            try {
                return w['window-type'];
            }
            catch (e) {
                //TODO: shouldn't be necessary
                Util.log.error("Failed to get window type for window " + w + ", error was:", e);
                return -1;
            }
        };
        WindowProperties.window_class = function (w) {
            return w.get_wm_class();
        };
        WindowProperties.is_shown_on_taskbar = function (w) {
            return !w.is_skip_taskbar();
        };
        WindowProperties.floating_window = function (w) {
            //TODO: add check for w.below when mutter exposes it as a property;
            return w.above;
        };
        WindowProperties.on_all_workspaces = function (w) {
            return w.is_on_all_workspaces();
        };
        WindowProperties.should_auto_tile = function (w) {
            return this.can_be_tiled(w) && this.is_resizeable(w) && !(this.floating_window(w));
        };
        WindowProperties.can_be_tiled = function (w) {
            if (w.is_skip_taskbar()) {
                // this.log.debug("uninteresting window: " + this);
                return false;
            }
            if (this.on_all_workspaces(w)) {
                return false;
            }
            var window_class = this.window_class(w);
            var blacklisted = WindowProperties.blacklist_classes.indexOf(window_class) != -1;
            if (blacklisted) {
                Util.log.debug("window class " + window_class + " is blacklisted");
                return false;
            }
            var window_type = this.window_type(w);
            var result = this.tileable_window_types.indexOf(window_type) != -1;
            // this.log.debug("window " + this + " with type == " + window_type + " can" + (result ? "" : " NOT") + " be tiled");
            return result;
        };
        WindowProperties.get_actor = function (w) {
            try {
                // terribly unobvious name for "this MetaWindow's associated MetaWindowActor"
                return w.get_compositor_private();
            }
            catch (e) {
                // not implemented for some special windows - ignore them
                Util.log.warn("couldn't call get_compositor_private for window " + w, e);
                if (w.get_compositor_private) {
                    Util.log.warn("But the function exists! aborting...");
                    throw (e);
                }
            }
            return null;
        };
        // This seems to be a good set, from trial and error...
        WindowProperties.tileable_window_types = [
            Meta.WindowType.NORMAL,
            Meta.WindowType.DIALOG,
            Meta.WindowType.TOOLBAR,
            Meta.WindowType.UTILITY,
            Meta.WindowType.SPLASHSCREEN
        ];
        // TODO: expose this as a preference if it gets used much
        WindowProperties.blacklist_classes = [
            'Conky'
        ];
        return WindowProperties;
    })();
    MutterWindow.WindowProperties = WindowProperties;
    var Window = (function () {
        function Window(meta_window, ext) {
            this.bound_signals = [];
            this.meta_window = meta_window;
            this.ext = ext;
            this.log = Logging.getLogger("shellshape.window");
            this.tile_preference = null;
        }
        Window.prototype.id = function () {
            return WindowProperties.id(this.meta_window);
        };
        Window.prototype.bring_to_front = function () {
            // NOOP (TODO: remove)
        };
        Window.prototype.is_active = function () {
            return this.ext.current_window() === this;
        };
        Window.prototype.activate = function () {
            this._activate();
        };
        Window.prototype._activate = function (time) {
            Main.activateWindow(this.meta_window, time);
        };
        Window.prototype.is_minimized = function () {
            return this.meta_window.minimized;
        };
        Window.prototype.minimize = function () {
            this.meta_window.minimize();
        };
        Window.prototype.unminimize = function () {
            this.meta_window.unminimize();
        };
        Window.prototype.maximize = function () {
            this.meta_window.maximize(Meta.MaximizeFlags.HORIZONTAL | Meta.MaximizeFlags.VERTICAL);
        };
        Window.prototype.activate_before_redraw = function (reason) {
            var time = global.get_current_time();
            if (time === 0) {
                this.log.debug("activate_before_redraw() when time==0 (probably during initialization) - disregarding");
                return;
            }
            var self = this;
            //TODO: idle seems to be the only LaterType that reliably works; but
            // it causes a visual flash. before_redraw would be better, but that
            // doesn't seem to be late enough in the layout cycle to move windows around
            // (which is what this hook is used for).
            Meta.later_add(Meta.LaterType.IDLE, function () {
                // self.log.debug("Activating window " + self + " (" + reason + ")");
                self._activate(time);
            }, null, null);
        };
        Window.prototype.move_to_workspace = function (new_index) {
            this.meta_window.change_workspace_by_index(new_index, false, global.get_current_time());
        };
        Window.prototype.move_resize = function (r) {
            this.meta_window.unmaximize(Meta.MaximizeFlags.VERTICAL | Meta.MaximizeFlags.HORIZONTAL);
            var pos = r.pos;
            var size = r.size;
            this.meta_window.move_resize_frame(true, pos.x, pos.y, size.x, size.y);
        };
        Window.prototype.set_tile_preference = function (new_pref) {
            if (this.tile_preference === new_pref) {
                this.log.debug("window already had tile preference of " + new_pref);
                return;
            }
            this.log.debug("window adopting tile preference of " + new_pref + " - " + this);
            this.tile_preference = new_pref;
        };
        Window.prototype.get_title = function () {
            return this.meta_window.get_title();
        };
        Window.prototype.toString = function () {
            return ("<#Window with MetaWindow: " + this.get_title() + ">");
        };
        Window.prototype.eq = function (other) {
            var eq = this.id() == other.id();
            if (eq && (this != other)) {
                this.log.warn("Multiple wrappers for the same MetaWindow created: " + this);
            }
            return eq;
        };
        // dimensions
        Window.prototype.rect = function () {
            var r = this.meta_window.get_frame_rect();
            return {
                pos: { x: r.x, y: r.y },
                size: { x: r.width, y: r.height }
            };
        };
        Window.prototype.get_actor = function () {
            return WindowProperties.get_actor(this.meta_window);
        };
        // proxy signals through to actor. If we attach signals directly to the actor, it
        // disappears before we can detach them and we leak BoundSignal objects.
        Window.prototype.connect = function (name, cb) {
            var actor = this.get_actor();
            return actor.connect.apply(actor, arguments);
        };
        Window.prototype.disconnect = function (sig) {
            var actor = this.get_actor();
            if (!actor) {
                this.log.debug("Can't disconnect signal - actor is destroyed");
                return;
            }
            return actor.disconnect.apply(actor, arguments);
        };
        return Window;
    })();
    MutterWindow.Window = Window;
})(MutterWindow || (MutterWindow = {}));
/// <reference path="common.ts" />
/// <reference path="extension.ts" />
/// <reference path="tiling.ts" />
/// <reference path="mutter_window.ts" />
var Workspace;
(function (_Workspace) {
    var Mainloop = imports.mainloop;
    var Lang = imports.lang;
    var Meta = imports.gi.Meta;
    var WindowProperties = MutterWindow.WindowProperties;
    var SIGNALS_ON_ACTORS = !Util.shell_version_gte(3, 11);
    function _duck_overview(fn) {
        return function () {
            var _this = this;
            var _args = arguments;
            this.extension.perform_when_overview_is_hidden(function () {
                return fn.apply(_this, _args);
            });
        };
    }
    ;
    function _duck_turbulence(fn) {
        return function () {
            var _this = this;
            var _args = arguments;
            this.turbulence.add_action(function () {
                return fn.apply(_this, _args);
            });
        };
    }
    ;
    function _duck_grab_op(fn) {
        return function () {
            var _this = this;
            var _args = arguments;
            return this._duck_grab_op(function () {
                return fn.apply(_this, _args);
            });
        };
    }
    ;
    var move_ops = [Meta.GrabOp.MOVING];
    var resize_ops = [
        Meta.GrabOp.RESIZING_SE,
        Meta.GrabOp.RESIZING_S,
        Meta.GrabOp.RESIZING_SW,
        Meta.GrabOp.RESIZING_N,
        Meta.GrabOp.RESIZING_NE,
        Meta.GrabOp.RESIZING_NW,
        Meta.GrabOp.RESIZING_W,
        Meta.GrabOp.RESIZING_E
    ];
    var all_grab_ops = move_ops.concat(resize_ops);
    // TubulentState allows actions to be delayed - applied when the turbulence is
    // over, but ONLY if this instance was not affected ("shaken").
    var TurbulentState = (function () {
        function TurbulentState() {
            this.affected = false;
            this.active = false;
            this.pending = [];
            this.log = Logging.getLogger("shellshape.workspace.turbulence");
        }
        TurbulentState.prototype.enter = function () {
            this.active = true;
            this.affected = false;
        };
        TurbulentState.prototype.shake = function () {
            this.affected = true;
        };
        TurbulentState.prototype.add_action = function (f) {
            if (this.active) {
                this.pending.push(f);
            }
            else {
                f();
            }
        };
        TurbulentState.prototype.leave = function () {
            if (this.affected) {
                this.log.debug("ignoring " + this.pending.length + " actions due to turbulence");
                this.active = false;
                if (this.cleanup)
                    this.cleanup();
            }
            else {
                for (var i = 0; i < this.pending.length; i++) {
                    this.pending[i]();
                }
            }
            this.pending = [];
        };
        return TurbulentState;
    })();
    var Default = (function () {
        function Default() {
        }
        Default.layout = Tiling.FloatingLayout;
        Default.max_autotile = null;
        return Default;
    })();
    _Workspace.Default = Default;
    var Workspace = (function () {
        function Workspace(meta_workspace, layout_state, ext) {
            this.bound_signals = [];
            this.relayout = _duck_overview(this._relayout);
            // after turbulence, windows may have shuffled. we best make sure we own all windows that we should,
            // and that we don't own any windows that have moved to other workspaces.
            this.check_all_windows = _duck_grab_op(function (is_resuming) {
                var self = this;
                var win;
                var changed = false;
                var expected_meta_windows = self.meta_windows();
                var layout_windows = [];
                var layout_meta_windows = [];
                self.layout.each(function (tile) {
                    var win = as(MutterWindow.Window, tile.window);
                    layout_windows.push(win);
                    layout_meta_windows.push(win.meta_window);
                });
                for (var i = 0; i < layout_meta_windows.length; i++) {
                    win = layout_meta_windows[i];
                    if (expected_meta_windows.indexOf(win) == -1 || !WindowProperties.can_be_tiled(win)) {
                        self.log.debug("removing unexpected window from workspace " + self + ": " + win.get_title());
                        self.on_window_remove(win, true);
                        changed = true;
                    }
                    else {
                        if (is_resuming) {
                            // reattach all signal handlers
                            self.connect_window_signals(layout_windows[i]);
                        }
                    }
                }
                for (var i = 0; i < expected_meta_windows.length; i++) {
                    win = expected_meta_windows[i];
                    if (layout_meta_windows.indexOf(win) == -1 && WindowProperties.can_be_tiled(win)) {
                        changed = true;
                        // we add new windows after a minor delay so that removal from the current workspace happens first
                        // (as removal will wipe out all attached signals)
                        Mainloop.idle_add(function () {
                            // self.log.debug("adding missing window to workspace " + self + ": " + win.get_title());
                            self.on_window_create(win);
                            return false;
                        });
                    }
                }
                if (is_resuming && !changed) {
                    // force a relayout on resume
                    // (if changed is true, this will happen after new windows are dealt with)
                    self.layout.layout();
                }
            });
            this.on_window_create = _duck_turbulence(_duck_overview(function (meta_window, reason) {
                var self = this;
                self._with_window_actor(meta_window, function () {
                    var ws = meta_window.get_workspace();
                    if (!WindowProperties.can_be_tiled(meta_window)) {
                        // self.log.debug("can\'t be tiled");
                        return;
                    }
                    if (!self.is_on_main_screen(meta_window)) {
                        // self.log.debug("not on main screen");
                        return;
                    }
                    if (ws !== self.meta_workspace) {
                        self.log.info("window `" + meta_window.get_title() + "` moved workspace before it could be added to the current layout");
                        return;
                    }
                    var win = self.extension.get_window(meta_window);
                    self.log.debug("on_window_create for " + win);
                    var added = self.layout.add(win, self.extension.focus_window);
                    if (!added) {
                        self.log.debug("window not added to layout (probably a duplicate)");
                        return;
                    }
                    self.connect_window_signals(win);
                    var tile_pref = win.tile_preference;
                    var should_auto_tile;
                    if (tile_pref === null) {
                        should_auto_tile = WindowProperties.should_auto_tile(meta_window);
                    }
                    else {
                        // if the window has a tiling preference (given by a previous user tile/untile action),
                        // that overrides the default should_auto_tile logic
                        self.log.debug("window has a tile preference, and it is " + String(tile_pref));
                        should_auto_tile = tile_pref;
                    }
                    if (should_auto_tile && self.has_tile_space_left()) {
                        self.layout.tile(win);
                    }
                });
            }));
            this.on_window_moved = _duck_overview(this._on_window_moved);
            this.on_window_resized = _duck_overview(this._on_window_resized);
            this.on_window_remove = _duck_turbulence(_duck_overview(this._on_window_remove));
            var self = this;
            assert(meta_workspace);
            assert(layout_state);
            assert(ext);
            this.log = Logging.getLogger("shellshape.workspace");
            this.layout_state = layout_state;
            this.meta_workspace = meta_workspace;
            this.extension = ext;
            this._do = ext._do;
            this.description = "<# Workspace at idx " + (meta_workspace.index()) + ": " + meta_workspace + " >";
            this.screen = ext.screen;
            this.set_layout(Default.layout);
            this.enable(true);
            this.turbulence = new TurbulentState();
            this.turbulence.cleanup = Lang.bind(this, this.check_all_windows);
            // add all initial windows
            this.meta_windows().map(function (win) {
                self.on_window_create(win);
            });
        }
        Workspace.prototype.destroy = function () {
            var self = this;
            self.disable();
            self.meta_windows().map(Lang.bind(self, function (win) {
                self._on_window_remove(win);
            }));
        };
        Workspace.prototype.enable = function (initial) {
            var self = this;
            Util.connect_and_track(this, this.meta_workspace, 'window-added', function (workspace, win) {
                self.on_window_create(win);
            });
            Util.connect_and_track(this, this.meta_workspace, 'window-removed', function (workspace, win) {
                self.on_window_remove(win);
            });
            if (!initial) {
                this.log.debug("Enabling " + this);
                this.check_all_windows(true);
            }
        };
        Workspace.prototype.disable = function () {
            // disable workspace (can be re-enabled)
            var self = this;
            self.log.debug("Disabling " + self);
            Util.disconnect_tracked_signals(self);
            // NOTE: we don't actually untile or remove windows here.
            // They're kept in the current state in case we later re-enable() this
            // workspace object.
            self.layout.restore_original_positions();
            self.layout.each(function (tile) {
                var win = as(MutterWindow.Window, tile.window);
                self.disconnect_window_signals(win);
            });
        };
        Workspace.prototype._reset_layout = function () {
            this.layout_state = this.layout_state.empty_copy();
            this.set_layout(Default.layout);
        };
        Workspace.prototype._take_layout_from = function (other) {
            this.turbulence.shake();
            if (!other) {
                this._reset_layout();
                return;
            }
            var keys = ['layout_state', 'layout', 'active_layout'];
            for (var i = 0; i < keys.length; i++) {
                this[keys[i]] = other[keys[i]];
            }
            this.relayout();
        };
        Workspace.prototype._relayout = function () {
            this.layout.layout();
        };
        Workspace.prototype.set_layout = function (cls) {
            if (!cls)
                throw new Error("invalid layout");
            this.active_layout = cls;
            this.layout = new cls(this.layout_state);
            this.log.debug("laying out according to new layout: " + this.layout);
            this.layout.layout();
        };
        Workspace.prototype.default_layout_changed = function (old_layout, new_layout) {
            if (this.active_layout === old_layout) {
                this.set_layout(new_layout);
            }
        };
        Workspace.prototype.toString = function () {
            return this.description;
        };
        Workspace.prototype._grab_op_signal_handler = function (change, relevant_grabs, cb) {
            // grab_ops occur continually throughout the course of a move / resize.
            // Unfortunately, there's no grab_op "end" signal. So on the first
            // grab_op we set change.pending, and keep triggering checks
            // (at the next available idle point) until the grab_op is over.
            var _handler = function (idle) {
                return Lang.bind(this, function () {
                    var grab_op = global.screen.get_display().get_grab_op();
                    if (relevant_grabs.indexOf(grab_op) != -1) {
                        //wait for the operation to end...
                        if (idle || !change.pending) {
                            Mainloop.idle_add(idle_handler);
                        }
                        change.pending = true;
                    }
                    else {
                        var change_happened = change.pending;
                        // it's critical that this flag be reset before cb() happens, otherwise the
                        // callback will (frequently) trigger a stream of feedback events.
                        change.pending = false;
                        if (grab_op == Meta.GrabOp.NONE && change_happened) {
                            this.log.debug("change event completed");
                            cb.call(this);
                        }
                    }
                    return false;
                });
            };
            var op_handler = _handler.call(this, false);
            var idle_handler = _handler.call(this, true);
            return op_handler;
        };
        Workspace.prototype._duck_grab_op = function (cb) {
            var change = { pending: false };
            var handler = this._grab_op_signal_handler(change, all_grab_ops, cb);
            // fire handler immediately
            handler();
            // if it isn't waiting, call the function immediately
            if (!change.pending)
                cb.call(this);
            else
                this.log.debug("ducking grab op...");
        };
        // TODO: this can go away once we drop support for 0.10
        Workspace.prototype._with_window_actor = function (meta_window, cb, initial) {
            var self = this;
            var actor = MutterWindow.WindowProperties.get_actor(meta_window);
            if (actor) {
                cb();
            }
            else {
                if (initial === false) {
                    self.log.warn("actor unavailable for " + meta_window.get_title());
                    return;
                }
                Mainloop.idle_add(function () {
                    self._with_window_actor(meta_window, cb, false);
                    return false;
                });
            }
        };
        Workspace.prototype.connect_window_signals = function (win) {
            var self = this;
            var emitter = SIGNALS_ON_ACTORS ? win : win.meta_window;
            var bind_to_window_change = function (event_name, relevant_grabs, cb) {
                // we only care about events *after* at least one relevant grab_op,
                var signal_handler = self._grab_op_signal_handler({ pending: false }, relevant_grabs, function () {
                    if (self.screen.count > 1) {
                        self.check_all_windows();
                    }
                    cb(win);
                });
                Util.connect_and_track(self, emitter, event_name + '-changed', signal_handler);
            };
            bind_to_window_change('position', move_ops, Lang.bind(self, self.on_window_moved, emitter));
            bind_to_window_change('size', resize_ops, Lang.bind(self, self.on_window_resized, emitter));
            Util.connect_and_track(self, win.meta_window, 'notify::minimized', Lang.bind(self, self.on_window_minimize_changed));
        };
        Workspace.prototype.disconnect_window_signals = function (win) {
            this.log.debug("Disconnecting signals from " + win);
            if (SIGNALS_ON_ACTORS) {
                // `win` acts as a proxy for the actor which doesn't get GC'd behind our back
                Util.disconnect_tracked_signals(this, win);
            }
            Util.disconnect_tracked_signals(this, win.meta_window);
        };
        Workspace.prototype.has_tile_space_left = function () {
            var n = 0;
            this.layout.tiles.each_tiled(function () {
                n = n + 1;
            });
            var max = Default.max_autotile;
            this.log.debug("there are " + n + " windows tiled, of maximum " + max);
            return (n < max);
        };
        // These functions are bound to the workspace and not the layout directly, since
        // the layout may change at any moment
        // NOTE: these two get shellshape `Window` objects as their callback argument, *not* MetaWindow
        Workspace.prototype._on_window_moved = function (win) {
            this.layout.on_window_moved(win);
        };
        Workspace.prototype._on_window_resized = function (win) {
            this.layout.on_window_resized(win);
        };
        Workspace.prototype.on_window_minimize_changed = function (meta_window) {
            this.log.debug("window minimization state changed for window " + meta_window);
            this.layout.layout();
        };
        Workspace.prototype._on_window_remove = function (meta_window, force) {
            var self = this;
            var win = self.extension.get_window(meta_window);
            var removed = self.layout.on_window_killed(win);
            if (removed) {
                self.log.debug("on_window_remove for " + win + " (" + self + ")");
                self.disconnect_window_signals(win);
            }
            else if (force) {
                self.log.error("Unable to remove window: " + win);
                self.layout.each(function (tile, idx) {
                    var tileWindow = tile.window;
                    if (tileWindow === win) {
                        self.log.error("And yet: Found window match at index: " + idx);
                    }
                    if (tileWindow.meta_window === meta_window) {
                        self.log.error("And yet: Found meta_window match at index: " + idx);
                    }
                    // the above code should be impossible to trigger, but it does, so try again for paranoia:
                    removed = self.layout.on_window_killed(win);
                    if (removed) {
                        self.log.error("Removing window the _second_ time worked");
                    }
                });
            }
            if (removed) {
                self.extension.remove_window(win);
            }
        };
        Workspace.prototype.meta_windows = function () {
            var wins = this.meta_workspace.list_windows();
            wins = wins.filter(Lang.bind(this, this.is_on_main_screen));
            // this.log.debug("Windows on " + this + " = [" + wins.join(", ") + "]");
            return wins;
        };
        Workspace.prototype.is_on_main_screen = function (meta_window) {
            if (this.screen.count <= 1 || meta_window.get_monitor() == this.screen.idx) {
                return true;
            }
            else {
                this.log.debug("ignoring window on non-primary monitor");
                return false;
            }
        };
        return Workspace;
    })();
    _Workspace.Workspace = Workspace;
    if (!SIGNALS_ON_ACTORS) {
        Workspace.prototype._with_window_actor = function (meta_window, cb, initial) {
            cb();
        };
    }
})(Workspace || (Workspace = {}));
/// <reference path="common.ts" />
/// <reference path="logging.ts" />
var ShellshapeSettings;
(function (ShellshapeSettings) {
    var Gio = imports.gi.Gio;
    var Glib = imports.gi.GLib;
    var Config = imports.misc.config;
    var ExtensionUtils = imports.misc.extensionUtils;
    var Ext = ExtensionUtils.getCurrentExtension();
    var SCHEMA_ROOT = 'org.gnome.shell.extensions.net.gfxmonk.shellshape';
    var KEYBINDINGS = SCHEMA_ROOT + '.keybindings';
    var PREFS = SCHEMA_ROOT + '.prefs';
    var log = Logging.getLogger("shellshape.settings");
    function envp_with_shellshape_xdg_data_dir() {
        var xdg_data_base = Ext.dir.get_child('data');
        if (!xdg_data_base.query_exists(null)) {
            log.info("xdg dir doesn't exist - assuming global install");
            return null;
        }
        xdg_data_base = xdg_data_base.get_path();
        var XDG_DATA_DIRS = 'XDG_DATA_DIRS';
        var old_xdg_data = Glib.getenv(XDG_DATA_DIRS);
        var new_xdg_data = null;
        if (old_xdg_data != null) {
            var entries = old_xdg_data.split(':');
            if (entries.indexOf(xdg_data_base) == -1) {
                new_xdg_data = old_xdg_data + ':' + xdg_data_base;
            }
        }
        else {
            var default_xdg = "/usr/local/share/:/usr/share/";
            new_xdg_data = default_xdg + ":" + xdg_data_base;
        }
        //TODO: so much effort to modify one key in the environment,
        // surely there is an easier way...
        var strings = [];
        strings.push(XDG_DATA_DIRS + '=' + new_xdg_data);
        var keys = Glib.listenv();
        for (var i in keys) {
            var key = keys[i];
            if (key == XDG_DATA_DIRS)
                continue;
            var val = Glib.getenv(key);
            strings.push(key + '=' + val);
        }
        ;
        return strings;
    }
    ShellshapeSettings.envp_with_shellshape_xdg_data_dir = envp_with_shellshape_xdg_data_dir;
    ;
    function get_local_gsettings(schema_path) {
        log.info("initting schemas");
        var GioSSS = Gio.SettingsSchemaSource;
        var schemaDir = Ext.dir.get_child('data').get_child('glib-2.0').get_child('schemas');
        var schemaSource;
        if (!(schemaDir.query_exists(null))) {
            log.warn("no directory at: " + schemaDir.get_path() + " - assuming schemas globally installed");
            schemaSource = GioSSS.get_default();
        }
        else {
            log.warn("loading schema from: " + schemaDir.get_path());
            schemaSource = GioSSS.new_from_directory(schemaDir.get_path(), GioSSS.get_default(), false);
        }
        var schemaObj = schemaSource.lookup(schema_path, true);
        if (!schemaObj) {
            throw new Error('Schema ' + schema_path + ' could not be found for extension ' + Ext.metadata.uuid);
        }
        return new Gio.Settings({ settings_schema: schemaObj });
    }
    ;
    function Keybindings() {
        var self = this;
        var settings = this.settings = get_local_gsettings(KEYBINDINGS);
        this.each = function (fn, ctx) {
            var keys = settings.list_children();
            for (var i = 0; i < keys.length; i++) {
                var key = keys[i];
                var setting = {
                    key: key,
                    get: function () {
                        return settings.get_string_array(key);
                    },
                    set: function (v) {
                        settings.set_string_array(key, v);
                    }
                };
                fn.call(ctx, setting);
            }
        };
    }
    ShellshapeSettings.Keybindings = Keybindings;
    ;
    function Prefs() {
        var self = this;
        var settings = this.settings = get_local_gsettings(PREFS);
        var get_int = function () {
            return settings.get_int(this.key);
        };
        var set_int = function (v) {
            settings.set_int(this.key, v);
        };
        var get_string = function () {
            return settings.get_string(this.key);
        };
        var set_string = function (v) {
            settings.set_string(this.key, v);
        };
        this.MAX_AUTOTILE = {
            key: 'max-autotiled-windows',
            gsettings: settings,
            get: get_int,
            set: set_int
        };
        this.DEFAULT_LAYOUT = {
            key: 'default-layout',
            gsettings: settings,
            get: get_string,
            set: set_string
        };
        this.PADDING = {
            key: 'tile-padding',
            gsettings: settings,
            get: get_int,
            set: set_int
        };
        this.SCREEN_PADDING = {
            key: 'screen-padding',
            gsettings: settings,
            get: get_int,
            set: set_int
        };
    }
    ShellshapeSettings.Prefs = Prefs;
    ;
    function initTranslations(domain) {
        domain = domain || Ext.metadata['gettext-domain'];
        // check if this extension was built with "make zip-file", and thus
        // has the locale files in a subfolder
        // otherwise assume that extension has been installed in the
        // same prefix as gnome-shell
        var localeDir = Ext.dir.get_child('locale');
        if (localeDir.query_exists(null)) {
            imports.gettext.bindtextdomain(domain, localeDir.get_path());
        }
        else {
            imports.gettext.bindtextdomain(domain, Config.LOCALEDIR);
        }
        log.info("translations initted for " + domain);
    }
    ShellshapeSettings.initTranslations = initTranslations;
})(ShellshapeSettings || (ShellshapeSettings = {}));
// shellshape -- a tiling window manager extension for gnome-shell
/// <reference path="common.ts" />
/// <reference path="logging.ts" />
/// <reference path="util.ts" />
/// <reference path="tiling.ts" />
/// <reference path="indicator.ts" />
/// <reference path="workspace.ts" />
/// <reference path="mutter_window.ts" />
/// <reference path="shellshape_settings.ts" />
/// <reference path="indicator.ts" />
/// <reference path="tiling.ts" />
var Lang = imports.lang;
var Extension;
(function (_Extension) {
    var Main = imports.ui.main;
    var Shell = imports.gi.Shell;
    var St = imports.gi.St;
    var Mainloop = imports.mainloop;
    var Signals = imports.signals;
    var ExtensionUtils = imports.misc.extensionUtils;
    var Extension = ExtensionUtils.getCurrentExtension();
    var Window = MutterWindow.Window;
    var GLib = imports.gi.GLib;
    var Gio = imports.gi.Gio;
    var KEYBINDING_BASE = 'org.gnome.shell.extensions.net.gfxmonk.shellshape.keybindings';
    var LAYOUTS = {
        'floating': Tiling.FloatingLayout,
        'vertical': Tiling.VerticalTiledLayout,
        'horizontal': Tiling.HorizontalTiledLayout,
        'fullscreen': Tiling.FullScreenLayout
    };
    var LAYOUT_ORDER = [
        Tiling.FloatingLayout,
        Tiling.VerticalTiledLayout,
        Tiling.HorizontalTiledLayout,
        Tiling.FullScreenLayout
    ];
    var checkWorkspacesMode = { paranoid: true, is_resuming: false };
    var initializeWorkspacesMode = { paranoid: false, is_resuming: true };
    var workspacesChangedMode = { paranoid: false, is_resuming: false };
    var Ext = (function () {
        function Ext() {
            this.bound_signals = [];
            this.workspaces = [];
            this.bounds = null;
            this.windows = {};
            this.dead_windows = [];
            this._bound_keybindings = {};
            this._pending_actions = [];
            var self = this;
            self.enabled = false;
            self.log = Logging.getLogger("shellshape.extension");
            self.prefs = new ShellshapeSettings.Prefs();
            ShellshapeSettings.initTranslations();
            self.screen_padding = 0;
            /* -------------------------------------------------------------
            *                 Utility functions
            * ------------------------------------------------------------- */
            // Returns a string representation of the extension.
            self.toString = function () {
                return "<Shellshape Extension>";
            };
            // Safely execute a callback by catching any
            // exceptions and logging the traceback and a caller-provided
            // description of the action.
            self._do = function _do(action, desc, fail) {
                try {
                    self.log.debug("+ start action: " + desc);
                    action();
                    return null;
                }
                catch (e) {
                    self.log.error("Uncaught error in " + desc + ": " + e + "\n" + e.stack);
                    if (fail)
                        throw e;
                    return e;
                }
            };
            /* -------------------------------------------------------------
            *           window / workspace object management
            * ------------------------------------------------------------- */
            // Given a `proxy GIName:Meta.Workspace`, return a corresponding
            // shellshape Workspace (as defined in shellshape/workspace.js).
            self.get_workspace = function get_workspace(meta_workspace) {
                assert(meta_workspace);
                self.update_workspaces(checkWorkspacesMode);
                // It's more efficient to use use MetaWorkspace#index(),
                // but it terminates gnome-shell if the workspace has been removed
                var ws = null;
                for (var i = 0; i < this.workspaces.length; i++) {
                    if (this.workspaces[i].meta_workspace === meta_workspace) {
                        return this.workspaces[i];
                    }
                }
                throw new Error("workspace not found: " + meta_workspace);
            };
            self.get_workspace_at = function get_workspace_at(idx) {
                self.update_workspaces(checkWorkspacesMode);
                var ws = self.workspaces[idx];
                assert(ws);
                return ws;
            };
            // Given a gome-shell meta window, return a shellshape Window object
            // and cache the result. Future calls with the same meta_window will
            // return the same wrapper.
            self.get_window = function get_window(meta_window, create_if_necessary) {
                create_if_necessary = create_if_necessary !== false; // default to true
                if (!meta_window)
                    return null;
                var id = MutterWindow.WindowProperties.id(meta_window);
                if (id == null) {
                    self.log.error("window has no ID: " + meta_window);
                    return null;
                }
                var win = self.windows[id];
                if (win == null && create_if_necessary) {
                    win = self.windows[id] = new Window(meta_window, self);
                }
                else {
                    // if window is scheduled for GC, stop that:
                    self.mark_window_as_active(win);
                }
                return win;
            };
            // Remove a window from the extension's cache.
            // this doesn't happen immediately, but only on the next "GC"
            // gc happens whenever the overview window is closed, or
            // dead_windows grows larger than 20 items
            self.remove_window = function (win) {
                var meta_window = win.meta_window;
                var id = MutterWindow.WindowProperties.id(meta_window);
                self.dead_windows.push(win);
                if (self.dead_windows.length > 20) {
                    self.gc_windows();
                }
            };
            self.mark_window_as_active = function (win) {
                var idx = self.dead_windows.indexOf(win);
                if (idx != -1) {
                    self.dead_windows.splice(idx, 1);
                }
            };
            // garbage collect windows that have been marked as "dead"
            // (and haven't been revived since then).
            self.gc_windows = function () {
                if (self.dead_windows.length > 0) {
                    self.log.info("Garbage collecting " + self.dead_windows.length + " windows");
                }
                for (var i = 0; i < self.dead_windows.length; i++) {
                    var win = self.dead_windows[i];
                    delete self.windows[MutterWindow.WindowProperties.id(win.meta_window)];
                }
                self.dead_windows = [];
            };
            // Returns a Workspace (shellshape/workspace.js) representing the
            // current workspace.
            self.current_workspace = function current_workspace() {
                return self.get_workspace_at(global.screen.get_active_workspace_index());
            };
            // Return a gnome-shell meta-workspace representing the current workspace.
            self.mutter_workspace = function current_meta_workspace(idx) {
                if (arguments.length === 0)
                    idx = global.screen.get_active_workspace_index();
                self.log.debug("getting workspace #" + idx);
                if (Logging.PARANOID) {
                    if (idx == null || idx > global.screen.get_n_workspaces())
                        throw new Error("no such workspace: " + idx);
                }
                return global.screen.get_workspace_by_index(idx);
            };
            // Returns the Layout (shellshape/tiling.js,coffee) tied to the current
            // workspace.
            self.current_layout = function current_layout() {
                return self.current_workspace().layout;
            };
            // Perform an action on each workspace
            self.on_all_workspaces = function (cb) {
                var num_workspaces = global.screen.get_n_workspaces();
                for (var i = 0; i < num_workspaces; i++) {
                    cb(self.get_workspace_at(i));
                }
            };
            // Returns the gnome-shell meta-display that is currently active.
            self.current_display = function current_display() {
                return global.screen.get_display();
            };
            // Returns the shellshape Window corresponding with the currently
            // focused-on window.
            self.current_window = function current_window() {
                var current = self.current_display()['focus-window'];
                if (!current) {
                    self.log.debug("no current window");
                    return null;
                }
                return self.get_window(current);
            };
            // Changes the current workspace by +1 or -1.  If provided with a
            // window, then that window is moved to the destination workspace.
            // Called directly upon keypress.  Bound in _init_keybindings().
            self.switch_workspace = function switch_workspace(offset, window) {
                var activate_index = global.screen.get_active_workspace_index();
                var new_index = activate_index + offset;
                if (new_index < 0 || new_index >= global.screen.get_n_workspaces()) {
                    self.log.debug("No such workspace; ignoring");
                    return;
                }
                var next_workspace = self.mutter_workspace(new_index);
                if (window !== undefined) {
                    window.move_to_workspace(new_index);
                    next_workspace.activate_with_focus(window.meta_window, global.get_current_time());
                }
                else {
                    next_workspace.activate(global.get_current_time());
                }
            };
            /* -------------------------------------------------------------
            *   OVERVIEW ducking, and dealing with changes in workspaces
            *          and windows from within the overview mode.
            * ------------------------------------------------------------- */
            self._init_overview = function _init_overview() {
                Util.connect_and_track(self, Main.overview, 'hiding', function () {
                    if (self._pending_actions.length > 0) {
                        self.log.debug("Overview hiding - performing " + self._pending_actions.length + " pending actions");
                        for (var i = 0; i < self._pending_actions.length; i++) {
                            self._do(self._pending_actions[i], "pending action " + i);
                        }
                        self._pending_actions = [];
                    }
                    self.gc_windows();
                });
            };
            // Pretty messy stuff: The overview workspace thumbnail area inserts a new workspace
            // by simply moving everything one workspace up and leaving a hole where the new workspace
            // is supposed to go. This means that our Shellshape.Workspace objects (keyed by
            // meta_workspace object) are now attached to the wrong meta_workspace.
            //
            // The attachment to workspace object is not itself a problem, but we need to move
            // all the user-facing details (layout and tile state) in the same way that the
            // overview moves the windows.
            //
            // Note that this function is executed once at construction time, not during init()
            // (and is never unbound). It doesn't *do* anything while the extension is inactive,
            // but there's no correct way to undo a monkey-patching if other extensions also
            // monkey-patched the same function.
            (function () {
                return; // NOTE: DISABLED until bugs are ironed out.
                var src = imports.ui.workspaceThumbnail.ThumbnailsBox.prototype;
                var orig = src.acceptDrop;
                var replace = function (old_idx, new_idx) {
                    self.log.debug("copying layout from workspace[" + old_idx + "] to workspace[" + new_idx + "]");
                    if (old_idx == new_idx)
                        return;
                    self.get_workspace_at(new_idx)._take_layout_from(self.get_workspace_at(old_idx));
                };
                var replacement = function () {
                    var subject = this;
                    if (!self.enabled)
                        return orig.apply(subject, arguments);
                    var _dropPlaceholderPos = subject._dropPlaceholderPos;
                    self.on_all_workspaces(function (ws) {
                        ws.turbulence.enter();
                    });
                    self.log.debug("acceptDrop start");
                    var ret = orig.apply(subject, arguments);
                    self.log.debug("acceptDrop returned: " + String(ret));
                    self.log.debug("_dropPlaceholderPos: " + String(_dropPlaceholderPos));
                    if (ret === true && _dropPlaceholderPos != -1) {
                        // a new workspace was inserted at _dropPlaceholderPos
                        _dropPlaceholderPos = _dropPlaceholderPos + 0; // just in case it's null or something daft.
                        self.log.debug("looks like a new workspace was inserted at position " + _dropPlaceholderPos);
                        var num_workspaces = global.screen.get_n_workspaces();
                        for (var i = num_workspaces - 1; i > _dropPlaceholderPos; i--) {
                            replace(i - 1, i);
                        }
                        self.get_workspace_at(_dropPlaceholderPos)._take_layout_from(null);
                        // confusing things will happen if we ever get two workspaces referencing the
                        // same layout, so make sure it hasn't happened:
                        var layouts = [];
                        for (var i = 0; i < num_workspaces; i++) {
                            var layout = self.get_workspace_at(i).layout;
                            if (layouts.indexOf(layout) != -1) {
                                throw new Error("Aliasing error! two workspaces ended up with the same layout: " + i + " and " + layouts.indexOf(layout));
                            }
                            layouts.push(layout);
                        }
                        self.emit('layout-changed');
                    }
                    ;
                    self.log.debug("acceptDrop end");
                    self.on_all_workspaces(function (ws) {
                        ws.turbulence.leave();
                    });
                    return ret;
                };
                src.acceptDrop = replacement;
            })();
            self.perform_when_overview_is_hidden = function (action) {
                if (Main.overview.visible) {
                    self.log.debug("Overview currently visible - delaying action");
                    self._pending_actions.push(action);
                }
                else {
                    action();
                }
            };
            /* -------------------------------------------------------------
            *                          KEYBINDINGS
            * ------------------------------------------------------------- */
            // Bind keys to callbacks.
            self._init_keybindings = function _init_keybindings() {
                var Meta = imports.gi.Meta;
                var gsettings = new ShellshapeSettings.Keybindings().settings;
                // Utility method that binds a callback to a named keypress-action.
                function handle(name, func) {
                    self._bound_keybindings[name] = true;
                    var handler = function () {
                        self._do(func, "handler for binding " + name);
                    };
                    var flags = Meta.KeyBindingFlags.NONE;
                    // API for 3.8+ only
                    var KeyBindingMode = Shell.ActionMode ? "ActionMode" : "KeyBindingMode";
                    var added = Main.wm.addKeybinding(name, gsettings, flags, Shell[KeyBindingMode].NORMAL | Shell[KeyBindingMode].MESSAGE_TRAY, handler);
                    if (!added) {
                        throw ("failed to add keybinding handler for: " + name);
                    }
                }
                self.log.debug("adding keyboard handlers for Shellshape");
                handle('tile-current-window', function () {
                    self.current_layout().tile(self.current_window());
                });
                handle('untile-current-window', function () {
                    self.current_layout().untile(self.current_window());
                });
                handle('adjust-splits-to-fit', function () {
                    self.current_layout().adjust_splits_to_fit(self.current_window());
                });
                handle('increase-main-window-count', function () {
                    self.current_layout().add_main_window_count(1);
                });
                handle('decrease-main-window-count', function () {
                    self.current_layout().add_main_window_count(-1);
                });
                handle('next-window', function () {
                    self.current_layout().select_cycle(1);
                });
                handle('prev-window', function () {
                    self.current_layout().select_cycle(-1);
                });
                handle('rotate-current-window', function () {
                    self.current_layout().cycle(1);
                });
                handle('rotate-current-window-reverse', function () {
                    self.current_layout().cycle(-1);
                });
                handle('focus-main-window', function () {
                    self.current_layout().activate_main_window();
                });
                handle('swap-current-window-with-main', function () {
                    self.current_layout().swap_active_with_main();
                });
                // layout changers
                handle('set-layout-tiled-vertical', function () {
                    self.change_layout(Tiling.VerticalTiledLayout);
                });
                handle('set-layout-tiled-horizontal', function () {
                    self.change_layout(Tiling.HorizontalTiledLayout);
                });
                handle('set-layout-floating', function () {
                    self.change_layout(Tiling.FloatingLayout);
                });
                handle('set-layout-fullscreen', function () {
                    self.change_layout(Tiling.FullScreenLayout);
                });
                handle('next-layout', function () {
                    self.next_layout();
                });
                handle('prev-layout', function () {
                    self.previous_layout();
                });
                // move a window's borders
                // to resize it
                handle('increase-main-split', function () {
                    self.current_layout().adjust_main_window_area(+Tiling.BORDER_RESIZE_INCREMENT);
                });
                handle('decrease-main-split', function () {
                    self.current_layout().adjust_main_window_area(-Tiling.BORDER_RESIZE_INCREMENT);
                });
                handle('increase-minor-split', function () {
                    self.current_layout().adjust_current_window_size(+Tiling.BORDER_RESIZE_INCREMENT);
                });
                handle('decrease-minor-split', function () {
                    self.current_layout().adjust_current_window_size(-Tiling.BORDER_RESIZE_INCREMENT);
                });
                // resize a window without
                // affecting others
                handle('decrease-main-size', function () {
                    self.current_layout().scale_current_window(-Tiling.WINDOW_ONLY_RESIZE_INCREMENT, 'x');
                });
                handle('increase-main-size', function () {
                    self.current_layout().scale_current_window(+Tiling.WINDOW_ONLY_RESIZE_INCREMENT, 'x');
                });
                handle('decrease-minor-size', function () {
                    self.current_layout().scale_current_window(-Tiling.WINDOW_ONLY_RESIZE_INCREMENT, 'y');
                });
                handle('increase-minor-size', function () {
                    self.current_layout().scale_current_window(+Tiling.WINDOW_ONLY_RESIZE_INCREMENT, 'y');
                });
                handle('decrease-size', function () {
                    self.current_layout().scale_current_window(-Tiling.WINDOW_ONLY_RESIZE_INCREMENT);
                });
                handle('increase-size', function () {
                    self.current_layout().scale_current_window(+Tiling.WINDOW_ONLY_RESIZE_INCREMENT);
                });
                handle('switch-workspace-down', function () {
                    self.switch_workspace(+1);
                });
                handle('switch-workspace-up', function () {
                    self.switch_workspace(-1);
                });
                handle('move-window-workspace-down', function () {
                    self.switch_workspace(+1, self.current_window());
                });
                handle('move-window-workspace-up', function () {
                    self.switch_workspace(-1, self.current_window());
                });
                handle('toggle-maximize', function () {
                    self.current_layout().toggle_maximize();
                });
                handle('minimize-window', function () {
                    self.current_layout().minimize_window();
                });
                handle('unminimize-last-window', function () {
                    self.current_layout().unminimize_last_window();
                });
                self.log.debug("Done adding keyboard handlers for Shellshape");
            };
            /* -------------------------------------------------------------
            *           workspace / layout changes
            * ------------------------------------------------------------- */
            // Change the layout of the current workspace.
            self.change_layout = function (cls) {
                self.current_workspace().set_layout(cls);
                // This emits a gobject signal that others can watch.
                // ShellshapeIndicator uses it to update the "current layout" display.
                self.emit('layout-changed');
            };
            var modulo = function (n, range) {
                return (n + range) % range;
            };
            var shift_layout = function (diff) {
                var current_layout = self.current_workspace().active_layout;
                var idx = LAYOUT_ORDER.indexOf(current_layout);
                if (idx === -1)
                    throw new Error("Unknown current_layout");
                var new_idx = modulo(idx + diff, LAYOUT_ORDER.length);
                var new_layout = LAYOUT_ORDER[new_idx];
                self.log.debug("Current idx = " + idx + ", new layout[" + (new_idx) + "] = " + new_layout);
                self.change_layout(new_layout);
            };
            self.next_layout = function () {
                shift_layout(1);
            };
            self.previous_layout = function () {
                shift_layout(-1);
            };
            self.update_workspaces = function (mode) {
                if (mode.paranoid && !Logging.PARANOID)
                    return; // don't bother
                var logm = mode.paranoid ? 'error' : 'debug';
                // modified from gnome-shell/js/ui/workspacesView.js
                var old_n = self.workspaces.length;
                var new_n = global.screen.get_n_workspaces();
                if (new_n > old_n) {
                    // Assume workspaces are only added at the end
                    self.log[logm]("new workspaces at index " + old_n + "-" + new_n);
                    for (var w = old_n; w < new_n; w++) {
                        var meta_workspace = self.mutter_workspace(w);
                        // TODO -- the bounds attribute is derived from the size
                        // of the 'screen' and 'monitor' during the .enable() method.
                        // That code overlooks the possibility of two monitors, so
                        // any attempt at two monitors may have to be taken up here
                        // as well.
                        var state = new Tiling.LayoutState(self.bounds);
                        self.workspaces[w] = new Workspace.Workspace(meta_workspace, state, self);
                    }
                }
                else if (new_n < old_n) {
                    // Assume workspaces are only removed sequentially
                    // (e.g. 2,3,4 - not 2,4,7)
                    var removedIndex = new_n; // if we don't get a mismatch during the below loop, the end workspaces must have been removed
                    var removedNum = old_n - new_n;
                    for (var w = 0; w < new_n; w++) {
                        var meta_workspace = self.mutter_workspace(w);
                        if (self.workspaces[w].meta_workspace != meta_workspace) {
                            removedIndex = w;
                            break;
                        }
                    }
                    self.log[logm]("removed workspaces at index " + removedIndex + "-" + (removedIndex + removedNum));
                    var lostWorkspaces = self.workspaces.splice(removedIndex, removedNum);
                    for (var l = 0; l < lostWorkspaces.length; l++) {
                        lostWorkspaces[l].destroy();
                    }
                }
                if (mode.is_resuming) {
                    for (var w = 0; w < Math.min(old_n, self.workspaces.length); w++) {
                        self.workspaces[w].enable();
                    }
                }
                if (Logging.PARANOID) {
                    for (var i = 0; i < new_n; i++) {
                        var actualIdx = self.workspaces[i].meta_workspace.index();
                        if (actualIdx !== i)
                            throw new Error("Workspace expected index " + i + ", but it's " + actualIdx);
                    }
                }
            };
            // Connect callbacks to all workspaces
            self._init_workspaces = function () {
                Util.connect_and_track(self, global.screen, 'notify::n-workspaces', function () {
                    self.update_workspaces(workspacesChangedMode);
                });
                self.update_workspaces(initializeWorkspacesMode);
                var display = self.current_display();
                //TODO: need to disconnect and reconnect when old display changes
                //      (when does that happen?)
                Util.connect_and_track(self, display, 'notify::focus-window', function (display, meta_window) {
                    // DON'T update `focus_window` if this is a window we've never seen before
                    // (it's probably new, and we want to know what the *previous* focus_window
                    // was in order to place it appropriately)
                    var old_focused = self.focus_window;
                    var new_focused = self.get_window(display['focus-window'], false);
                    if (new_focused) {
                        self.focus_window = new_focused;
                    }
                });
            };
            /* -------------------------------------------------------------
            *              PREFERENCE monitoring
            * ------------------------------------------------------------- */
            self._init_prefs = function () {
                var initial = true;
                // default layout
                (function () {
                    var default_layout = self.prefs.DEFAULT_LAYOUT;
                    var update = function () {
                        var name = default_layout.get();
                        var new_layout = LAYOUTS[name];
                        if (new_layout) {
                            self.log.debug("updating default layout to " + name);
                            if (!initial) {
                                var old_layout = Workspace.Default.layout;
                                self.on_all_workspaces(function (ws) {
                                    ws.default_layout_changed(old_layout, new_layout);
                                });
                            }
                            Workspace.Default.layout = new_layout;
                        }
                        else {
                            self.log.error("Unknown layout name: " + name);
                        }
                    };
                    Util.connect_and_track(self, default_layout.gsettings, 'changed::' + default_layout.key, update);
                    update();
                })();
                // max-autotile
                (function () {
                    var pref = self.prefs.MAX_AUTOTILE;
                    var update = function () {
                        var val = pref.get();
                        self.log.debug("setting max-autotile to " + val);
                        Workspace.Default.max_autotile = val;
                    };
                    Util.connect_and_track(self, pref.gsettings, 'changed::' + pref.key, update);
                    update();
                })();
                // padding
                (function () {
                    var pref = self.prefs.PADDING;
                    var update = function () {
                        var val = pref.get();
                        self.log.debug("setting padding to " + val);
                        Tiling.BaseLayout.prototype.padding = val;
                        if (!initial) {
                            self.current_workspace().relayout();
                        }
                    };
                    Util.connect_and_track(self, pref.gsettings, 'changed::' + pref.key, update);
                    update();
                })();
                // screen padding
                (function () {
                    var pref = self.prefs.SCREEN_PADDING;
                    var update = function () {
                        var val = pref.get();
                        self.log.debug("setting screen padding to " + val);
                        // TODO: this is 2* to maintain consistency with inter-window padding (which is applied twice).
                        // inter-window padding should be applied only once so that this isn't required.
                        self.screen_padding = 2 * val;
                        if (!initial) {
                            self.current_workspace().relayout();
                        }
                    };
                    Util.connect_and_track(self, pref.gsettings, 'changed::' + pref.key, update);
                    update();
                })();
                initial = false;
            };
            /* -------------------------------------------------------------
            *                   setup / teardown
            * ------------------------------------------------------------- */
            // Enable ShellshapeIndicator
            self._init_indicator = function () {
                Indicator.ShellshapeIndicator.enable(self);
            };
            var Screen = function () {
                this.bounds = new Bounds();
                this.update();
            };
            Screen.prototype.update = function () {
                this.count = global.screen.get_n_monitors();
                this.idx = global.screen.get_primary_monitor();
                this.bounds.update(global.screen.get_monitor_geometry(this.idx));
            };
            var Bounds = function () {
            };
            Bounds.prototype.update = function (newMonitor) {
                if (newMonitor)
                    this.monitor = newMonitor;
                if (!this.monitor)
                    throw new Error("monitor not yet set");
                var panel_height = Main.panel.actor.height;
                this.pos = {
                    x: this.monitor.x + self.screen_padding,
                    y: this.monitor.y + panel_height + self.screen_padding
                };
                this.size = {
                    x: this.monitor.width - (2 * self.screen_padding),
                    y: this.monitor.height - panel_height - (2 * self.screen_padding)
                };
            };
            // Turn on the extension.  Grabs the screen size to set up boundaries
            // in the process.
            self.enable = function () {
                self.log.info("shellshape enable() called");
                self.enabled = true;
                self.screen = new Screen();
                self.bounds = self.screen.bounds;
                self._do(self._init_overview, "init overview ducking");
                self._do(self._init_prefs, "init preference bindings");
                self._do(self._init_keybindings, "init keybindings");
                self._do(self._init_workspaces, "init workspaces");
                self._do(self._init_screen, "init screen");
                self._do(self._init_indicator, "init indicator");
                self.log.info("shellshape enabled");
            };
            self._init_screen = function () {
                var update_monitor = function () {
                    self.log.info("monitors changed");
                    self.screen.update();
                    self.on_all_workspaces(function (ws) {
                        ws.check_all_windows();
                        ws.relayout();
                    });
                };
                var workspace_switched = function (screen, old_idx, new_idx, direction) {
                    self.get_workspace_at(new_idx).check_all_windows();
                };
                var update_window_workspace = function (screen, idx, meta_window) {
                    if (idx == self.screen.idx) {
                        var ws = meta_window.get_workspace();
                        if (ws)
                            self.get_workspace(ws).check_all_windows();
                        else
                            self.log.debug("update_window_workspace called for a window with no workspace: " + meta_window.get_title());
                    }
                };
                // do a full update when monitors changed (dimensions, num_screens, main_screen_idx, relayout)
                Util.connect_and_track(self, global.screen, 'monitors-changed', update_monitor);
                // sanity check workspaces when switching to them (TODO: remove this if it never fails)
                Util.connect_and_track(self, global.screen, 'workspace-switched', workspace_switched);
                // window-entered-monitor and window-left-monitor seem really twitchy - they
                // can fire a handful of times in a single atomic window placement.
                // So we just use the hint to check window validity, rather than assuming
                // it's actually a new or removed window.
                Util.connect_and_track(self, global.screen, 'window-entered-monitor', update_window_workspace);
                Util.connect_and_track(self, global.screen, 'window-left-monitor', update_window_workspace);
            };
            // Unbinds keybindings
            // NOTE: remove_keybinding should really take a schema,
            // but they don't yet.
            // see: https://bugzilla.gnome.org/show_bug.cgi?id=666513
            self._unbind_keys = function () {
                var display = self.current_display();
                for (var k in self._bound_keybindings) {
                    if (!self._bound_keybindings.hasOwnProperty(k))
                        continue;
                    var desc = "unbinding key " + k;
                    self._do(function () {
                        self.log.debug(desc);
                        if (Main.wm.removeKeybinding) {
                            Main.wm.removeKeybinding(k);
                        }
                        else {
                            display.remove_keybinding(k);
                        }
                    }, desc);
                }
            };
            // Disconnects from *all* workspaces.  Disables and removes
            // them from our cache
            self._disable_workspaces = function () {
                for (var i = 0; i < self.workspaces.length; i++) {
                    self.workspaces[i].disable();
                }
            };
            // Disable the extension.
            self.disable = function () {
                self.enabled = false;
                self.log.info("shellshape disable() called");
                self._do(function () {
                    Indicator.ShellshapeIndicator.disable();
                }, "disable indicator");
                self._do(self._disable_workspaces, "disable workspaces");
                self._do(self._unbind_keys, "unbind keys");
                self._do(function () {
                    Util.disconnect_tracked_signals(self);
                }, "disconnect signals");
                self.log.info("shellshape disabled");
            };
            // If we got here, then nothing exploded while initializing the extension.
            self.log.info("shellshape initialized!");
        }
        return Ext;
    })();
    _Extension.Ext = Ext;
    Signals.addSignalMethods(Ext.prototype);
})(Extension || (Extension = {}));
// export toplevel symbols
function init() {
    Logging.init(true);
    var Gdk = imports.gi.Gdk;
    // inject the get_mouse_position function
    Tiling.get_mouse_position = function () {
        var display = Gdk.Display.get_default();
        var device_manager = display.get_device_manager();
        var pointer = device_manager.get_client_pointer();
        var _pos = pointer.get_position();
        var _screen = _pos[0];
        var pointerX = _pos[1];
        var pointerY = _pos[2];
        return { x: pointerX, y: pointerY };
    };
    return new Extension.Ext();
}
function main() {
    init().enable();
}
;
