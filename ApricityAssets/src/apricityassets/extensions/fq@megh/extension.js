const Lang = imports.lang;
const St = imports.gi.St;
const Main = imports.ui.main;

const GLib = imports.gi.GLib;

let button;

function init() {
    button = new St.Bin({ style_class: 'panel-button',
                          reactive: true,
                          can_focus: true,
                          x_fill: true,
                          y_fill: false,
                          track_hover: true });
    let icon = new St.Icon({ icon_name: 'window-close',
                             style_class: 'system-status-icon' });

    button.set_child(icon);
    button.connect('button-release-event',   function () {
	GLib.spawn_command_line_async('xkill');
        });
}

function enable() {
	let appMenu=Main.panel.statusArea.appMenu.actor.get_parent();
	Main.panel._leftBox.insert_child_below(button, appMenu);
	/*change this to below if you want to add it before the appmenu button*/
}

function disable() {
    Main.panel._leftBox.remove_child(button);
}
