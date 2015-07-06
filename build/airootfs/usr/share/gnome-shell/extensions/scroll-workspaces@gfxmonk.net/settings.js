const Gio = imports.gi.Gio;
const ExtensionUtils = imports.misc.extensionUtils;
const Extension = ExtensionUtils.getCurrentExtension();

const SCHEMA_PATH = 'org.gnome.shell.extensions.net.gfxmonk.scroll-workspaces';

function get_local_gsettings(schema_path) {
	const GioSSS = Gio.SettingsSchemaSource;

	let schemaDir = Extension.dir.get_child('schemas');
	let schemaSource = GioSSS.new_from_directory(
		schemaDir.get_path(),
		GioSSS.get_default(),
		false);

	let schemaObj = schemaSource.lookup(schema_path, true);
	if (!schemaObj) {
		throw new Error(
			'Schema ' + schema_path +
			' could not be found for extension ' +
			Extension.metadata.uuid
		);
	}
	return new Gio.Settings({ settings_schema: schemaObj });
};

function Prefs() {
	let self = this;
	let settings = this.settings = get_local_gsettings(SCHEMA_PATH);
	this.IGNORE_LAST_WORKSPACE = {
		key: 'ignore-last-workspace',
		get: function() { return settings.get_boolean(this.key); },
		set: function(v) { settings.set_boolean(this.key, v); },
		changed: function(cb) { return settings.connect('changed::' + this.key, cb); },
		disconnect: function() { return settings.disconnect.apply(settings, arguments); },
	};

	this.SCROLL_DELAY = {
		key: 'scroll-delay',
		get: function() { return settings.get_int(this.key); },
		set: function(v) { settings.set_int(this.key, v); },
		changed: function(cb) { return settings.connect('changed::' + this.key, cb); },
		disconnect: function() { return settings.disconnect.apply(settings, arguments); },
	};
};
