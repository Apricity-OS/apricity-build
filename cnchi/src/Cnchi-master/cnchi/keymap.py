#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  keymap.py
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

from gi.repository import Gtk, GLib

import os
import logging
import subprocess

import misc.misc as misc
import misc.keyboard_names as keyboard_names
import misc.keyboard_widget as keyboard_widget

from gtkbasebox import GtkBaseBox


class Keymap(GtkBaseBox):
    def __init__(self, params, prev_page="timezone", next_page="installation_ask"):
        super().__init__(self, params, "keymap", prev_page, next_page)

        self.prepare_called = False

        self.layout_treeview = self.ui.get_object("keyboardlayout")
        self.variant_treeview = self.ui.get_object("keyboardvariant")

        self.keyboard_test_entry = self.ui.get_object("keyboard_test_entry")
        self.keyboard_widget = self.ui.get_object("keyboard_widget")

        self.keyboard_layout = { 'code': None, 'name': None }
        self.keyboard_variant  = { 'code': None, 'name': None }

        self.kbd_names = keyboard_names.KeyboardNames(os.path.join(self.settings.get('data'), "kbdnames.gz"))

        self.create_treeviews()

    def translate_ui(self):
        """ Translates all ui elements """
        self.header.set_subtitle(_("Select Your Keyboard Layout"))

        lbl = self.ui.get_object("label_layouts")
        lbl.set_markup(_("Keyboard Layouts"))

        lbl = self.ui.get_object("label_variants")
        lbl.set_markup(_("Keyboard Variants"))

    def create_treeviews(self):
        render = Gtk.CellRendererText()
        col = Gtk.TreeViewColumn(_("Layouts"), render, text=0)
        liststore = Gtk.ListStore(str)
        self.layout_treeview.append_column(col)
        self.layout_treeview.set_model(liststore)

        render = Gtk.CellRendererText()
        col = Gtk.TreeViewColumn(_("Variants"), render, text=0)
        liststore = Gtk.ListStore(str)
        self.variant_treeview.append_column(col)
        self.variant_treeview.set_model(liststore)

    def prepare(self, direction):
        self.translate_ui()

        if direction == 'forwards':
            self.fill_layout_treeview()
            self.forward_button.set_sensitive(False)

            # selected_country has the country name in English
            selected_country = self.settings.get('country')
            selected_country = self.fix_countries(selected_country)
            self.keyboard_layout['code'] = self.kbd_names.get_layout_code("C", selected_country)

            if self.keyboard_layout['code'] is None:
                # Country was not found, let's choose USA as default
                logging.debug(_("Country was not found, let's choose USA as default"))
                self.keyboard_layout['code'] = "us"
                self.keyboard_layout['name'] = "USA"
                self.select_value_in_treeview(self.layout_treeview, self.keyboard_layout['name'])
            else:
                lang = self.settings.get("language_code")
                if not self.kbd_names.has_language(lang):
                    lang = "C"
                self.keyboard_layout['name'] = self.kbd_names.get_layout_name(lang, self.keyboard_layout['code'])
                self.select_value_in_treeview(self.layout_treeview, self.keyboard_layout['name'])

                # Country was found, here we should put specific variant cases
                if selected_country == "Spain" and self.settings.get("language_name") == "Catalan":
                    self.keyboard_variant['code'] = "cat"
                    self.keyboard_variant['name'] = "Spain - Catalan variant with middle-dot L"
                    self.select_value_in_treeview(self.variant_treeview, self.keyboard_variant['name'])
                if selected_country == "Canada" and self.settings.get("language_name") == "English":
                    self.keyboard_variant['code'] = "eng"
                    self.keyboard_variant['name'] = "Canada - English"
                    self.select_value_in_treeview(self.variant_treeview, self.keyboard_variant['name'])

        self.prepare_called = True

        self.show_all()

    @staticmethod
    def fix_countries(country):
        # FIXME: There are some countries that do not match with 'kbdnames' so we convert them here manually.
        # I'm not sure if there're more countries that should be added here.

        if country == "United States":
            country = "USA"
        elif country == "Russian Federation":
            country = "Russia"

        return country

    def fill_layout_treeview(self):
        lang = self.settings.get("language_code")

        if not self.kbd_names.has_language(lang):
            lang = "C"

        self.kbd_names.load(lang)

        sorted_layouts = []

        layouts = self.kbd_names.get_layouts(lang)
        for layout_code in layouts:
            sorted_layouts.append(layouts[layout_code])

        sorted_layouts = misc.sort_list(sorted_layouts, self.settings.get("locale"))

        # Block signal
        self.layout_treeview.handler_block_by_func(self.on_keyboardlayout_cursor_changed)

        # Clear our model
        liststore = self.layout_treeview.get_model()
        liststore.clear()

        # Add layouts (sorted)
        for layout in sorted_layouts:
            liststore.append([layout])

        # Unblock signal
        self.layout_treeview.handler_unblock_by_func(self.on_keyboardlayout_cursor_changed)

    def select_value_in_treeview(self, treeview, value):
        model = treeview.get_model()
        tree_iter = model.get_iter(0)

        index = 0

        found = False

        while tree_iter is not None and not found:
            if model[tree_iter][0] == value:
                treeview.set_cursor(index)
                path = model.get_path(tree_iter)
                GLib.idle_add(self.scroll_to_cell, treeview, path)
                tree_iter = None
                found = True
            else:
                index += 1
                tree_iter = model.iter_next(tree_iter)

        return found

    @staticmethod
    def scroll_to_cell(treeview, path):
        treeview.scroll_to_cell(path)
        return False

    def fill_variant_treeview(self):
        selected = self.layout_treeview.get_selection()

        if selected:
            (ls, iterator) = selected.get_selected()
            if iterator:
                # Store layout selected
                self.keyboard_layout['name'] = ls.get_value(iterator, 0)

                lang = self.settings.get("language_code")
                if not self.kbd_names.has_language(lang):
                    lang = "C"

                self.kbd_names.load(lang)

                self.keyboard_layout['code'] = self.kbd_names.get_layout_code(lang, self.keyboard_layout['name'])

                sorted_variants = []

                if self.kbd_names.has_variants(lang, self.keyboard_layout['code']):
                    variants = self.kbd_names.get_variants(lang, self.keyboard_layout['code'])
                    for variant in variants:
                        sorted_variants.append(variants[variant])
                else:
                    logging.warning("Keyboard Layout %s has no variants", self.keyboard_layout['name'])

                sorted_variants = misc.sort_list(sorted_variants, self.settings.get("locale"))

                # Block signal
                self.variant_treeview.handler_block_by_func(self.on_keyboardvariant_cursor_changed)

                # Clear our model
                liststore = self.variant_treeview.get_model()
                liststore.clear()

                # Add keyboard variants (sorted)
                for variant in sorted_variants:
                    liststore.append([variant])

                # Unblock signal
                self.variant_treeview.handler_unblock_by_func(self.on_keyboardvariant_cursor_changed)

                self.variant_treeview.set_cursor(0)
        else:
            liststore = self.variant_treeview.get_model()
            liststore.clear()

    def on_keyboardlayout_cursor_changed(self, widget):
        self.fill_variant_treeview()
        self.forward_button.set_sensitive(True)

    def on_keyboardvariant_cursor_changed(self, widget):
        self.store_values()
        self.set_keyboard_widget()

    def store_values(self):
        if self.keyboard_layout['code'] is None or self.keyboard_layout['name'] is None:
            # We have not previously stored our layout
            return

        selected = self.variant_treeview.get_selection()

        self.keyboard_variant['name'] = None

        if selected:
            (ls, iterator) = selected.get_selected()
            if iterator:
                self.keyboard_variant['name'] = ls.get_value(iterator, 0)

        lang = self.settings.get("language_code")

        if not self.kbd_names.has_language(lang):
            lang = "C"

        self.kbd_names.load(lang)

        self.keyboard_variant['code'] = self.kbd_names.get_variant_code(lang,
            self.keyboard_layout['code'],
            self.keyboard_variant['name'])

        self.settings.set("keyboard_layout", self.keyboard_layout['code'])
        self.settings.set("keyboard_variant", self.keyboard_variant['code'])

        if self.keyboard_variant['code'] is None or len(self.keyboard_variant['code']) == 0:
            txt = _("Set keyboard to layout name '{0}' ({1})").format(
            self.keyboard_layout['name'],
            self.keyboard_layout['code'])
        else:
            txt = _("Set keyboard to layout name '{0}' ({1}) and variant name '{2}' ({3})").format(
            self.keyboard_layout['name'],
            self.keyboard_layout['code'],
            self.keyboard_variant['name'],
            self.keyboard_variant['code'])
        logging.debug(txt)

        # This fixes issue 75: Won't pick/load the keyboard layout after selecting one (sticks to qwerty)
        if not self.testing and self.prepare_called:
            self.setkb()

        return True

    def setkb(self):
        if len(self.keyboard_layout['code']) > 0:
            cmd = ['setxkbmap', '-layout', self.keyboard_layout['code']]

            if len(self.keyboard_variant['code']) > 0:
                cmd.extend(["-variant", self.keyboard_variant['code']])

            try:
                subprocess.check_call(cmd)
            except subprocess.CalledProcessError as process_error:
                logging.warning(process_error)

            with misc.raised_privileges():
                cmd = ['localectl', 'set-keymap', '--no-convert', self.keyboard_layout['code']]
                try:
                    subprocess.check_call(cmd)
                except subprocess.CalledProcessError as process_error:
                    logging.warning(process_error)

    def set_keyboard_widget(self):
        """ Pass current keyboard layout to the keyboard widget. """
        self.keyboard_widget.set_layout(self.keyboard_layout['code'])
        self.keyboard_widget.set_variant(self.keyboard_variant['code'])
        self.keyboard_widget.show_all()

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    from test_screen import _, run

    run('Keymap')
