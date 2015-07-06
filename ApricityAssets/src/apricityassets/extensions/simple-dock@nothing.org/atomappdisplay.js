// -*- mode: js; js-indent-level: 4; indent-tabs-mode: nil -*-
/*jshint esnext: true */
/*jshint indent: 4 */
const Lang = imports.lang;
const Signals = imports.signals;
const Clutter = imports.gi.Clutter;
const Meta = imports.gi.Meta;
const Shell = imports.gi.Shell;
const St = imports.gi.St;

const AppDisplay = imports.ui.appDisplay;
const AppFavorites = imports.ui.appFavorites;
const Main = imports.ui.main;
const PopupMenu = imports.ui.popupMenu;

let ShellVersion = imports.misc.config.PACKAGE_VERSION.split(".").map(function (x) { return +x; });
const MAJOR_VERSION = ShellVersion[0];
const MINOR_VERSION = ShellVersion[1];

/* This class is a extension of the upstream AppIcon class (ui.appDisplay.js).
 * Changes are done to modify activate, popup menu and running app behavior.
 */
const AtomAppIcon = new Lang.Class({
    Name: 'AtomAppIcon',
    Extends: AppDisplay.AppIcon,

    _init : function(app, iconParams) {

        this.parent(app, iconParams, { setSizeManually: true, showLabel: false });
        this._windowsChangedId = this.app.connect('windows-changed', Lang.bind(this, this._onStateChanged));
        this.actor.set_style("padding: 0px;");
    },

    activate: function (button) {

        let event = Clutter.get_current_event();
        let modifiers = event ? event.get_state() : 0;
        let openNewWindow = modifiers & Clutter.ModifierType.CONTROL_MASK &&
                            this.app.state == Shell.AppState.RUNNING ||
                            button && button == 2;
        let focusedApp = Shell.WindowTracker.get_default().focus_app;
        let windows = this.getAppInterestingWindows();

        if (openNewWindow) {
            this.app.open_new_window(-1);
        } else {
            if (this.app == focusedApp && !Main.overview._shown) {
                this.setMinimizeGeometry(true);
            } else {
                this.app.activate();
                if(windows.length>0) {
                    for (let i=windows.length-1; i>=0; i--){
                        Main.activateWindow(windows[i]);
                    }
                }
            }
        }

        Main.overview.hide();
    },

    setMinimizeGeometry: function (minimize){

        let windows = this.getAppInterestingWindows();
        let current_workspace = global.screen.get_active_workspace();
        let rect = new Meta.Rectangle();

        [rect.x, rect.y] = this.actor.get_transformed_position();
        [rect.width, rect.height] = this.actor.get_transformed_size();

        for (let i = 0; i < windows.length; i++) {
            let w = windows[i];
            if (w.get_workspace() == current_workspace && w.showing_on_its_workspace()){
                w.set_icon_geometry(rect);
                if (minimize) { w.minimize(); }
            }
        }
    },

    getAppInterestingWindows: function(app) {
        // Filter out unnecessary windows, for instance
        // nautilus desktop window.
        return this.app.get_windows().filter(function(w) { return !w.skip_taskbar; });
    },

    _onStateChanged: function() {
        if (this.app.state !== Shell.AppState.STOPPED && this._isAppOnActiveWorkspace()) {
            this.actor.add_style_class_name('running');
        } else {
            this.actor.remove_style_class_name('running');
        }
        this.setMinimizeGeometry(false);
    },

    _onDestroy: function() {
        if (this._windowsChangedId > 0) {
            this.app.disconnect(this._windowsChangedId);
        }

        this._windowsChangedId = 0;
        this.parent();
    },

    popupMenu: function() {
        this._removeMenuTimeout();
        this.actor.fake_release();
        this._draggable.fakeRelease();

        if (!this._menu) {
            this._menu = new AtomAppIconMenu(this);
            this._menu.connect('activate-window', Lang.bind(this, function (menu, window) {
                    this.activateWindow(window);
                })
            );
            this._menu.connect('open-state-changed', Lang.bind(this, function (menu, isPoppedUp) {
                    if (!isPoppedUp) {
                        this._onMenuPoppedDown();
                    }
                })
            );
            Main.overview.connect('hiding', Lang.bind(this, this._menu.close));
            this._menuManager.addMenu(this._menu);
        }

        this.emit('menu-state-changed', true);
        this.actor.set_hover(true);
        this._menu.popup();
        this._menuManager.ignoreRelease();
        this.emit('sync-tooltip');

        return false;
    },

    _isAppOnActiveWorkspace: function() {
        return this.app.is_on_workspace(global.screen.get_active_workspace());
    }
});

Signals.addSignalMethods(AtomAppIcon.prototype);

/* This class is a fork of the upstream AppIconMenu class (ui.appDisplay.js).
 * Changes are done to make popup displayed on top side.
 */
