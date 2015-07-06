const Gio = imports.gi.Gio;
const Gtk = imports.gi.Gtk;
const Lang = imports.lang;

const ExtensionUtils = imports.misc.extensionUtils;
const Me = ExtensionUtils.getCurrentExtension();

const PREFS_UI = 'prefs.ui';

function _getSettings() {
	let prefsSchema = Me.metadata['settings-schema'];
	let localSchemas = Me.dir.get_child('schemas').get_path();
	let systemSchemas = Gio.SettingsSchemaSource.get_default();
	let schemaSource = Gio.SettingsSchemaSource.new_from_directory(localSchemas, systemSchemas, true);
	let schemaObj = schemaSource.lookup(prefsSchema, true);
	if (!schemaObj) {
		throw new Error('Schema ' + prefsSchema + ' could not be found for extension ' + Me.metadata.uuid + '. Please check your installation.');
	}
	return new Gio.Settings({ settings_schema: schemaObj });
}

let settings = _getSettings();

const WorkspaceScrollerPrefsWidget = new Lang.Class({
	Name: 'WorkspaceScrollerPrefsWidget',

	_init: function() {
		this._builder = new Gtk.Builder();
		let noLast = settings.get_boolean('ignore-last-workspace');
		let delay = settings.get_int('scroll-delay');
		settings.set_boolean('ignore-last-workspace', noLast);
		settings.set_int('scroll-delay', delay);
	},

	_updateWidget: function() {
		let noLast = settings.get_boolean('ignore-last-workspace');
		let delay = settings.get_int('scroll-delay');
		this._builder.get_object('lastworkspaceswitch').set_active(noLast);
		this._builder.get_object('scrolldelayscale').set_value(delay);
	},

	buildPrefsWidget: function() {
		this._builder.add_from_file(Me.dir.get_path() + '/' + PREFS_UI);
		this._updateWidget();
		let settingsChangedId = settings.connect('changed', Lang.bind(this, this._updateWidget));
		this._builder.get_object('lastworkspaceswitch').connect('notify::active', Lang.bind(this, function(widget) {
			settings.set_boolean('ignore-last-workspace', widget.get_active());
		}));
		this._builder.get_object('scrolldelayscale').connect('value-changed', Lang.bind(this, function(widget) {
			settings.set_int('scroll-delay', widget.get_value());
		}));
		return this._builder.get_object('mainbox');
	}
});

let _prefsWidget;

function init() {
	_prefsWidget = new WorkspaceScrollerPrefsWidget();
}

function buildPrefsWidget() {
	return _prefsWidget.buildPrefsWidget();
}
