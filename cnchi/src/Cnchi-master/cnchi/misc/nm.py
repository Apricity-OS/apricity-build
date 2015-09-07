#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
#  nm.py
#
#  Copyright (C) 2012 Canonical Ltd.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

import string
import subprocess

import dbus
from dbus.mainloop.glib import DBusGMainLoop

DBusGMainLoop(set_as_default=True)
from gi.repository import Gtk, GObject, GLib


NM = 'org.freedesktop.NetworkManager'
NM_DEVICE = 'org.freedesktop.NetworkManager.Device'
NM_DEVICE_WIFI = 'org.freedesktop.NetworkManager.Device.Wireless'
NM_AP = 'org.freedesktop.NetworkManager.AccessPoint'
NM_SETTINGS = 'org.freedesktop.NetworkManager.Settings'
NM_SETTINGS_CONN = 'org.freedesktop.NetworkManager.Settings.Connection'
NM_SETTINGS_PATH = '/org/freedesktop/NetworkManager/Settings'
NM_ERROR_NOSECRETS = 'org.freedesktop.NetworkManager.AgentManager.NoSecrets'
DEVICE_TYPE_WIRED = 1
DEVICE_TYPE_WIFI = 2
NM_STATE_DISCONNECTED = 20
NM_STATE_CONNECTING = 40
NM_STATE_CONNECTED_GLOBAL = 70


# TODO: DBus exceptions.  Catch 'em all.

def decode_ssid(characters):
    return bytearray(characters).decode('UTF-8', 'replace')


def get_prop(obj, iface, prop):
    try:
        return obj.Get(iface, prop, dbus_interface=dbus.PROPERTIES_IFACE)
    except dbus.DBusException as e:
        if e.get_dbus_name() == 'org.freedesktop.DBus.Error.UnknownMethod':
            return None
        else:
            raise


def get_vendor_and_model(udi):
    vendor = ''
    model = ''
    cmd = ['udevadm', 'info', '--path={0}'.format(udi), '--query=property']
    with open('/dev/null', 'w') as devnull:
        out = subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stderr=devnull,
            universal_newlines=True)
        out = out.communicate()
    if not out[1]:
        for prop in out[0].split('\n'):
            if prop.startswith('ID_VENDOR_FROM_DATABASE'):
                vendor = prop.split('ID_VENDOR_FROM_DATABASE=')[1]
            elif prop.startswith('ID_MODEL_FROM_DATABASE'):
                model = prop.split('ID_MODEL_FROM_DATABASE=')[1]
    return vendor, model


def wireless_hardware_present():
    # NetworkManager keeps DBus objects for wireless devices around even when
    # the hardware switch is off.
    bus = dbus.SystemBus()
    manager = bus.get_object(NM, '/org/freedesktop/NetworkManager')
    try:
        devices = manager.GetDevices()
    except dbus.DBusException:
        return False
    for device_path in devices:
        device_obj = bus.get_object(NM, device_path)
        if get_prop(device_obj, NM_DEVICE, 'DeviceType') == DEVICE_TYPE_WIFI:
            return True
    return False


