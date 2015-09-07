/*  Copyright (C) 2014 Raphael Freudiger <laser_b@gmx.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Author: Raphael Freudiger <laser_b@gmx.ch>
**/
const Gio = imports.gi.Gio;
const GLib = imports.gi.GLib;
const Lang = imports.lang;
const Mainloop = imports.mainloop;

const LoginManager = imports.misc.loginManager;
const Main = imports.ui.main;
const StatusSystem = imports.ui.status.system;
const PopupMenu = imports.ui.popupMenu;
const ExtensionSystem = imports.ui.extensionSystem;

const SHOW_TWO_BUTTONS = 'show-two-buttons';
const SUSPEND_DEFAULT = 'suspend-default';

const Gettext = imports.gettext.domain('gnome-shell-extension-suspend-button');
const _ = Gettext.gettext;

const Me = imports.misc.extensionUtils.getCurrentExtension();
const Lib = Me.imports.lib;

const Extension = new Lang.Class({
    Name: 'SuspendButton.Extension',

    enable: function() {
        this._loginManager = LoginManager.getLoginManager();
        this.systemMenu = Main.panel.statusArea['aggregateMenu']._system;

        this._settings = Lib.getSettings(Me);
        this._toggleTwoButtonsID = this._settings.connect("changed::" + SHOW_TWO_BUTTONS, Lang.bind(this, function() {
            this._toggleTwoButtons();
        }));
        this._toggleSuspendDefaultID = this._settings.connect("changed::" + SUSPEND_DEFAULT, Lang.bind(this, function() {
            this._toggleSuspendDefault();
        }));
        
        if (this._settings.get_boolean(SHOW_TWO_BUTTONS)) {
            this._createActions();
            this._removealtSwitcher();
            this._addSingleButtons();
        }
        else {
            if (this._settings.get_boolean(SUSPEND_DEFAULT)) {
                this._createActions();
                this._removealtSwitcher();
                this._createaltStatusSwitcher();
            }
        }

        this._menuOpenStateChangedId = this.systemMenu.menu.connect('open-state-changed', Lang.bind(this,
            function(menu, open) {
                if (!open)
                    return;
                this._altsuspendAction.visible = true;
                this._altpowerOffAction.visible = true;
            }));
    },

    disable: function() {
        this._settings.disconnect(this._toggleTwoButtonsID);
        this._settings.disconnect(this._toggleSuspendDefaultID);
        
        if (this._menuOpenStateChangedId) {
            this.systemMenu.menu.disconnect(this._menuOpenStateChangedId);
            this._menuOpenStateChangedId = 0;
        }

           this._destroyActions();
        if (this._settings.get_boolean(SUSPEND_DEFAULT)) {
              this._removealtStatusSwitcher();   
        }
        this._addDefaultButton();
    },
    
    _toggleTwoButtons: function() {
        if (this._settings.get_boolean(SHOW_TWO_BUTTONS)) {
            if (this._settings.get_boolean(SUSPEND_DEFAULT)) {
                this._removealtStatusSwitcher();
                this._destroyActions();
            }
            else {
                this._removealtSwitcher();
            }
            this._createActions();
            this._addSingleButtons();
        }
        else {
            this._destroyActions();
            if (this._settings.get_boolean(SUSPEND_DEFAULT)) {
                this._createActions();
                this._createaltStatusSwitcher();
            }
            else {
                this._addDefaultButton();
            }
        }
    },
    
    _toggleSuspendDefault: function() {
        if (!this._settings.get_boolean(SHOW_TWO_BUTTONS)) {
            if (this._settings.get_boolean(SUSPEND_DEFAULT)) {
                this._removealtSwitcher();
                this._createActions();
                this._createaltStatusSwitcher();
            }
            else {
                this._destroyActions();
                this._removealtStatusSwitcher();
                this._addDefaultButton();
            }
            
        }
    },
    
    _createActions: function() {
        this._altsuspendAction = this.systemMenu._createActionButton('media-playback-pause-symbolic', _("Suspend"));
        this._altsuspendActionID = this._altsuspendAction.connect('clicked', Lang.bind(this, this._onSuspendClicked));

        this._altpowerOffAction = this.systemMenu._createActionButton('system-shutdown-symbolic', _("Power Off"));
        this._altpowerOffActionId = this._altpowerOffAction.connect('clicked', Lang.bind(this, this._onPowerOffClicked));
    },
    
    _destroyActions: function() {
        if (this._altsuspendActionId) {
            this._altsuspendAction.disconnect(this._altsuspendActionId);
            this._altsuspendActionId = 0;
        }

        if (this._altpowerOffActionId) {
            this._altpowerOffAction.disconnect(this._altpowerOffActionId);
            this._altpowerOffActionId = 0;
        }
        
        if (this._altsuspendAction) {
            this._altsuspendAction.destroy();
            this._altsuspendAction = 0;
        }

        if (this._altpowerOffAction) {
            this._altpowerOffAction.destroy();
            this._altpowerOffAction = 0;
        }
    },
    
    _addDefaultButton: function() {
        this.systemMenu._actionsItem.actor.add(this.systemMenu._altSwitcher.actor, { expand: true, x_fill: false });
    },
    
    _addSingleButtons: function() {
        this.systemMenu._actionsItem.actor.add(this._altsuspendAction, { expand: true, x_fill: false });
        this.systemMenu._actionsItem.actor.add(this._altpowerOffAction, { expand: true, x_fill: false });
    },
    
    _removealtSwitcher: function() {
        this.systemMenu._actionsItem.actor.remove_child(this.systemMenu._altSwitcher.actor);
    },
    
    _createaltStatusSwitcher: function() {
        this._altStatusSwitcher = new StatusSystem.AltSwitcher(this._altsuspendAction,this._altpowerOffAction);
        this.systemMenu._actionsItem.actor.add(this._altStatusSwitcher.actor, { expand: true, x_fill: false });
    },
    
    _removealtStatusSwitcher: function() {
        if (this._altStatusSwitcher) {
            this.systemMenu._actionsItem.actor.remove_child(this._altStatusSwitcher.actor);
            this._altStatusSwitcher.actor.destroy();
            this._altStatusSwitcher = 0;
        }
    },
    
    _onPowerOffClicked: function() {
        this.systemMenu._onPowerOffClicked()
    },

    _onSuspendClicked: function() {
        this.systemMenu._onSuspendClicked();
    }
});

function init(metadata) {
    Lib.initTranslations(Me);
    return new Extension();
}

