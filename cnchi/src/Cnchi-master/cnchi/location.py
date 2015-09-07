#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  location.py
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

from gi.repository import Gtk

# Import functions
import os
import logging
import sys
import locale

try:
    import xml.etree.cElementTree as eTree
except ImportError:
    import xml.etree.ElementTree as eTree

from gtkbasebox import GtkBaseBox


class Location(GtkBaseBox):
    def __init__(self, params, prev_page="check", next_page="timezone"):
        super().__init__(self, params, "location", prev_page, next_page)

        self.listbox = self.ui.get_object("listbox")
        self.listbox.connect("row-selected", self.on_listbox_row_selected)
        self.listbox.set_selection_mode(Gtk.SelectionMode.BROWSE)

        self.label_choose_country = self.ui.get_object("label_choose_country")
        self.label_help = self.ui.get_object("label_help")

        self.locales = {}
        self.load_locales()

        self.selected_country = ""

        self.show_all_locations = False

        button = self.ui.get_object("show_all_locations_checkbutton")
        button.connect("toggled", self.on_show_all_locations_checkbox_toggled, "")

        self.scrolledwindow = self.ui.get_object("scrolledwindow1")

    def on_show_all_locations_checkbox_toggled(self, button, name):
        self.show_all_locations = button.get_active()
        self.fill_listbox()

    def translate_ui(self):
        """ Translates all ui elements """
        txt = _("The location you select will be used to help determine the system locale. "
                "It should normally be the country in which you reside. "
                "Here is a shortlist of locations based on the language you selected.")

        self.label_help.set_text(txt)
        self.label_help.set_name("label_help")

        txt = _("Country, territory or area:")
        txt = "<span weight='bold'>{0}</span>".format(txt)
        self.label_choose_country.set_markup(txt)

        check = self.ui.get_object('show_all_locations_checkbutton')
        txt = _("Show all locations")
        check.set_label(txt)

        self.header.set_subtitle(_("Select your location"))

    def select_first_listbox_item(self):
        listbox_row = self.listbox.get_children()[0]
        self.listbox.select_row(listbox_row)

    def hide_all(self):
        names = [
            "location_box", "label_help", "label_choose_country", "box1",
            "eventbox1", "eventbox2", "scrolledwindow1", "listbox_countries"]

        for name in names:
            control = self.ui.get_object(name)
            if control is not None:
                control.hide()

    def prepare(self, direction):
        self.hide_all()
        self.fill_listbox()

        self.select_first_listbox_item()
        self.translate_ui()
        self.show_all()

        self.forward_button.set_sensitive(True)

    def load_locales(self):
        data_dir = self.settings.get('data')
        xml_path = os.path.join(data_dir, "locale", "locales.xml")

        self.locales = {}

        try:
            tree = eTree.parse(xml_path)
        except FileNotFoundError as file_error:
            logging.error(file_error)
            sys.exit(1)

        root = tree.getroot()
        locale_name = ""
        language_name = ""
        for child in root.iter("language"):
            for item in child:
                if item.tag == 'language_name':
                    language_name = item.text
                elif item.tag == 'locale_name':
                    locale_name = item.text
            if len(locale_name) > 0 and len(language_name) > 0:
                self.locales[locale_name] = language_name

        xml_path = os.path.join(data_dir, "locale", "iso3366-1.xml")

        countries = {}

        try:
            tree = eTree.parse(xml_path)
        except FileNotFoundError as file_error:
            logging.error(file_error)
            sys.exit(1)

        root = tree.getroot()
        for child in root:
            code = child.attrib['value']
            name = child.text
            countries[code] = name

        for locale_name in self.locales:
            language_name = self.locales[locale_name]
            for country_code in countries:
                if country_code in language_name:
                    self.locales[locale_name] = self.locales[locale_name] + ", " + countries[country_code]

    def get_areas(self):
        areas = []

        if not self.show_all_locations:
            lang_code = self.settings.get("language_code")
            for locale_name in self.locales:
                if lang_code in locale_name:
                    areas.append(self.locales[locale_name])
            if len(areas) == 0:
                # When we don't find any country we put all language codes.
                # This happens with Esperanto and Asturianu at least.
                for locale_name in self.locales:
                    areas.append(self.locales[locale_name])
        else:
            # Put all language codes (forced by the checkbox)
            for locale_name in self.locales:
                areas.append(self.locales[locale_name])
        
        areas.sort()

        return areas

    def fill_listbox(self):
        areas = self.get_areas()

        for listbox_row in self.listbox.get_children():
            listbox_row.destroy()

        for area in areas:
            label = Gtk.Label.new()
            label.set_markup(area)
            label.show_all()
            self.listbox.add(label)

        self.selected_country = areas[0]

    def on_listbox_row_selected(self, listbox, listbox_row):
        if listbox_row is not None:
            label = listbox_row.get_children()[0]
            if label is not None:
                self.selected_country = label.get_text()

    def set_locale(self, mylocale):
        self.settings.set("locale", mylocale)
        try:
            locale.setlocale(locale.LC_ALL, mylocale)
            logging.info(_("locale changed to : %s"), mylocale)
        except locale.Error:
            logging.warning(_("Can't change to locale '%s'"), mylocale)
            if mylocale.endswith(".UTF-8"):
                # Try without the .UTF-8 trailing
                mylocale = mylocale[:-len(".UTF-8")]
                try:
                    locale.setlocale(locale.LC_ALL, mylocale)
                    logging.info(_("locale changed to : %s"), mylocale)
                    self.settings.set("locale", mylocale)
                except locale.Error:
                    logging.warning(_("Can't change to locale '%s'"), mylocale)
            else:
                logging.warning(_("Can't change to locale '%s'"), mylocale)

    def store_values(self):
        location = self.selected_country
        logging.debug("Selected location: %s", location)
        self.settings.set('location', location)
        for mylocale in self.locales:
            if self.locales[mylocale] == location:
                self.set_locale(mylocale)
        if ',' in location:
            country = location.split(',')[1].strip()
        else:
            country = 'USA'
        logging.debug("Selected country: %s", country)
        self.settings.set('country', country)
        return True

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    from test_screen import _, run

    run('Location')
