const Clutter = imports.gi.Clutter;
const Lang = imports.lang;
const Main = imports.ui.main;
const ExtensionUtils = imports.misc.extensionUtils;
const Extension = ExtensionUtils.getCurrentExtension();
const Settings = Extension.imports.settings;
const Meta = imports.gi.Meta;
const Shell = imports.gi.Shell;

const BUFFER_SHOW_ALL_WORKSPACES = 0;
const BUFFER_IGNORE_LAST_WORKSPACE = 1;

function Ext() {
	this._init.apply(this, arguments);
}

Ext.prototype = {
	_init: function(){
		this._panel = Main.panel;
		this._panelBinding = null;
		this._lastScroll = Date.now();

		let self = this;
		// setup ignore-last-workspace pref
		this._prefs = new Settings.Prefs();
		(function() {
			let pref = self._prefs.IGNORE_LAST_WORKSPACE;
			let update = function() {
				self._tailBuffer = pref.get() ? BUFFER_IGNORE_LAST_WORKSPACE : BUFFER_SHOW_ALL_WORKSPACES ;
			};
			pref.changed(update);
			update(); // set initial value
		}
		)();

		// setup scroll-delay pref
		(function() {
			let pref = self._prefs.SCROLL_DELAY;
			let update = function() {
				self._scroll_delay = pref.get();
			};
			pref.changed(update);
			update(); // set initial value
		}
		)();
	},

	disable: function() {
		if (this._panelBinding) {
			this._panel.actor.disconnect(this._panelBinding);
			this._panelBinding = null;
		}
	},

	enable: function() {
		this._panel.reactive = true;
		if (this._panelBinding) {
			// enabled twice in a row? should be impossible
			this.disable();
		}
		this._panelBinding = this._panel.actor.connect('scroll-event', Lang.bind(this, this._onScrollEvent));
	},

	_onScrollEvent : function(actor, event) {
		let source = event.get_source();
		if (source != actor) {
			// Actors in the status area often have their own scroll events,
			let inStatusArea = this._panel._rightBox && this._panel._rightBox.contains && this._panel._rightBox.contains(source);
			if (inStatusArea) return Clutter.EVENT_PROPAGATE;
		}

		let motion;
		switch (event.get_scroll_direction()) {
		case Clutter.ScrollDirection.UP:
			motion = Meta.MotionDirection.UP;
			break;
		case Clutter.ScrollDirection.DOWN:
			motion = Meta.MotionDirection.DOWN;
			break;
		case Clutter.ScrollDirection.LEFT:
			motion = Meta.MotionDirection.LEFT;
			break;
		case Clutter.ScrollDirection.RIGHT:
			motion = Meta.MotionDirection.RIGHT;
			break;
		default:
			return Clutter.EVENT_PROPAGATE;
		}
		let activeWs = global.screen.get_active_workspace();
		let ws = activeWs.get_neighbor(motion);
		if(!ws) return Clutter.EVENT_STOP;

		let currentTime = Date.now();
		
		// global.log("scroll time diff = " + (currentTime - this._lastScroll));
		if (currentTime < this._lastScroll + this._scroll_delay) {
			if (currentTime < this._lastScroll) {
				// Clock went backwards. Reset & accept event
				this._lastScroll = 0;
			} else {
				// within wait period - consume this event (but do nothing)
				// to prevent accidental rapid scrolling
				return Clutter.EVENT_STOP;
			}
		}

		let tailBuffer = Main.overview.visible ? BUFFER_SHOW_ALL_WORKSPACES : this._tailBuffer;
		if (ws.index() < global.screen.n_workspaces - tailBuffer) {
			this._lastScroll = currentTime;
			Main.wm.actionMoveWorkspace(ws);
		}
		return Clutter.EVENT_STOP;
	},
}

function init(meta) {
	let ext = new Ext();
	return ext;
}

function main() {
	init().enable();
};