class NetworkManager:
    def __init__(self, model, state_changed=None):
        self.model = model
        self.timeout_id = 0
        self.start(state_changed)
        self.active_connection = None
        self.active_device_obj = None
        self.active_conn = None
        self.bus = None
        self.manager = None
        self.passphrases_cache = {}

    def start(self, state_changed=None):
        self.bus = dbus.SystemBus()
        self.manager = self.bus.get_object(
            NM, '/org/freedesktop/NetworkManager')
        add = self.bus.add_signal_receiver
        add(self.queue_build_cache, 'AccessPointAdded', NM_DEVICE_WIFI, NM)
        add(self.queue_build_cache, 'AccessPointRemoved', NM_DEVICE_WIFI, NM)
        if state_changed:
            add(state_changed, 'StateChanged', NM, NM)
        add(self.queue_build_cache, 'DeviceAdded', NM, NM)
        add(self.queue_build_cache, 'DeviceRemoved', NM, NM)
        add(self.properties_changed, 'PropertiesChanged', NM_AP,
            path_keyword='path')
        self.build_cache()
        self.build_passphrase_cache()

    def get_state(self):
        return self.manager.state()

    def is_connected(self, device, ap):
        device_obj = self.bus.get_object(NM, device)
        connectedap = get_prop(device_obj, NM_DEVICE_WIFI, 'ActiveAccessPoint')
        if not connectedap:
            return False
        connect_obj = self.bus.get_object(NM, connectedap)
        ssid = get_prop(connect_obj, NM_AP, 'Ssid')
        if ssid:
            return ap == decode_ssid(ssid)
        else:
            return False

    def connect_to_ap(self, device, ap, passphrase=None):
        device_obj = self.bus.get_object(NM, device)
        ap_list = device_obj.GetAccessPoints(dbus_interface=NM_DEVICE_WIFI)
        saved_strength = 0
        saved_path = ''
        for ap_path in ap_list:
            ap_obj = self.bus.get_object(NM, ap_path)
            ssid = get_prop(ap_obj, NM_AP, 'Ssid')
            if ssid:
                strength = get_prop(ap_obj, NM_AP, 'Strength')
                if decode_ssid(ssid) == ap and saved_strength < strength:
                    # Connect to the strongest AP.
                    saved_strength = strength
                    saved_path = ap_path
        if not saved_path:
            return

        obj = dbus.Dictionary(signature='sa{sv}')
        if passphrase:
            obj['802-11-wireless-security'] = {'psk': passphrase}
        self.active_conn, self.active_connection = (
            self.manager.AddAndActivateConnection(
                obj, dbus.ObjectPath(device), dbus.ObjectPath(saved_path),
                signature='a{sa{sv}}oo'))
        self.active_device_obj = device_obj

    def disconnect_from_ap(self):
        if self.active_connection is not None:
            self.manager.DeactivateConnection(self.active_connection)
            self.active_connection = None
        if self.active_device_obj is not None:
            self.active_device_obj.Disconnect()
            self.active_device_obj = None
        if self.active_conn is not None:
            conn_obj = self.bus.get_object(NM, self.active_conn)
            conn_obj.Delete()
            self.active_conn = None

    def build_passphrase_cache(self):
        self.passphrases_cache = {}
        settings_obj = self.bus.get_object(NM, NM_SETTINGS_PATH)
        for conn in settings_obj.ListConnections(dbus_interface=NM_SETTINGS):
            conn_obj = self.bus.get_object(NM, conn)
            props = conn_obj.GetSettings(dbus_interface=NM_SETTINGS_CONN)
            if '802-11-wireless-security' in props:
                try:
                    sec = conn_obj.GetSecrets('802-11-wireless-security',
                                              dbus_interface=NM_SETTINGS_CONN)
                    sec = list(sec['802-11-wireless-security'].values())[0]
                    ssid = decode_ssid(props['802-11-wireless']['ssid'])
                    self.passphrases_cache[ssid] = sec
                except dbus.exceptions.DBusException as e:
                    if e.get_dbus_name() != NM_ERROR_NOSECRETS:
                        raise

    def ssid_in_model(self, iterator, ssid, security):
        i = self.model.iter_children(iterator)
        while i:
            row = self.model[i]
            if row[0] == ssid and row[1] == security:
                return i
            i = self.model.iter_next(i)
        return None

    def prune(self, iterator, ssids):
        to_remove = []
        while iterator:
            ssid = self.model[iterator][0]
            if ssid not in ssids:
                to_remove.append(iterator)
            iterator = self.model.iter_next(iterator)
        for iterator in to_remove:
            self.model.remove(iterator)

    def queue_build_cache(self, *args):
        if self.timeout_id:
            GLib.source_remove(self.timeout_id)
        self.timeout_id = GLib.timeout_add(500, self.build_cache)

    def properties_changed(self, props, path=None):
        if 'Strength' in props:
            ap_obj = self.bus.get_object(NM, path)
            ssid = get_prop(ap_obj, NM_AP, 'Ssid')
            if ssid:
                ssid = decode_ssid(ssid)
                security = (get_prop(ap_obj, NM_AP, 'WpaFlags') != 0 or
                            get_prop(ap_obj, NM_AP, 'RsnFlags') != 0)
                strength = int(props['Strength'])
                iterator = self.model.get_iter_first()
                while iterator:
                    i = self.ssid_in_model(iterator, ssid, security)
                    if i:
                        self.model.set_value(i, 2, strength)
                    iterator = self.model.iter_next(iterator)

    def build_cache(self):
        devices = self.manager.GetDevices()
        for device_path in devices:
            device_obj = self.bus.get_object(NM, device_path)
            device_type_prop = get_prop(device_obj, NM_DEVICE, 'DeviceType')
            if device_type_prop != DEVICE_TYPE_WIFI:
                continue
            iterator = None
            i = self.model.get_iter_first()
            while i:
                if self.model[i][0] == device_path:
                    iterator = i
                    break
                i = self.model.iter_next(i)
            if not iterator:
                udi = get_prop(device_obj, NM_DEVICE, 'Udi')
                if udi:
                    vendor, model = get_vendor_and_model(udi)
                else:
                    vendor, model = ('', '')
                iterator = self.model.append(
                    None, [device_path, vendor, model])
            ap_list = device_obj.GetAccessPoints(dbus_interface=NM_DEVICE_WIFI)
            ssids = []
            for ap_path in ap_list:
                ap_obj = self.bus.get_object(NM, ap_path)
                ssid = get_prop(ap_obj, NM_AP, 'Ssid')
                if ssid:
                    ssid = decode_ssid(ssid)
                    strength = int(get_prop(ap_obj, NM_AP, 'Strength') or 0)
                    security = (get_prop(ap_obj, NM_AP, 'WpaFlags') != 0 or
                                get_prop(ap_obj, NM_AP, 'RsnFlags') != 0)
                    i = self.ssid_in_model(iterator, ssid, security)
                    if not i:
                        self.model.append(iterator, [ssid, security, strength])
                    else:
                        self.model.set_value(i, 2, strength)
                    ssids.append(ssid)
            i = self.model.iter_children(iterator)
            self.prune(i, ssids)
        i = self.model.get_iter_first()
        self.prune(i, devices)
        return False


