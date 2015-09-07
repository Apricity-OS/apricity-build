#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  timezone.py
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

from gi.repository import Gtk, Gdk

import os
import threading
import multiprocessing
import queue
import urllib.request
import urllib.error
import time
import logging
import hashlib

import misc.tz as tz
import misc.misc as misc
import misc.timezonemap as timezonemap
from mirrorlist import GenerateMirrorListThread
from gtkbasebox import GtkBaseBox

NM = 'org.freedesktop.NetworkManager'
NM_STATE_CONNECTED_GLOBAL = 70


class Timezone(GtkBaseBox):
    def __init__(self, params, prev_page="location", next_page="keymap"):
        super().__init__(self, params, "timezone", prev_page, next_page)

        self.map_window = self.ui.get_object('timezone_map_window')

        self.combobox_zone = self.ui.get_object('comboboxtext_zone')
        self.combobox_region = self.ui.get_object('comboboxtext_region')

        # Show regions in three columns
        self.combobox_region.set_wrap_width(3)

        self.tzdb = tz.Database()
        self.timezone = None

        # This is for populate_cities
        self.old_zone = None

        # Autotimezone thread will store detected coords in this queue
        self.auto_timezone_coords = multiprocessing.Queue()

        # Thread to try to determine timezone.
        self.autodetected_coords = None
        self.auto_timezone_thread = None
        self.start_auto_timezone_thread()

        # Thread to generate a pacman mirrorlist based on country code
        # Why do this? There're foreign mirrors faster than the Spanish ones... - Karasu
        self.mirrorlist_thread = None
        # self.start_mirrorlist_thread()

        # Setup window
        self.tzmap = timezonemap.TimezoneMap()
        self.tzmap.connect('location-changed', self.on_location_changed)

        # Strip .UTF-8 from locale, icu doesn't parse it
        self.locale = os.environ['LANG'].rsplit('.', 1)[0]
        self.map_window.add(self.tzmap)
        self.tzmap.show()

    def translate_ui(self):
        """ Translates all ui elements """
        label = self.ui.get_object('label_zone')
        txt = _("Zone:")
        label.set_markup(txt)

        label = self.ui.get_object('label_region')
        txt = _("Region:")
        label.set_markup(txt)

        label = self.ui.get_object('label_ntp')
        txt = _("Uses Network Time Protocol (NTP) for clock synchronization")
        label.set_markup(txt)

        self.header.set_subtitle(_("Select Your Timezone"))

    def on_location_changed(self, tzmap, tz_location):
        # loc = self.tzdb.get_loc(self.timezone)
        if not tz_location:
            self.timezone = None
            self.forward_button.set_sensitive(False)
        else:
            self.timezone = tz_location.get_property('zone')
            logging.info(_("location changed to : %s"), self.timezone)
            self.update_comboboxes(self.timezone)
            self.forward_button.set_sensitive(True)

    def update_comboboxes(self, timezone):
        zone, region = timezone.split('/', 1)
        self.select_combobox_item(self.combobox_zone, zone)
        self.populate_cities(zone)
        self.select_combobox_item(self.combobox_region, region)

    @staticmethod
    def select_combobox_item(combobox, item):
        tree_model = combobox.get_model()
        tree_iter = tree_model.get_iter_first()

        while tree_iter is not None:
            value = tree_model.get_value(tree_iter, 0)
            if value == item:
                combobox.set_active_iter(tree_iter)
                tree_iter = None
            else:
                tree_iter = tree_model.iter_next(tree_iter)

    def set_timezone(self, timezone):
        self.timezone = timezone
        res = self.tzmap.set_timezone(timezone)
        # res will be False if the timezone is unrecognised
        self.forward_button.set_sensitive(res)

    def on_zone_combobox_changed(self, widget):
        new_zone = self.combobox_zone.get_active_text()
        if new_zone is not None:
            self.populate_cities(new_zone)

    def on_region_combobox_changed(self, widget):
        new_zone = self.combobox_zone.get_active_text()
        new_region = self.combobox_region.get_active_text()
        if new_zone is not None and new_region is not None:
            new_timezone = "{0}/{1}".format(new_zone, new_region)
            # Only set timezone if it has changed :p
            if self.timezone != new_timezone:
                self.set_timezone(new_timezone)

    def populate_zones(self):
        zones = []
        for loc in self.tzdb.locations:
            zone = loc.zone.split('/', 1)[0]
            if zone not in zones:
                zones.append(zone)
        zones.sort()
        tree_model = self.combobox_zone.get_model()
        tree_model.clear()
        for zone in zones:
            tree_model.append([zone, zone])

    def populate_cities(self, selected_zone):
        if self.old_zone != selected_zone:
            regions = []
            for loc in self.tzdb.locations:
                zone, region = loc.zone.split('/', 1)
                if zone == selected_zone:
                    regions.append(region)
            regions.sort()
            tree_model = self.combobox_region.get_model()
            tree_model.clear()
            for region in regions:
                tree_model.append([region, region])
            self.old_zone = selected_zone

    def prepare(self, direction):
        self.translate_ui()
        self.populate_zones()
        self.timezone = None
        self.forward_button.set_sensitive(False)

        if self.autodetected_coords is None:
            try:
                self.autodetected_coords = self.auto_timezone_coords.get(False, timeout=20)
            except queue.Empty:
                msg = _("Can't autodetect timezone coordinates")
                if __name__ == '__main__':
                    # When testing this screen, give 5 more seconds and try again just in case.
                    # misc.set_cursor(Gdk.CursorType.WATCH)
                    import time
                    time.sleep(5)
                    try:
                        self.autodetected_coords = self.auto_timezone_coords.get(False, timeout=20)
                    except queue.Empty:
                        logging.warning(msg)
                    finally:
                        # misc.set_cursor(Gdk.CursorType.ARROW)
                        pass
                else:
                    logging.warning(msg)

        if self.autodetected_coords:
            coords = self.autodetected_coords
            latitude = float(coords[0])
            longitude = float(coords[1])
            timezone = self.tzmap.get_timezone_at_coords(latitude, longitude)
            self.set_timezone(timezone)
            self.forward_button.set_sensitive(True)

        self.show_all()

    def start_auto_timezone_thread(self):
        self.auto_timezone_thread = AutoTimezoneThread(self.auto_timezone_coords, self.settings)
        self.auto_timezone_thread.start()

    def start_mirrorlist_thread(self):
        scripts_dir = os.path.join(self.settings.get('cnchi'), "scripts")
        self.mirrorlist_thread = GenerateMirrorListThread(self.auto_timezone_coords, scripts_dir)
        self.mirrorlist_thread.start()

    def log_location(self, loc):
        logging.debug("timezone human zone: %s", loc.human_zone)
        logging.debug("timezone country: %s", loc.country)
        logging.debug("timezone zone: %s", loc.zone)
        logging.debug("timezone human country: %s", loc.human_country)

        if loc.comment:
            logging.debug("timezone comment: %s", loc.comment)

        if loc.latitude:
            logging.debug("timezone latitude: %s", loc.latitude)

        if loc.longitude:
            logging.debug("timezone longitude: %s", loc.longitude)
        
    def store_values(self):
        loc = self.tzdb.get_loc(self.timezone)

        if loc:
            self.settings.set("timezone_human_zone", loc.human_zone)
            self.settings.set("timezone_country", loc.country)
            self.settings.set("timezone_zone", loc.zone)
            self.settings.set("timezone_human_country", loc.human_country)

            if loc.comment:
                self.settings.set("timezone_comment", loc.comment)
            else:
                self.settings.set("timezone_comment", "")

            if loc.latitude:
                self.settings.set("timezone_latitude", loc.latitude)
            else:
                self.settings.set("timezone_latitude", "")

            if loc.longitude:
                self.settings.set("timezone_longitude", loc.longitude)
            else:
                self.settings.set("timezone_longitude", "")
            
            self.log_location(loc)

        # This way process.py will know that all info has been entered
        self.settings.set("timezone_done", True)
        
        return True

    def stop_threads(self):
        logging.debug(_("Stoping timezone threads..."))
        if self.auto_timezone_thread is not None:
            self.auto_timezone_thread.stop()
        if self.mirrorlist_thread is not None:
            self.mirrorlist_thread.stop()

    def on_switch_ntp_activate(self, ntp_switch):
        self.settings['use_ntp'] = ntp_switch.get_active()


