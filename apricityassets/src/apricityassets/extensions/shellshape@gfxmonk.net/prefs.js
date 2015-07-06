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
/// <reference path="common.ts" />
/// <reference path="shellshape_settings.ts" />
// NOTE: this file should *not* be imported into extension.js,
// because it provides the roplevel symbols required
// for prefs.js (which clash with extension.js
//
// log4javascript inits /tmp/shellshape.log, which
// is a pain since prefs.js runs in a new process, and
// overwrites the useful log file with a basically empty one.
var Gtk = imports.gi.Gtk;
var GLib = imports.gi.GLib;
var Config = imports.misc.config;
var Gettext = imports.gettext.domain('shellshape');
var _ = Gettext.gettext;
function init() {
    Logging.init();
    ShellshapeSettings.initTranslations();
}
;
function buildPrefsWidget() {
    var config = new ShellshapeSettings.Prefs();
    var frame = new Gtk.Box({
        orientation: Gtk.Orientation.VERTICAL,
        border_width: 10
    });
    var vbox = new Gtk.Box({
        orientation: Gtk.Orientation.VERTICAL,
        spacing: 14
    });
    var label = new Gtk.Label({
        label: _("<b>Tiling:</b>"),
        use_markup: true,
        xalign: 0
    });
    vbox.add(label);
    (function () {
        var hbox = new Gtk.Box({
            orientation: Gtk.Orientation.HORIZONTAL,
            spacing: 20
        });
        var label = new Gtk.Label({ label: _("Maximum number of windows to auto-tile:") });
        var adjustment = new Gtk.Adjustment({
            lower: 0,
            upper: 20,
            step_increment: 1
        });
        var scale = new Gtk.HScale({
            digits: 0,
            adjustment: adjustment,
            value_pos: Gtk.PositionType.RIGHT
        });
        hbox.add(label);
        hbox.pack_end(scale, true, true, 0);
        vbox.add(hbox);
        var pref = config.MAX_AUTOTILE;
        scale.set_value(pref.get());
        scale.connect('value-changed', function (sw) {
            var oldval = pref.get();
            var newval = sw.get_value();
            if (newval != pref.get()) {
                pref.set(newval);
            }
        });
    })();
    (function () {
        var hbox = new Gtk.Box({
            orientation: Gtk.Orientation.HORIZONTAL,
            spacing: 20
        });
        var label = new Gtk.Label({ label: _("Default layout:") });
        var radio_box = new Gtk.Box({
            orientation: Gtk.Orientation.VERTICAL,
            spacing: 2
        });
        var r_floating = new Gtk.RadioButton({ label: _("Floating") });
        var r_vertical = new Gtk.RadioButton({ label: _("Vertical"), group: r_floating });
        var r_horizontal = new Gtk.RadioButton({ label: _("Horizontal"), group: r_floating });
        var r_fullscreen = new Gtk.RadioButton({ label: _("Full Screen"), group: r_floating });
        var layout_radios = {
            'floating': r_floating,
            'horizontal': r_horizontal,
            'vertical': r_vertical,
            'fullscreen': r_fullscreen
        };
        var pref = config.DEFAULT_LAYOUT;
        var active = layout_radios[pref.get()];
        if (active) {
            active.set_active(true);
        }
        var init_radio = function (k) {
            var radio = layout_radios[k];
            radio.connect('toggled', function () {
                if (radio.get_active()) {
                    pref.set(k);
                }
            });
            radio_box.add(radio);
        };
        init_radio('floating');
        init_radio('vertical');
        init_radio('horizontal');
        init_radio('fullscreen');
        hbox.add(label);
        hbox.add(radio_box);
        vbox.add(hbox);
    })();
    (function () {
        var hbox = new Gtk.Box({
            orientation: Gtk.Orientation.HORIZONTAL,
            spacing: 20
        });
        var label = new Gtk.Label({ label: _("Padding between tiles (px)") });
        var adjustment = new Gtk.Adjustment({
            lower: 0,
            upper: 20,
            step_increment: 1
        });
        var scale = new Gtk.HScale({
            digits: 0,
            adjustment: adjustment,
            value_pos: Gtk.PositionType.RIGHT
        });
        hbox.add(label);
        hbox.pack_end(scale, true, true, 0);
        vbox.add(hbox);
        var pref = config.PADDING;
        scale.set_value(pref.get());
        scale.connect('value-changed', function (sw) {
            var oldval = pref.get();
            var newval = sw.get_value();
            if (newval != pref.get()) {
                pref.set(newval);
            }
        });
    })();
    //screenpadding
    (function () {
        var hbox = new Gtk.Box({
            orientation: Gtk.Orientation.HORIZONTAL,
            spacing: 20
        });
        var label = new Gtk.Label({ label: _("Padding around screen edge (px)") });
        var adjustment = new Gtk.Adjustment({
            lower: 0,
            upper: 20,
            step_increment: 1
        });
        var scale = new Gtk.HScale({
            digits: 0,
            adjustment: adjustment,
            value_pos: Gtk.PositionType.RIGHT
        });
        hbox.add(label);
        hbox.pack_end(scale, true, true, 0);
        vbox.add(hbox);
        var pref = config.SCREEN_PADDING;
        scale.set_value(pref.get());
        scale.connect('value-changed', function (sw) {
            var oldval = pref.get();
            var newval = sw.get_value();
            if (newval != pref.get()) {
                pref.set(newval);
            }
        });
    })();
    var label = new Gtk.HSeparator();
    vbox.add(label);
    var label = new Gtk.Label({
        label: _("<b>Advanced settings:</b>"),
        use_markup: true,
        xalign: 0
    });
    vbox.add(label);
    (function () {
        var hbox = new Gtk.Box({
            orientation: Gtk.Orientation.HORIZONTAL,
            spacing: 20
        });
        var label = new Gtk.Label({
            label: _("Edit keyboard settings") + "\n<small>" + _("(make sure you have dconf-editor installed)") + "\n" + _("Navigate to") + " org/gnome/shell/extensions/net/gfxmonk/shellshape</small>",
            use_markup: true
        });
        var button = new Gtk.Button({
            label: 'dconf-editor'
        });
        var error_msg = new Gtk.Label();
        button.connect('clicked', function (sw) {
            try {
                // The magic sauce that lets dconf-editor see our local schema:
                var envp = ShellshapeSettings.envp_with_shellshape_xdg_data_dir();
                GLib.spawn_async(null, ['dconf-editor'], envp, GLib.SpawnFlags.SEARCH_PATH, null);
            }
            catch (e) {
                error_msg.set_label(_("ERROR: Could not launch dconf-editor. Is it installed?"));
                throw e;
            }
        });
        hbox.add(label);
        hbox.pack_end(button, false, false, 0);
        vbox.add(hbox);
        vbox.add(error_msg);
    })();
    frame.add(vbox);
    frame.show_all();
    return frame;
}