class NetworkManagerTreeView(Gtk.TreeView):
    __gtype_name__ = 'NetworkManagerTreeView'

    def __init__(self, password_entry=None, state_changed=None):
        Gtk.TreeView.__init__(self)
        self.password_entry = password_entry
        self.configure_icons()
        model = Gtk.TreeStore(str, object, object)
        model.set_sort_column_id(0, Gtk.SortType.ASCENDING)
        # TODO eventually this will subclass GenericTreeModel.
        self.wifi_model = NetworkManager(model, state_changed)
        self.set_model(model)

        ssid_column = Gtk.TreeViewColumn('')
        cell_pixbuf = Gtk.CellRendererPixbuf()
        cell_text = Gtk.CellRendererText()
        ssid_column.pack_start(cell_pixbuf, False)
        ssid_column.pack_start(cell_text, True)
        ssid_column.set_cell_data_func(cell_text, self.data_func)
        ssid_column.set_cell_data_func(cell_pixbuf, self.pixbuf_func)
        self.connect('row-activated', self.row_activated)

        self.append_column(ssid_column)
        self.set_headers_visible(False)
        self.setup_row_expansion_handling(model)

    def setup_row_expansion_handling(self, model):
        """
        If the user collapses a row, save that state. If all the APs go away
        and then return, such as when the user toggles the wifi kill switch,
        the UI should keep the row collapsed if it already was, or expand it.
        """
        self.expand_all()
        self.rows_changed_id = None

        def queue_rows_changed(*args):
            if self.rows_changed_id:
                GLib.source_remove(self.rows_changed_id)
            self.rows_changed_id = GLib.idle_add(self.rows_changed)

        model.connect('row-inserted', queue_rows_changed)
        model.connect('row-deleted', queue_rows_changed)

        self.user_collapsed = {}

        def collapsed(self, iterator, path, collapse):
            udi = model[iterator][0]
            self.user_collapsed[udi] = collapse

        self.connect('row-collapsed', collapsed, True)
        self.connect('row-expanded', collapsed, False)

    def rows_changed(self, *args):
        model = self.get_model()
        i = model.get_iter_first()
        while i:
            udi = model[i][0]
            try:
                if not self.user_collapsed[udi]:
                    path = model.get_path(i)
                    self.expand_row(path, False)
            except KeyError:
                path = model.get_path(i)
                self.expand_row(path, False)
            i = model.iter_next(i)

    def get_state(self):
        return self.wifi_model.get_state()

    def disconnect_from_ap(self):
        self.wifi_model.disconnect_from_ap()

    def row_activated(self, unused, path, column):
        passphrase = None
        if self.password_entry:
            passphrase = self.password_entry.get_text()
        self.connect_to_selection(passphrase)

    def configure_icons(self):
        it = Gtk.IconTheme()
        default = Gtk.IconTheme.get_default()
        default = default.load_icon(Gtk.STOCK_MISSING_IMAGE, 22, 0)
        it.set_custom_theme('ubuntu-mono-light')
        self.icons = []
        for n in ['nm-signal-00',
                  'nm-signal-25',
                  'nm-signal-50',
                  'nm-signal-75',
                  'nm-signal-100',
                  'nm-signal-00-secure',
                  'nm-signal-25-secure',
                  'nm-signal-50-secure',
                  'nm-signal-75-secure',
                  'nm-signal-100-secure']:
            ico = it.lookup_icon(n, 22, 0)
            if ico:
                ico = ico.load_icon()
            else:
                ico = default
            self.icons.append(ico)

    def pixbuf_func(self, column, cell, model, iterator, data):
        if not model.iter_parent(iterator):
            cell.set_property('pixbuf', None)
            return
        strength = model[iterator][2]
        if strength < 30:
            icon = 0
        elif strength < 50:
            icon = 1
        elif strength < 70:
            icon = 2
        elif strength < 90:
            icon = 3
        else:
            icon = 4
        if model[iterator][1]:
            icon += 5
        cell.set_property('pixbuf', self.icons[icon])

    def data_func(self, column, cell, model, iterator, data):
        ssid = model[iterator][0]

        if not model.iter_parent(iterator):
            txt = '{0} {1}'.format(model[iterator][1], model[iterator][2])
            cell.set_property('text', txt)
        else:
            cell.set_property('text', ssid)

    def get_passphrase(self, ssid):
        try:
            cached = self.wifi_model.passphrases_cache[ssid]
        except KeyError:
            return ''
        return cached

    def is_row_an_ap(self):
        model, iterator = self.get_selection().get_selected()
        if iterator is None:
            return False
        return model.iter_parent(iterator) is not None

    def is_row_connected(self):
        model, iterator = self.get_selection().get_selected()
        if iterator is None:
            return False
        ssid = model[iterator][0]
        parent = model.iter_parent(iterator)
        if parent and self.wifi_model.is_connected(model[parent][0], ssid):
            return True
        else:
            return False

    def connect_to_selection(self, passphrase):
        model, iterator = self.get_selection().get_selected()
        ssid = model[iterator][0]
        parent = model.iter_parent(iterator)
        if parent:
            self.wifi_model.connect_to_ap(model[parent][0], ssid, passphrase)


