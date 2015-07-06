// -*- mode: js; js-indent-level: 4; indent-tabs-mode: nil -*-
/*jshint esnext: true */
/*jshint indent: 4 */

const Config = imports.misc.config;
const Lang = imports.lang;
const Mainloop = imports.mainloop;
const Meta = imports.gi.Meta;
const Shell = imports.gi.Shell;
const Main = imports.ui.main;

const Me = imports.misc.extensionUtils.getCurrentExtension();
const Convenience = Me.imports.convenience;

const handledWindowTypes = [
    Meta.WindowType.NORMAL,
    Meta.WindowType.DIALOG,
    Meta.WindowType.MODAL_DIALOG,
    Meta.WindowType.TOOLBAR,
    Meta.WindowType.MENU,
    Meta.WindowType.UTILITY,
    Meta.WindowType.SPLASHSCREEN
];

/*
 * Forked from Michele's Dash to Dock extension
 * https://github.com/micheleg/dash-to-dock
 *
 * A rough and ugly implementation of the intellihide behaviour.
 * Intellihide object: call show()/hide() function based on the overlap with the
 * the target actor object;
 *
 * Target object has to contain a Clutter.ActorBox object named staticBox and
 * emit a 'box-changed' signal when this changes.
 *
 */

const Intellihide = new Lang.Class({
    Name: 'Intellihide',

    _init: function(show, hide, retop, target) {
        this._signalHandler = new Convenience.GlobalSignalHandler();
        this._tracker = Shell.WindowTracker.get_default();
        this._focusApp = null;
        this.showMethod = 0;
        this.overviewShowing = false;
        // 3.14 Remove until '<' when losing compatibility
        this.trayShowing = false;
        // <
        // current intellihide status
        this.status = undefined;
        // Set base functions
        this.showFunction = show;
        this.hideFunction = hide;
        this.retopFunction = retop;
        // Target object
        this._target = target;

        /* Main id of the timeout controlling timeout for updateDockVisibility
         * function when windows are dragged around (move and resize).
         */
        this._windowChangedTimeout = 0;

        // Connect global signals
        // 3.14 Remove next if "3.14" ... when losing compatibility
        if (Config.PACKAGE_VERSION.indexOf("3.14.") !== -1) {
            this._signalHandler.push(
                [
                    this._target,
                    'box-changed',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    this._target.dash,
                    'redisplay-workspace-switched',
                    Lang.bind(this, this._switchWorkspace)
                ],
                [
                    global.display,
                    'grab-op-begin',
                    Lang.bind(this, this._grabOpBegin)
                ],
                [
                    global.display,
                    'grab-op-end',
                    Lang.bind(this, this._grabOpEnd)
                ],
                [
                    global.window_manager,
                    'maximize',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    global.window_manager,
                    'unmaximize',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    global.screen,
                    'restacked',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    global.screen,
                    'monitors-changed',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    Main.overview,
                    'showing',
                    Lang.bind(this, this._overviewOn)
                ],
                [
                    Main.overview,
                    'hidden',
                    Lang.bind(this, this._overviewOff)
                ],
                [
                    Main.messageTray,
                    'showing',
                    Lang.bind(this, this._trayOn)
                ],
                [
                    Main.messageTray,
                    'hiding',
                    Lang.bind(this, this._trayOff)
                ]
            );
        } else {
            this._signalHandler.push(
                [
                    this._target,
                    'box-changed',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    this._target.dash,
                    'redisplay-workspace-switched',
                    Lang.bind(this, this._switchWorkspace)
                ],
                [
                    global.display,
                    'grab-op-begin',
                    Lang.bind(this, this._grabOpBegin)
                ],
                [
                    global.display,
                    'grab-op-end',
                    Lang.bind(this, this._grabOpEnd)
                ],
                [
                    global.window_manager,
                    'maximize',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    global.window_manager,
                    'unmaximize',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    global.screen,
                    'restacked',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    global.screen,
                    'monitors-changed',
                    Lang.bind(this, this._updateDockVisibility)
                ],
                [
                    Main.overview,
                    'showing',
                    Lang.bind(this, this._overviewOn)
                ],
                [
                    Main.overview,
                    'hidden',
                    Lang.bind(this, this._overviewOff)
                ]
            );
        }

        // initialize: call show forcing to initialize status variable
        this._show(true);

        // update visibility
        this._updateDockVisibility();
    },

    destroy: function() {
        // Disconnect global signals
        this._signalHandler.disconnect();

        if (this._windowChangedTimeout > 0) {
            Mainloop.source_remove(this._windowChangedTimeout);
        }

        this._windowChangedTimeout = 0;
    },

    _show: function(force) {

        if (this.status !== true || force) {
            this.status = true;
            this.showFunction();
        }

    },

    _hide: function(force) {

        if (this.status !== false || force) {
            this.status = false;
            this.hideFunction();
        }

    },

    _retop: function() {
        this.retopFunction();
    },

    _grabOpBegin: function() {
        // A good compromise between reactivity and efficiency; to be tuned.
        let INTERVAL = 100;

        if (this._windowChangedTimeout > 0) {
            Mainloop.source_remove(this._windowChangedTimeout); // Just to be sure
        }

        this._windowChangedTimeout = Mainloop.timeout_add(INTERVAL,
            Lang.bind(this, function() {
                this._updateDockVisibility();
                // to make the loop continue
                return true;
            })
        );
    },

    _grabOpEnd: function() {

        if (this._windowChangedTimeout > 0) {
            Mainloop.source_remove(this._windowChangedTimeout);
        }

        this._windowChangedTimeout = 0;
        this._updateDockVisibility();
    },

    _switchWorkspace: function() {
        this._updateDockVisibility();
    },

    _updateDockVisibility: function() {

        // 3.14 Remove until '<' when losing compatibility
        if (this.trayShowing) {
            this._hide();
        } else /* < */ if (this.overviewShowing) {
            this._show();
        } else  if (this.showMethod === 2) {
            this._show();
        } else if (this.showMethod === 1) {
            this._hide();
        } else if (this.showMethod === 0) {
         
                let overlaps = false;
                let windows = global.get_window_actors();

                if (windows.length > 0) {
                    // This is the window on top of all others in the current workspace
                    let topWindow = windows[windows.length - 1].get_meta_window();
                    // If there isn't a focused app, use that of the window on top
                    this._focusApp = this._tracker.focus_app ||
                        this._tracker.get_window_app(topWindow);

                    windows = windows.filter(this._intellihideFilterInteresting, this);

                    for (let i = 0; i < windows.length; i++) {

                        let win = windows[i].get_meta_window();
                        if (win) {
                            let rect = win.get_frame_rect();

                            let test = (rect.x < this._target.staticBox.x2) &&
                                (rect.x +rect.width > this._target.staticBox.x1) &&
                                (rect.y < this._target.staticBox.y2) &&
                                (rect.y +rect.height > this._target.staticBox.y1);

                            if (test) {
                                overlaps = true;
                                break;
                            }
                        }
                    }
                }

                if (overlaps) {
                    this._hide();
                } else {
                    this._show();
                }
        }
        this._retop();
    },

    /* Filter interesting windows to be considered for intellihide.
     * Consider all windows visible on the current workspace.
     * Optionally skip windows of other applications
     */
    _intellihideFilterInteresting: function(wa) {
        let currentWorkspace = global.screen.get_active_workspace_index();

        let meta_win = wa.get_meta_window();
        if (!meta_win) {
            return false;
        }

        if (!this._handledWindow(meta_win)) {
            return false;
        }

        let wksp = meta_win.get_workspace();
        let wksp_index = wksp.index();

        // Skip windows of other apps
        if (this._focusApp) {
            /* The DropDownTerminal extension is not an application per sec
             * so we match its window by wm class instead
             */
            if (meta_win.get_wm_class() === 'DropDownTerminalWindow') {
                return true;
            }

            let currentApp = this._tracker.get_window_app(meta_win);

            /* But consider half maximized windows
             * Useful if one is using two apps side by side
             */
            if (this._focusApp !== currentApp &&
                    !(meta_win.maximized_vertically &&
                        !meta_win.maximized_horizontally) && !meta_win.is_above()) {
                return false;
            }
        }

        if (wksp_index === currentWorkspace && meta_win.showing_on_its_workspace()) {
            return true;
        } else {
            return false;
        }
    },

    /* Filter windows by type
     * inspired by Opacify@gnome-shell.localdomain.pl
     */
    _handledWindow: function(metaWindow) {

        /* The DropDownTerminal extension uses the POPUP_MENU window type hint
         * so we match its window by wm class instead
         */
        if (metaWindow.get_wm_class() === 'DropDownTerminalWindow') {

            return true;
        }

        let wtype = metaWindow.get_window_type();

        for (let i = 0; i < handledWindowTypes.length; i++) {
            let hwtype = handledWindowTypes[i];

            if (hwtype === wtype) {

                return true;
            } else if (hwtype > wtype) {

                return false;
            }
        }

        return false;
    },

    _overviewOn: function() {
        this.overviewShowing = true;
        this._updateDockVisibility();
    },

    _overviewOff: function() {
        this.overviewShowing = false;
        this._updateDockVisibility();
    },
    // 3.14 Remove until '<' when losing compatibility
    _trayOn: function() {
        this.trayShowing = true;
        this._updateDockVisibility();
    },

    _trayOff: function() {
        this.trayShowing = false;
        this._updateDockVisibility();
    },
    // <
    setShowMethod: function(method) {
        let trackActor = this._target.actor;
        if (method === 2) {
            Main.layoutManager._trackActor(trackActor, {affectsStruts: true});
            this._target._resetPosition();
        } else {
            Main.layoutManager._untrackActor(trackActor);
        }

        this.showMethod = method;
        this._updateDockVisibility();
    }
});
