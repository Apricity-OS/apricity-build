const St = imports.gi.St;
const Shell = imports.gi.Shell;
const Config = imports.misc.config;
const Main = imports.ui.main;
const Mainloop = imports.mainloop;
const MessageTray = Main.messageTray;
const Lang = imports.lang;
const LayoutManager = Main.layoutManager;
const PointerWatcher = imports.ui.pointerWatcher;
const Tweener = imports.ui.tweener;
const ExtensionUtils = imports.misc.extensionUtils;
const Me = ExtensionUtils.getCurrentExtension();
const Convenience = Me.imports.convenience;

let panel = Main.layoutManager.panelBox;

// Constants from messageTray:
const ANIMATION_TIME = 0.2;
const IDLE_TIME = 1000;
const State = {
    HIDDEN:  0,
    SHOWING: 1,
    SHOWN:   2,
    HIDING:  3
};
const Urgency = {
    LOW: 0,
    NORMAL: 1,
    HIGH: 2,
    CRITICAL: 3
};
const TRAY_DWELL_CHECK_INTERVAL = 100;

let messageTray_showNotification = function() {
    this._notification = this._notificationQueue.shift();

    this._userActiveWhileNotificationShown = this.idleMonitor.get_idletime() <= IDLE_TIME;
    if (!this._userActiveWhileNotificationShown) {
        // If the user isn't active, set up a watch to let us know
        // when the user becomes active.
        this.idleMonitor.add_user_active_watch(Lang.bind(this, this._onIdleMonitorBecameActive));
    }

    this._notificationClickedId = this._notification.connect('done-displaying',
                                                             Lang.bind(this, this._escapeTray));
    this._notificationUnfocusedId = this._notification.connect('unfocused', Lang.bind(this, function() {
        this._updateState();
    }));
    this._notificationBin.child = this._notification.actor;

    this._notificationWidget.opacity = 0;
// > SimpleDock (Notifications to top)
    // this._notificationWidget.y = 0;
	let yTop = -global.screen_height;
	let yBottom = 0;
	this._notificationWidget.y = yTop;
// < SimpleDock
    this._notificationWidget.show();

    this._updateShowingNotification();

    let [x, y, mods] = global.get_pointer();
    // We save the position of the mouse at the time when we started showing the notification
    // in order to determine if the notification popped up under it. We make that check if
    // the user starts moving the mouse and _onNotificationHoverChanged() gets called. We don't
    // expand the notification if it just happened to pop up under the mouse unless the user
    // explicitly mouses away from it and then mouses back in.
    this._showNotificationMouseX = x;
    this._showNotificationMouseY = y;
    // We save the coordinates of the mouse at the time when we started showing the notification
    // and then we update it in _notificationTimeout(). We don't pop down the notification if
    // the mouse is moving towards it or within it.
    this._lastSeenMouseX = x;
    this._lastSeenMouseY = y;

    this._resetNotificationLeftTimeout();
};

let messageTray_hideNotification = function(animate) {
    this._notificationFocusGrabber.ungrabFocus();

    if (this._notificationExpandedId) {
        this._notification.disconnect(this._notificationExpandedId);
        this._notificationExpandedId = 0;
    }
// > SimpleDock (Notifications to top)
	let yPos = -global.screen_height;
// < SimpleDock
    if (this._notificationClickedId) {
        this._notification.disconnect(this._notificationClickedId);
        this._notificationClickedId = 0;
    }
    if (this._notificationUnfocusedId) {
        this._notification.disconnect(this._notificationUnfocusedId);
        this._notificationUnfocusedId = 0;
    }

    this._resetNotificationLeftTimeout();

    if (animate) {
        this._tween(this._notificationWidget, '_notificationState', State.HIDDEN,
// > SimpleDock (Notifications to top)
                    // { y: this.actor.height,
					{ y: yPos,
// < SimpleDock
                      opacity: 0,
                      time: ANIMATION_TIME,
                      transition: 'easeOutQuad',
                      onComplete: this._hideNotificationCompleted,
                      onCompleteScope: this
                    });
    } else {
        Tweener.removeTweens(this._notificationWidget);
// > SimpleDock (Notifications to top)
        // this._notificationWidget.y = this.actor.height;
		this._notificationWidget.y = yPos;
// < SimpleDock
        this._notificationWidget.opacity = 0;
        this._notificationState = State.HIDDEN;
        this._hideNotificationCompleted();
    }
};

