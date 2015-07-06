const Gtk = imports.gi.Gtk;

let Extension = imports.misc.extensionUtils.getCurrentExtension();
let Settings = Extension.imports.settings;

function init() {
}

function buildPrefsWidget() {
	let config = new Settings.Prefs();
	let frame = new Gtk.Box({
		orientation: Gtk.Orientation.VERTICAL,
		border_width: 10
	});

	(function() {
		let hbox = new Gtk.Box({
			orientation: Gtk.Orientation.HORIZONTAL,
			spacing: 20
		});

		let label = new Gtk.Label({
			label: "Ignore last workspace:",
			use_markup: false,
		});
		let checkbutton = new Gtk.CheckButton();

		hbox.add(label);
		hbox.pack_end(checkbutton, true, true, 0);
		frame.add(hbox);

		var pref = config.IGNORE_LAST_WORKSPACE;
		checkbutton.set_active(pref.get());
		checkbutton.connect('toggled', function(sw) {
			var oldval = pref.get();
			var newval = sw.get_active();
			if (newval != pref.get()) {
				pref.set(newval);
			}
		});
	})();

	(function() {
		let hbox = new Gtk.Box({
			orientation: Gtk.Orientation.HORIZONTAL,
			spacing: 20
		});

		let label = new Gtk.Label({
			label: "Minimum delay between scroll events (ms)\n<small>(prevents accidental double-scrolling)</small>",
			use_markup: true,
		});
		let adjustment = new Gtk.Adjustment({
			lower: 0,
			upper: 500,
			step_increment: 10
		});
		let scale = new Gtk.HScale({
			digits:0,
			adjustment: adjustment,
			value_pos: Gtk.PositionType.RIGHT
		});

		hbox.add(label);
		hbox.pack_end(scale, true, true, 0);
		frame.add(hbox);

		var pref = config.SCROLL_DELAY;
		scale.set_value(pref.get());
		scale.connect('value-changed', function(sw) {
			var oldval = pref.get();
			var newval = sw.get_value();
			if (newval != pref.get()) {
				pref.set(newval);
			}
		});
	})();

	frame.show_all();
	return frame;
}
