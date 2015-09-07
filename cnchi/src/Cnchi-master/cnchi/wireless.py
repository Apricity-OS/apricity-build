# -*- coding: utf-8; Mode: Python; indent-tabs-mode: nil; tab-width: 4 -*-
#
#  wireless.py
#
#  Copyright (C) 2010 Canonical Ltd.
#  Written by Evan Dandrea <evan.dandrea@canonical.com>
#
#  Copyright © 2015 Apricity
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

import dbus

from misc import misc, nm
from gtkbasebox import GtkBaseBox


class Wireless(GtkBaseBox):
    def __init__(self, params, prev_page="check", next_page="desktop"):
        # Check whether we can talk to NM at all
        try:
            misc.has_connection()
        except dbus.DBusException:
            self.page = None
            return

        super().__init__(self, params, "wireless", prev_page, next_page)

        self.page = self.ui.get_object('wireless')

        self.nmwidget = self.ui.get_object('nmwidget')
        self.nmwidget.connect('connection', self.state_changed)
        self.nmwidget.connect('selection_changed', self.selection_changed)
        self.nmwidget.connect('pw_validated', self.pw_validated)

        self.no_wireless = self.ui.get_object('no_wireless')
        self.use_wireless = self.ui.get_object('use_wireless')
        self.use_wireless.connect('toggled', self.wireless_toggled)
        self.plugin_widgets = self.page
        self.have_selection = False
        self.state = self.nmwidget.get_state()
        self.next_normal = True
        self.back_normal = True
        self.connect_text = None
        self.stop_text = None
        self.skip = False

    def translate_ui(self):
        lbl = self.ui.get_object('wireless_section_label')
        lbl.set_markup(_("Connecting this computer to a wi-fi network"))

        btn = self.ui.get_object('no_wireless')
        btn.set_label(_("I don't want to connect to a wi-fi network right now"))

        btn = self.ui.get_object('use_wireless')
        btn.set_label(_("Connect to this network"))

        password_label_text = _("Password:")
        display_password_text = _("Display password")
        self.nmwidget.translate(password_label_text, display_password_text)

    def selection_changed(self, unused):
        self.have_selection = True
        self.use_wireless.set_active(True)
        assert self.state is not None

        if self.state == nm.NM_STATE_CONNECTING:
            self.next_normal = True
            print("NM_SATE_CONNECTING")
        else:
            if not self.nmwidget.is_row_an_ap() or self.nmwidget.is_row_connected():
                self.next_normal = True
                print("is not an ap or is already connected")
            else:
                self.next_normal = False
                print("not connected")

    def wireless_toggled(self, unused):
        print("wireless_toggled")

        if self.use_wireless.get_active():
            self.nmwidget.hbox.set_sensitive(True)
            if not self.have_selection:
                self.nmwidget.select_usable_row()
            self.state_changed(None, self.state)
        else:
            # TODO: hide and stop spinner
            self.nmwidget.hbox.set_sensitive(False)

    '''
    def plugin_on_back_clicked(self):
        frontend = self.controller._wizard
        if frontend.back.get_label() == self.stop_text:
            self.nmwidget.disconnect_from_ap()
            return True
        else:
            frontend.connecting_spinner.hide()
            frontend.connecting_spinner.stop()
            frontend.connecting_label.hide()
            self.no_wireless.set_active(True)
            return False

    def plugin_on_next_clicked(self):
        frontend = self.controller._wizard
        if frontend.next.get_label() == self.connect_text:
            self.nmwidget.connect_to_ap()
            return True
        else:
            frontend.connecting_spinner.hide()
            frontend.connecting_spinner.stop()
            frontend.connecting_label.hide()
            return False
    '''

    def state_changed(self, unused, state):
        print("state_changed")
        self.state = state
        if not self.use_wireless.get_active():
            return
        if state != nm.NM_STATE_CONNECTING:
            # TODO: Hide and stop spinner
            pass
        else:
            pass
            # TODO: Show and start spinner
            # self.spinner.show()
            # self.spinner.start()

        self.selection_changed(None)

    def pw_validated(self, unused, validated):
        pass

    def prepare(self, direction):
        self.translate_ui()
        self.show_all()
        if not nm.wireless_hardware_present():
            self.nmwidget.set_sensitive(False)
            btn = self.ui.get_object('use_wireless')
            btn.set_sensitive(False)

    def store_values(self):
        return True

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    from test_screen import _, run

    run('Wireless')