let messageTray_updateShowingNotification = function() {
// > SimpleDock (Notifications to top)
    this._notification._table.remove_style_class_name('sd_notification');
    this._notification._table.remove_style_class_name('sd_notification_top');
    this._notification._table.remove_style_class_name('sd_notification_bottom');
    this._notification._table.add_style_class_name('sd_notification_top');
// < SimpleDock
        this._notification.acknowledged = true;
        this._notification.playSound();

        // We auto-expand notifications with CRITICAL urgency, or for which the relevant setting
        // is on in the control center.
        if (this._notification.urgency == Urgency.CRITICAL ||
            this._notification.source.policy.forceExpanded)
            this._expandNotification(true);
// > SimpleDock (Notifications to top)
    let yTop = panel.y + panel.height - global.screen_height;
    if (yTop < (-global.screen_height))
        yTop = -global.screen_height;
    let yBottom = -this._notificationWidget.height;
    this._notificationWidget.x = global.screen_width / 50;
// < SimpleDock
        // We tween all notifications to full opacity. This ensures that both new notifications and
        // notifications that might have been in the process of hiding get full opacity.
        //
        // We tween any notification showing in the banner mode to the appropriate height
        // (which is banner height or expanded height, depending on the notification state)
        // This ensures that both new notifications and notifications in the banner mode that might
        // have been in the process of hiding are shown with the correct height.
        //
        // We use this._showNotificationCompleted() onComplete callback to extend the time the updated
        // notification is being shown.

        let tweenParams = { opacity: 255,
// > SimpleDock (Notifications to top)
                            // y: -this._notificationWidget.height,
							y: yTop,
// < SimpleDock
                            time: ANIMATION_TIME,
                            transition: 'easeOutQuad',
                            onComplete: this._showNotificationCompleted,
                            onCompleteScope: this
                          };

        this._tween(this._notificationWidget, '_notificationState', State.SHOWN, tweenParams);
};

let messageTray_onNotificationExpanded = function() {
// > SimpleDock (Notifications to top)
    // let expandedY = - this._notificationWidget.height;
	let yTop = panel.y + panel.height - global.screen_height;
	if (yTop < (-global.screen_height))
	    yTop = -global.screen_height;
	let yBottom = -this._notificationWidget.height;
	let expandedY = yTop;
// < SimpleDock
    this._closeButton.show();

    // Don't animate the notification to its new position if it has shrunk:
    // there will be a very visible "gap" that breaks the illusion.
    if (this._notificationWidget.y < expandedY) {
        this._notificationWidget.y = expandedY;
    } else if (this._notification.y != expandedY) {
        // Tween also opacity here, to override a possible tween that's
        // currently hiding the notification.
        Tweener.addTween(this._notificationWidget,
                         { y: expandedY,
                           opacity: 255,
                           time: ANIMATION_TIME,
                           transition: 'easeOutQuad',
                           // HACK: Drive the state machine here better,
                           // instead of overwriting tweens
                           onComplete: Lang.bind(this, function() {
                               this._notificationState = State.SHOWN;
                           }),
                         });
    }
};

let messageTray_resetNotificationLeftTimeout = function() {
    this._useLongerNotificationLeftTimeout = false;
    if (this._notificationLeftTimeoutId) {
        Mainloop.source_remove(this._notificationLeftTimeoutId);
        this._notificationLeftTimeoutId = 0;
        this._notificationLeftMouseX = -1;
        this._notificationLeftMouseY = -1;
    }
};

