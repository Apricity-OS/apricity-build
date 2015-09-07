#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  check.py
#
#  Copyright © 2015 Apricity
#
#  This file is part of Cnchi.
#
#  Cnchi is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  Cnchi is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Cnchi; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

""" Check screen (detects if Antergos prerequisites are meet) """

from gi.repository import GLib
import subprocess
import os
import logging

import misc.misc as misc

from gtkbasebox import GtkBaseBox

from rank_mirrors import AutoRankmirrorsThread

# Constants
NM = 'org.freedesktop.NetworkManager'
NM_STATE_CONNECTED_GLOBAL = 70
UPOWER = 'org.freedesktop.UPower'
UPOWER_PATH = '/org/freedesktop/UPower'
MIN_ROOT_SIZE = 10000000000


class Check(GtkBaseBox):
    """ Check class """

    def __init__(self, params, prev_page="language", next_page="location"):
        """ Init class ui """
        super().__init__(self, params, "check", prev_page, next_page)

        self.remove_timer = False

        self.thread = None

        self.prepare_power_source = None
        self.prepare_network_connection = None
        self.prepare_enough_space = None
        self.timeout_id = None
        self.prepare_best_results = None

        # Boolean variable to check if reflector has been run once or not
        self.reflector_launched = False

        self.label_space = self.ui.get_object("label_space")

        if 'checks_are_optional' in params:
            self.checks_are_optional = params['checks_are_optional']
        else:
            self.checks_are_optional = False

        '''
        data_dir = self.settings.get('data')
        image1 = self.ui.get_object('image1')
        image1_path = os.path.join(data_dir, "images/apricity/apricity-for-everyone-white1.png")
        image1.set_from_file(image1_path)
        '''

    def translate_ui(self):
        """ Translates all ui elements """
        txt = _("System Check")
        self.header.set_subtitle(txt)

        self.prepare_enough_space = self.ui.get_object("prepare_enough_space")
        txt = _("has at least {0}GB available storage space").format(MIN_ROOT_SIZE / 1000000000)
        self.prepare_enough_space.props.label = txt

        txt = _("")
        txt = "<i>{0}</i>".format(txt)
        self.label_space.set_markup(txt)

        self.prepare_power_source = self.ui.get_object("prepare_power_source")
        txt = _("is plugged into a power source")
        self.prepare_power_source.props.label = txt

        self.prepare_network_connection = self.ui.get_object("prepare_network_connection")
        txt = _("is connected to the Internet")
        self.prepare_network_connection.props.label = txt

        self.prepare_best_results = self.ui.get_object("prepare_best_results")
        txt = _("For best results, please ensure that this computer:")
        txt = '<span weight="bold" size="large">{0}</span>'.format(txt)
        self.prepare_best_results.set_markup(txt)

    def check_all(self):
        """ Check that all requirements are meet """
        has_internet = misc.has_connection()
        self.prepare_network_connection.set_state(has_internet)

        on_power = not self.on_battery()
        self.prepare_power_source.set_state(on_power)

        space = self.has_enough_space()
        self.prepare_enough_space.set_state(space)

        if self.checks_are_optional:
            return True

        if has_internet and space:
            return True

        return False

    def on_battery(self):
        """ Checks if we are on battery power """
        import dbus

        if self.has_battery():
            bus = dbus.SystemBus()
            upower = bus.get_object(UPOWER, UPOWER_PATH)
            return misc.get_prop(upower, UPOWER_PATH, 'OnBattery')

        return False

    def has_battery(self):
        # UPower doesn't seem to have an interface for this.
        path = '/sys/class/power_supply'
        if not os.path.exists(path):
            return False
        for folder in os.listdir(path):
            type_path = os.path.join(path, folder, 'type')
            if os.path.exists(type_path):
                with open(type_path) as power_file:
                    if power_file.read().startswith('Battery'):
                        self.settings.set('laptop', 'True')
                        return True
        return False

    @staticmethod
    def has_enough_space():
        """ Check that we have a disk or partition with enough space """
        lsblk = subprocess.Popen(["lsblk", "-lnb"], stdout=subprocess.PIPE)
        output = lsblk.communicate()[0].decode("utf-8").split("\n")

        max_size = 0

        for item in output:
            col = item.split()
            if len(col) >= 5:
                if col[5] == "disk" or col[5] == "part":
                    size = int(col[3])
                    if size > max_size:
                        max_size = size

        if max_size >= MIN_ROOT_SIZE:
            return True

        return False

    def on_timer(self):
        """ If all requirements are meet, enable forward button """
        if not self.remove_timer:
            self.forward_button.set_sensitive(self.check_all())
        return not self.remove_timer

    def store_values(self):
        """ Continue """
        # Remove timer
        self.remove_timer = True

        logging.info(_("We have Internet connection."))
        logging.info(_("We're connected to a power source."))
        logging.info(_("We have enough disk space."))

        # Enable forward button
        self.forward_button.set_sensitive(True)

        if not self.testing and not self.reflector_launched:
            # Launch reflector script to determine the 10 fastest mirrors
            self.thread = AutoRankmirrorsThread()
            self.thread.start()
            self.reflector_launched = True

        return True

    def prepare(self, direction):
        """ Load screen """
        self.translate_ui()
        self.show_all()

        self.forward_button.set_sensitive(self.check_all())

        # Set timer
        self.timeout_id = GLib.timeout_add(5000, self.on_timer)

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    from test_screen import _, run

    run('Check')