class AutoTimezoneThread(threading.Thread):
    def __init__(self, coords_queue, settings):
        super(AutoTimezoneThread, self).__init__()
        self.coords_queue = coords_queue
        self.settings = settings
        self.stop_event = threading.Event()

    def stop(self):
        self.stop_event.set()

    def run(self):
        # Calculate logo hash
        logo = "data/images/apricity/apricity-logo-mini2.png"
        logo_path = os.path.join(self.settings.get("cnchi"), logo)
        with open(logo_path, "rb") as logo_file:
            logo_bytes = logo_file.read()
        logo_hasher = hashlib.sha1()
        logo_hasher.update(logo_bytes)
        logo_digest = logo_hasher.digest()

        # Wait until there is an Internet connection available
        while not misc.has_connection():
            if self.stop_event.is_set() or self.settings.get('stop_all_threads'):
                return
            time.sleep(10)  # Delay and try again
            logging.warning(_("Can't get network status."))

        # Do not start looking for our timezone until we've reached the language screen
        # (welcome.py sets timezone_start to true when next is clicked)
        while not self.settings.get('timezone_start'):
            if self.stop_event.is_set() or self.settings.get('stop_all_threads'):
                return
            time.sleep(2)

        # OK, now get our timezone

        logging.debug(_("We have connection. Let's get our timezone"))
        try:
            url = urllib.request.Request(
                url="http://geo.antergos.com",
                data=logo_digest,
                headers={"User-Agent": "Apricity OS Installer", "Connection": "close"})
            with urllib.request.urlopen(url) as conn:
                coords = conn.read().decode('utf-8').strip()

            if coords == "0 0":
                # Sometimes server returns 0 0, we treat it as an error
                coords = None
        except Exception as general_error:
            logging.error(general_error)
            coords = None

        if coords:
            coords = coords.split()
            msg = _("Timezone (latitude {0}, longitude {1}) detected.")
            msg = msg.format(coords[0], coords[1])
            logging.debug(msg)
            self.coords_queue.put(coords)

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    from test_screen import _, run

    run('Timezone')