GObject.type_register(NetworkManagerTreeView)


class NetworkManagerWidget(Gtk.Box):
    __gtype_name__ = 'NetworkManagerWidget'
    __gsignals__ = {
        'connection': (
            GObject.SignalFlags.RUN_FIRST, GObject.TYPE_NONE,
            (GObject.TYPE_UINT,)),
        'selection_changed': (
            GObject.SignalFlags.RUN_FIRST, GObject.TYPE_NONE, ()),
        'pw_validated': (
            GObject.SignalFlags.RUN_FIRST, GObject.TYPE_NONE,
            (GObject.TYPE_BOOLEAN,))}

    def __init__(self):
        Gtk.Box.__init__(self)
        self.set_orientation(Gtk.Orientation.VERTICAL)
        self.set_spacing(12)
        self.password_entry = Gtk.Entry()
        self.view = NetworkManagerTreeView(self.password_entry,
                                           self.state_changed)
        scrolled_window = Gtk.ScrolledWindow()
        scrolled_window.set_policy(
            Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)
        scrolled_window.set_shadow_type(Gtk.ShadowType.IN)
        scrolled_window.add(self.view)
        self.pack_start(scrolled_window, True, True, 0)
        self.hbox = Gtk.Box(spacing=6)
        self.pack_start(self.hbox, False, True, 0)
        self.password_label = Gtk.Label('Password:')
        self.password_entry.set_visibility(False)
        self.password_entry.connect('activate', self.connect_to_ap)
        self.password_entry.connect('changed', self.password_entry_changed)
        self.display_password = Gtk.CheckButton('Display password')
        self.display_password.connect('toggled', self.display_password_toggled)
        self.hbox.pack_start(self.password_label, False, True, 0)
        self.hbox.pack_start(self.password_entry, True, True, 0)
        self.hbox.pack_start(self.display_password, False, True, 0)
        self.hbox.set_sensitive(False)
        self.selection = self.view.get_selection()
        self.selection.connect('changed', self.changed)
        self.show_all()

    def translate(self, password_label_text, display_password_text):
        self.password_label.set_label(password_label_text)
        self.display_password.set_label(display_password_text)

    def get_state(self):
        return self.view.get_state()

    def is_row_an_ap(self):
        return self.view.is_row_an_ap()

    def is_row_connected(self):
        return self.view.is_row_connected()

    def select_usable_row(self):
        self.selection.select_path('0:0')

    def state_changed(self, state):
        self.emit('connection', state)

    def password_is_valid(self):
        passphrase = self.password_entry.get_text()
        if 7 < len(passphrase) < 64:
            return True
        if len(passphrase) == 64:
            for c in passphrase:
                if c not in string.hexdigits:
                    return False
            return True
        else:
            return False

    def connect_to_ap(self, *args):
        if self.password_is_valid():
            passphrase = self.password_entry.get_text()
            self.view.connect_to_selection(passphrase)

    def disconnect_from_ap(self):
        self.view.disconnect_from_ap()

    def password_entry_changed(self, *args):
        self.emit('pw_validated', self.password_is_valid())

    def display_password_toggled(self, *args):
        self.password_entry.set_visibility(self.display_password.get_active())

    def changed(self, selection):
        iterator = selection.get_selected()[1]
        if not iterator:
            return
        row = selection.get_tree_view().get_model()[iterator]
        secure = row[1]
        ssid = row[0]
        if secure:
            self.hbox.set_sensitive(True)
            passphrase = self.view.get_passphrase(ssid)
            self.password_entry.set_text(passphrase)
            self.emit('pw_validated', False)
        else:
            self.hbox.set_sensitive(False)
            self.password_entry.set_text('')
            self.emit('pw_validated', True)
        self.emit('selection_changed')


GObject.type_register(NetworkManagerWidget)

if __name__ == '__main__':
    window = Gtk.Window()
    window.connect('destroy', Gtk.main_quit)
    window.set_size_request(300, 300)
    window.set_border_width(12)
    nm = NetworkManagerWidget()
    window.add(nm)
    window.show_all()
    Gtk.main()