const AtomAppIconMenu = new Lang.Class({
    Name: 'AtomAppIconMenu',
    Extends: PopupMenu.PopupMenu,

    _init: function(source) {

        this.parent(source.actor, 0.5, St.Side.TOP);

        // We want to keep the item hovered while the menu is up
        this.blockSourceEvents = true;
        this._source = source;
        this.actor.add_style_class_name('app-well-menu');

        // Chain our visibility and lifecycle to that of the source
        source.actor.connect('notify::mapped',
            Lang.bind(this, function() {
                if (!source.actor.mapped) {
                    this.close();
                }
            })
        );

        source.actor.connect('destroy', Lang.bind(this, this.actor.destroy));
        Main.uiGroup.add_actor(this.actor);
    },

    _redisplay: function() {

        let app = this._source.app;
        let appInfo = app.get_app_info();
        let actions = null;
        let windows = app.get_windows().filter(function(w) {
            return !w.skip_taskbar;
        });

        this.removeAll();

        if (appInfo === null) {

            this._appendListWindows(windows);
            this._appendSeparator();
            this._appendQuit();

        } else if (!app.is_window_backed()) {

            this._appendListWindows(windows);

            if (windows.length > 0) {

                this._appendSeparator();

                // Add 'action' buttons
                actions = appInfo.list_actions();
                this._appendActions(appInfo, actions, windows);

                if (actions.length > 0) {

                    this._appendSeparator();

                } else {

                    this._appendNewWindow(windows);
                    this._appendSeparator();
                }

                this._openWindowMenuItem = null;

            } else {

                this._appendOpen();
                this._appendSeparator();
            }

            this._appendFavorites(app);

            if (windows.length > 0) {
                this._appendQuit();
            }
        }
    },

    _appendListWindows: function (windows) {

        // Add 'open windows' buttons'
        this._windowMenuItems = new Array(windows.length);
        for (let i = 0; i < windows.length; i++) {
            let name = windows[i].title;
            let chars = 30;
            if (name.length > chars) { name = name.substring(0, 15) + '...' + name.substring(name.length - (chars - 15)); }
            this._windowMenuItems[i] = this._appendMenuItem(name);
            this._windowMenuItems[i]._refWindow = windows[i];
            this._windowMenuItems[i].connect('activate', Lang.bind(this, function (actor, event) {
                this.emit('activate-window', actor._refWindow);
                this.close();
            }));
        }
    },

    _appendActions: function (appInfo, actions, windows) {

        // Add custom application 'actions'
        this._actionMenuItems = new Array(actions.length);
        for (i = 0; i < actions.length; i++) {
            this._actionMenuItems[i] = this._appendMenuItem(appInfo.get_action_name(actions[i]));
            this._actionMenuItems[i]._refAction = actions[i];
            this._actionMenuItems[i]._refWindow = windows[0];
            this._actionMenuItems[i].connect('activate', Lang.bind(this, function (actor, event) {
                let app = this._source.app;
                app.launch_action(actor._refAction, event.get_time(), -1);
                this.emit('activate-window', actor._refWindow);
                this.close();
            }));
        }
    },

    _appendNewWindow: function (windows) {

        // Add 'new window' button only if there are open windows and there is no 'action'
        this._newWindowMenuItem = this._appendMenuItem(_("New Window"));
        this._newWindowMenuItem._refWindow = windows[0];
        this._newWindowMenuItem.connect('activate', Lang.bind(this, function (actor, event) {
            let app = this._source.app;
            app.open_new_window(-1);
            this.emit('activate-window', actor._refWindow);
            this.close();
        }));
    },

    _appendOpen: function () {

        // Add 'open' button (only if there are no open windows)
        this._openWindowMenuItem = this._appendMenuItem(_("Open"));
        this._openWindowMenuItem.connect('activate', Lang.bind(this, function (actor, event) {
            let app = this._source.app;
            app.open_new_window(-1);
            this.emit('activate-window', null);
            this.close();
        }));
    },

    _appendFavorites: function (app) {

        // Add 'add/remove favorites' button
        let isFavorite = AppFavorites.getAppFavorites().isFavorite(app.get_id());
        this._toggleFavoriteMenuItem = this._appendMenuItem(isFavorite ? _("Remove from Favorites") : _("Add to Favorites"));
        this._toggleFavoriteMenuItem.connect('activate', Lang.bind(this, function (actor, event) {
            let app = this._source.app;
            let favs = AppFavorites.getAppFavorites();
            let isFavorite = favs.isFavorite(app.get_id());
            if (isFavorite) {
                favs.removeFavorite(app.get_id());
            } else {
                favs.addFavorite(app.get_id());
            }
            this.close();
        }));
    },

    _appendQuit: function () {

        // Add 'quit' button
        this._quitMenuItem = this._appendMenuItem(_("Quit"))
        this._quitMenuItem.connect('activate', Lang.bind(this, function (actor, event) {
            let app = this._source.app;
            let wins = app.get_windows();
            for (let i=0; i < wins.length; i++) {
                wins[i].delete(global.get_current_time());
            }
            this.close();
        }));
    },

    _appendSeparator: function () {
        let separator = new PopupMenu.PopupSeparatorMenuItem();
        this.addMenuItem(separator);
    },

    _appendMenuItem: function(labelText) {
        let item = new PopupMenu.PopupMenuItem(labelText);
        this.addMenuItem(item);
        return item;
    },

    popup: function(activatingButton) {
        this._redisplay();
        this.open();
    }
});

Signals.addSignalMethods(AtomAppIconMenu.prototype);
