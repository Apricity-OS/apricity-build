const Lang = imports.lang;
const Clutter = imports.gi.Clutter;
const Shell = imports.gi.Shell;
const Meta = imports.gi.Meta;

const Main = imports.ui.main;

const ExtensionUtils = imports.misc.extensionUtils;
const Me = ExtensionUtils.getCurrentExtension();
const Prefs = Me.imports.prefs;

const WorkspaceScroller = new Lang.Class({
	Name: 'WorkspaceScroller',

	_init: function() {
		Main.panel.actor.reactive = true;
		this._panelScrollEventId = Main.panel.actor.connect('scroll-event', Lang.bind(this, this._onScrollEvent));
		this._lastScrollTime = global.get_current_time();
	},

	destroy: function() {
		if (this._panelScrollEventId) {
			Main.panel.actor.disconnect(this._panelScrollEventId);
			this._panelScrollEventId = 0;
		}
	},

	get _delay() {
		return Prefs.settings.get_int('scroll-delay');
	},
	get _noLast() {
		return Prefs.settings.get_boolean('ignore-last-workspace');
	},

	_activate: function(ws) {
		if (ws.index() == global.screen.n_workspaces - 1 && !Main.overview.visible && this._noLast) {
			return;
		}
		this._lastScrollTime = global.get_current_time();
		Main.wm.actionMoveWorkspace(ws);
	},

	_onScrollEvent: function(actor, event) {
		let source = event.get_source();
		if (!(source instanceof Shell.GenericContainer)) {
			// Actors in the "status" area may have their own scroll events
			return Clutter.EVENT_PROPAGATE;
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

		if (global.get_current_time() >= this._lastScrollTime + this._delay) {
			// Ensure a minimum delay between workspace scrolls
			this._activate(ws);
		}
		return Clutter.EVENT_STOP;
	}
});

function init(meta) {
	/* do nothing */
}

let _scroller;

function enable() {
	_scroller = new WorkspaceScroller();
}

function disable() {
	_scroller.destroy();
}
