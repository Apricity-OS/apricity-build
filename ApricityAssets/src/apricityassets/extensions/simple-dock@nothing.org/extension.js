const Config = imports.misc.config;
const St = imports.gi.St;
const Shell = imports.gi.Shell;
const Main = imports.ui.main;
const LayoutManager = Main.layoutManager;
const ExtensionUtils = imports.misc.extensionUtils;
const Me = ExtensionUtils.getCurrentExtension();
const Gio = imports.gi.Gio;
const GnomeDash = Me.imports.gnomedash;
const Intellihide = Me.imports.intellihide;
const AtomDock = Me.imports.atomdock;
// 3.14 Delete files 'messageTrayModified.js' and 'stylesheet.css'
// 3.14 Remove until '<' when losing compatibility 
if (Config.PACKAGE_VERSION.indexOf("3.14.") !== -1) {
    const ModifiedMessageTray = Me.imports.messageTrayModified;
    let modifiedMessageTray;
    let settingsTraySignal = null;
    const SETTINGS_CHANGE_MESSAGE_TRAY = "change-message-tray";
}
// <
let oldDash;
let atomDock;
let intellihide;
let settings = null;
let settingsAppsButtonSignal = null;
let settingsShowMethodSignal = null;
let settingsIconSignal = null;
let settingsBackgroundSignal = null;

// Settings
const SETTINGS_SHOW_APPS_BUTTON = "show-apps-button";
const SETTINGS_SHOW_METHOD = "show-method";
const SETTINGS_MAX_ICON_SIZE = "max-icon-size";
const SETTINGS_BACKGROUND_OPACITY = "background-opacity";
// 3.14 Remove until '<' when losing compatibility
function setSettingsNotifications() {
    if (settings.get_boolean(SETTINGS_CHANGE_MESSAGE_TRAY)) {
        modifiedMessageTray.enable(true);
    } else {
        modifiedMessageTray.disable(true);
    }
}
// <
function settingsAppsButtonChanged() {
    atomDock.setShowAppsButton(settings.get_boolean(SETTINGS_SHOW_APPS_BUTTON));
}

function settingsShowMethodChanged() {
    intellihide.setShowMethod(settings.get_int(SETTINGS_SHOW_METHOD));
}

function settingsIconChanged() {
    atomDock.setMaxIconSize(settings.get_int(SETTINGS_MAX_ICON_SIZE), true);
}

function settingsBackgroundChanged() {
    atomDock.setBackgroundOpacity(settings.get_double(SETTINGS_BACKGROUND_OPACITY), true);
}

// Init settings
function initSettings() {
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

function init() {
    oldDash = new GnomeDash.GnomeDash();
    initSettings();
}

function show() {
    atomDock.intelliShow();
}

function hide() {
    atomDock.intelliHide();
}

function retop() {
    atomDock.retop();
}

function enable() {
    // Hide old dash
    oldDash.hideDash();

    // Enable new dock
    atomDock = new AtomDock.AtomDock();
    let iconSize = settings.get_int(SETTINGS_MAX_ICON_SIZE);
    if (iconSize) {
        atomDock.setMaxIconSize(iconSize, false);
    } else {
        atomDock.setMaxIconSize(48, false);
    }
    let appsButton = settings.get_boolean(SETTINGS_SHOW_APPS_BUTTON);
    if (typeof appsButton !== 'undefined') {
        atomDock.setShowAppsButton(appsButton);
    } else {
        atomDock.setShowAppsButton(true);
    }

	let backOpacity = settings.get_double(SETTINGS_BACKGROUND_OPACITY);
    atomDock.setBackgroundOpacity(backOpacity, true);

    intellihide = new Intellihide.Intellihide(show, hide, retop, atomDock);
    let showMethod = settings.get_int(SETTINGS_SHOW_METHOD);
    if (showMethod) {
        intellihide.setShowMethod(showMethod);
    } else {
        intellihide.setShowMethod(0);
    }
// 3.14 Remove until '<' when losing compatibility
    if (Config.PACKAGE_VERSION.indexOf("3.14.") !== -1) {
        modifiedMessageTray = new ModifiedMessageTray.ModifiedMessageTray(settings.get_boolean(SETTINGS_CHANGE_MESSAGE_TRAY));

        settingsTraySignal = settings.connect("changed::" + SETTINGS_CHANGE_MESSAGE_TRAY,
            function() { setSettingsNotifications(); });
    }
// <
    settingsAppsButtonSignal = settings.connect("changed::" + SETTINGS_SHOW_APPS_BUTTON,
        function() { settingsAppsButtonChanged(); });

    settingsShowMethodSignal = settings.connect("changed::" + SETTINGS_SHOW_METHOD,
        function() { settingsShowMethodChanged(); });

    settingsIconSignal = settings.connect("changed::" + SETTINGS_MAX_ICON_SIZE,
        function() { settingsIconChanged(); });

    settingsBackgroundSignal = settings.connect("changed::" + SETTINGS_BACKGROUND_OPACITY,
        function() { settingsBackgroundChanged(); });
}

function disable() {

    intellihide.destroy();
    atomDock.destroy();
    oldDash.showDash();
// 3.14 Remove until '<' when losing compatibility
    if (Config.PACKAGE_VERSION.indexOf("3.14.") !== -1) {
	    modifiedMessageTray.destroy();

        if(settingsTraySignal != null) {
            settings.disconnect(settingsTraySignal);
            settingsTraySignal = null;
        }
    }
// <
    if(settingsAppsButtonSignal != null) {
        settings.disconnect(settingsAppsButtonSignal);
        settingsAppsButtonSignal = null;
    }

    if(settingsShowMethodSignal != null) {
        settings.disconnect(settingsShowMethodSignal);
        settingsShowMethodSignal = null;
    }

    if(settingsIconSignal != null) {
        settings.disconnect(settingsIconSignal);
        settingsIconSignal = null;
    }

    if(settingsBackgroundSignal != null) {
        settings.disconnect(settingsBackgroundSignal);
        settingsBackgroundSignal = null;
    }
}