const ModifiedMessageTray = new Lang.Class({
    Name: 'ModifiedMessageTray',

    _init: function(enabled) {

        if (enabled) {
            this.enable(false);
        }
    },

	initialize: function() {

// > SimpleDock (Notifications to top)
		this.testNotificationTimeout = undefined;
		this.originalNotificationWidgetX = MessageTray._notificationWidget.x;
		this.originalShowNotification = MessageTray._showNotification;
		this.originalHideNotification = MessageTray._hideNotification;
		this.originalUpdateShowingNotification = MessageTray._updateShowingNotification;
		this.originalOnNotificationExpanded = MessageTray._onNotificationExpanded;
		this.originalResetNotificationLeftTimeout = MessageTray._resetNotificationLeftTimeout;
// < SimpleDock

// > SimpleDock (Disable Message tray)
		this.originalDwell = null;
		this.watcher = null;
// < SimpleDock
		this.initialized = true;
	},

    enable: function(testNotification) {

		if (!this.initialized) { this.initialize(); }

// > SimpleDock (Notifications to top)
		MessageTray._showNotification = messageTray_showNotification;
		MessageTray._hideNotification = messageTray_hideNotification;
		MessageTray._updateShowingNotification = messageTray_updateShowingNotification;
		MessageTray._onNotificationExpanded = messageTray_onNotificationExpanded;
		MessageTray._resetNotificationLeftTimeout = messageTray_resetNotificationLeftTimeout;

		if (testNotification) {
			this.testNotification();
		}
// < SimpleDock

// > SimpleDock (Disable Message tray)
		this.originalDwell = MessageTray._trayDwellTimeout;
		if("_trayPressure" in LayoutManager) {
            LayoutManager._trayPressure._keybindingMode = Shell.KeyBindingMode.OVERVIEW;
        }
		MessageTray._trayDwellTimeout = function() { return false; };

		let pointerWatcher = PointerWatcher.getPointerWatcher();
        this.watcher = pointerWatcher.addWatch(TRAY_DWELL_CHECK_INTERVAL, this.checkPointer);
// < SimpleDock
    },

    disable: function() {

// > SimpleDock (Notifications to top)
    	if (this.testNotificationTimeout !== undefined)
        	Mainloop.source_remove(this.testNotificationTimeout);

		if (MessageTray._notification) {
		    MessageTray._notification._table.remove_style_class_name('sd_notification');
		    MessageTray._notification._table.remove_style_class_name('sd_notification_top');
		    MessageTray._notification._table.remove_style_class_name('sd_notification_bottom');
		}

		Main.messageTray._notificationWidget.x = this.originalNotificationWidgetX;
		MessageTray._showNotification = this.originalShowNotification;
		MessageTray._hideNotification = this.originalHideNotification;
		MessageTray._updateShowingNotification = this.originalUpdateShowingNotification;
		MessageTray._onNotificationExpanded = this.originalOnNotificationExpanded;
		MessageTray._resetNotificationLeftTimeout = this.originalResetNotificationLeftTimeout;
// < SimpleDock

// > SimpleDock (Disable Message tray)
        if("_trayPressure" in LayoutManager) {
            LayoutManager._trayPressure._keybindingMode = Shell.KeyBindingMode.NORMAL | Shell.KeyBindingMode.OVERVIEW;
        }
        if(this.originalDwell !== null) {
            MessageTray._trayDwellTimeout = this.originalDwell;
        }
		if (this.watcher !== null) {
			let pointerWatcher = PointerWatcher.getPointerWatcher();
        	pointerWatcher._removeWatch(this.watcher);
		}
// < SimpleDock
		this.initialized = false;
    },

    destroy: function() {
		this.disable();
    },

// > SimpleDock (Notifications to top)
	testNotification: function() {
        if (this.testNotificationTimeout !== undefined)
            Mainloop.source_remove(this.testNotificationTimeout);

    	this.testNotificationTimeout = Mainloop.timeout_add(250,
			Lang.bind(this, function() {
				Main.notify("Simple Dock", "This is just a notification example.\n\nLorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
        		return false;
    		}));
	},
// < SimpleDock

// > SimpleDock (Disable Message tray)
    showMessageTray: function() {
	    if (MessageTray._trayState === 0) {
		    MessageTray.openTray();
	    } else {
		    MessageTray.hide();
	    }
    },
// < SimpleDock

// > SimpleDock (Disable Message tray)
    checkPointer: function (x, y) {
		let monitor = Main.layoutManager.bottomMonitor;

		if (y !== monitor.y + monitor.height - 1) return; // Prevent absurd calculations

		let shouldDwell = (x >= monitor.x && x <= monitor.x + monitor.width
				  && (x <= (monitor.x + 2) || x >= (monitor.x + monitor.width - 2)));

		if (shouldDwell && MessageTray._trayState === 0) {
			MessageTray.openTray();
		}
    }
// < SimpleDock
});

