const Gio = imports.gi.Gio;
const Gtk = imports.gi.Gtk;
const Config = imports.misc.config;

const Me = imports.misc.extensionUtils.getCurrentExtension();
const _ = imports.gettext.domain(Me.uuid).gettext;

// 3.14 Remove until '<' when losing compatibility
const SETTINGS_CHANGE_MESSAGE_TRAY = "change-message-tray";
// <
const SETTINGS_SHOW_APPS_BUTTON = "show-apps-button";
const SETTINGS_SHOW_METHOD = "show-method";
const SETTINGS_MAX_ICON_SIZE = "max-icon-size";
const SETTINGS_BACKGROUND_OPACITY = "background-opacity";

let settings;

function init() {

    imports.gettext.bindtextdomain(Me.uuid, Me.path + "/locale");
    const GioSSS = Gio.SettingsSchemaSource;

    let schemaSource = GioSSS.new_from_directory(Me.path + "/schemas", 
            GioSSS.get_default(), false);

    let schemaObj = schemaSource.lookup(Me.metadata["settings-schema"], true);
    if(!schemaObj) {
        throw new Error("Schema " + Me.metadata["settings-schema"] + " could not be found for extension " +
                        Me.uuid + ". Please check your installation.");
    }

    settings = new Gio.Settings({ settings_schema: schemaObj });
}

function buildPrefsWidget() {

    // 3.14 Remove until '<' when losing compatibility
    let settingLabel = new Gtk.Label({ xalign: 1, label: _("Adapt message bar and notifications") + ":" });
    let settingSwitch = new Gtk.Switch({
		hexpand: false,
        active: settings.get_boolean(SETTINGS_CHANGE_MESSAGE_TRAY)
    });
    settingSwitch.connect("notify::active", function(button) {
        settings.set_boolean(SETTINGS_CHANGE_MESSAGE_TRAY, button.active);
    });
    settingSwitch.set_tooltip_text(_("Sets bottom message tray corners and moves notifications to the top bar"));
    // < 

    let showAppsLabel = new Gtk.Label({ xalign: 1, label: _("Show applications button") + "*:" });
    let showAppsSwitch = new Gtk.Switch({
		hexpand: false,
        active: settings.get_boolean(SETTINGS_SHOW_APPS_BUTTON)
    });
    showAppsSwitch.connect("notify::active", function(button) {
        settings.set_boolean(SETTINGS_SHOW_APPS_BUTTON, button.active);
    });
    showAppsSwitch.set_tooltip_text(_("Show applications button"));

    let allSizes  =[ 16, 24, 32, 48, 64, 96, 128 ];
    let iconSizeLabel = new Gtk.Label({ xalign: 1, label: _("Maximum icon size") + ":" });
    let settingIconCombo = new Gtk.ComboBoxText({ halign:Gtk.Align.START });
    settingIconCombo.append_text(_("16"));
    settingIconCombo.append_text(_("24"));
    settingIconCombo.append_text(_("32"));
    settingIconCombo.append_text(_("48"));
    settingIconCombo.append_text(_("64"));
    settingIconCombo.append_text(_("96"));
    settingIconCombo.append_text(_("128"));
    settingIconCombo.set_active(allSizes.indexOf(settings.get_int(SETTINGS_MAX_ICON_SIZE)));
    settingIconCombo.connect("changed", function(widget) {
        settings.set_int(SETTINGS_MAX_ICON_SIZE, allSizes[widget.get_active()]);
    });
    settingIconCombo.set_tooltip_text(_("Maximum icon size"));

    let opacityLabel = new Gtk.Label({ xalign: 1, label: _("Background opacity") + ":" });
    let opacitySlider =  new Gtk.Scale({orientation: Gtk.Orientation.HORIZONTAL, valuePos: Gtk.PositionType.RIGHT});
    opacitySlider.set_range(0, 100);
    opacitySlider.set_value(settings.get_double(SETTINGS_BACKGROUND_OPACITY) * 100);
    opacitySlider.set_digits(0);
    opacitySlider.set_increments(5,5);
    opacitySlider.set_size_request(150, -1);
    opacitySlider.connect("value-changed", function(widget) {
            settings.set_double(SETTINGS_BACKGROUND_OPACITY, widget.get_value()/100);
        });
    opacitySlider.set_tooltip_text(_("Sets background opacity"));

    let showMethodLabel = new Gtk.Label({ xalign: 1, label: _("Show method") + ":" });
    let showMethod0 = new Gtk.RadioButton ({ label: _("Hide if focused window overlaps the dock") });
    let showMethod1 = new Gtk.RadioButton ({ group: showMethod0, label: _("Hide when not being used") });
    let showMethod2 = new Gtk.RadioButton ({ group: showMethod0, label: _("Always show") });
    switch (settings.get_int(SETTINGS_SHOW_METHOD)) {
    case 0:
        showMethod0.set_active (true);
        break;
    case 1:
        showMethod1.set_active (true);
        break;
    case 2:
        showMethod2.set_active (true);
        break;
    }
    showMethod0.connect("toggled", function(widget) { 
        if (widget.get_active()) { settings.set_int(SETTINGS_SHOW_METHOD, 0); }
    });
    showMethod1.connect("toggled", function(widget) { 
        if (widget.get_active()) { settings.set_int(SETTINGS_SHOW_METHOD, 1); }
    });
    showMethod2.connect("toggled", function(widget) { 
        if (widget.get_active()) { settings.set_int(SETTINGS_SHOW_METHOD, 2); }
    });

    let infoLabel = new Gtk.Label({
        halign: Gtk.Align.CENTER,
        label: _("*Settings are accessible from 'Applications button', use 'Tweak-Tool' when not shown"),
        margin_top: 15
    });
	infoLabel.set_justify(Gtk.Justification.CENTER);

    grid = new Gtk.Grid({ column_spacing: 25, halign: Gtk.Align.CENTER, margin: 10, row_spacing: 10 });
    grid.set_border_width(15);
    // 3.14 Remove until '<' when losing compatibility
    if (Config.PACKAGE_VERSION.indexOf("3.14.") !== -1) {
        grid.attach(settingLabel,       0, 0, 1, 1);
        grid.attach(settingSwitch,      1, 0, 1, 1);
    }
    // <
    grid.attach(showAppsLabel,      0, 1, 1, 1);
    grid.attach(showAppsSwitch,     1, 1, 1, 1);
    grid.attach(iconSizeLabel,      0, 2, 1, 1);
    grid.attach(settingIconCombo,   1, 2, 1, 1);
    grid.attach(opacityLabel,       0, 3, 1, 1);
    grid.attach(opacitySlider,      1, 3, 2, 1);
    grid.attach(showMethodLabel,    0, 4, 1, 1);
    grid.attach(showMethod0,        1, 4, 4, 1);
    grid.attach(showMethod1,        1, 5, 4, 1);
    grid.attach(showMethod2,        1, 6, 4, 1);
    grid.attach(infoLabel,          0, 7, 6, 1);
    grid.show_all();

    return grid;
}
