#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  language.py
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
import gettext
import locale
import os
import logging
import sys

from gtkbasebox import GtkBaseBox

# Useful vars for gettext (translations)
APP_NAME = "cnchi"
LOCALE_DIR = "/usr/share/locale"

import misc.i18n as i18n


class Language(GtkBaseBox):
    def __init__(self, params, prev_page="welcome", next_page="check"):
        super().__init__(self, params, "language", prev_page, next_page)

        # Set up list box
        self.listbox = self.ui.get_object("listbox")
        self.listbox.connect("row-selected", self.on_listbox_row_selected)
        self.listbox.set_selection_mode(Gtk.SelectionMode.BROWSE)

        data_dir = self.settings.get('data')

        self.current_locale = locale.getdefaultlocale()[0]
        self.language_list = os.path.join(data_dir, "locale", "languagelist.txt.gz")
        self.set_languages_list()

        image1 = self.ui.get_object("image1")
        image1.set_from_file(os.path.join(data_dir, "images/languages.png"))

        label = self.ui.get_object("welcome_label")
        label.set_name("WelcomeMessage")

    def on_listbox_row_selected(self, listbox, listbox_row):
        """ Someone selected a different row of the listbox """
        if listbox_row is not None:
            for vbox in listbox_row:
                for label in vbox.get_children():
                    current_language, sorted_choices, display_map = i18n.get_languages(self.language_list)
                    lang = label.get_text()
                    lang_code = display_map[lang][1]
                    self.set_language(lang_code)

    def translate_ui(self):
        """ Translates all ui elements """
        txt_bold = _("Notice: The Cnchi Installer is beta software")
        # FIXME: Can't use an a html tag in the label. Causes an accessible GTK Assertion
        txt = _("Cnchi is beta software that is under active development.\n"
                "It does not yet properly handle RAID, btrfs subvolumes, or other advanced\n"
                "setups. Please proceed with caution as data loss is possible!\n\n")
        txt_markup = "<span weight='bold'>{0}</span>\n\n{1}".format(txt_bold, txt)
        label = self.ui.get_object("welcome_label")
        label.set_markup(txt_markup)

        txt = _("Choose your language")
        self.header.set_subtitle(txt)

    def langcode_to_lang(self, display_map):
        # Special cases in which we need the complete current_locale string
        if self.current_locale not in ('pt_BR', 'zh_CN', 'zh_TW'):
            self.current_locale = self.current_locale.split("_")[0]

        for lang, lang_code in display_map.items():
            if lang_code[1] == self.current_locale:
                return lang

    def set_languages_list(self):
        """ Load languages list """
        try:
            current_language, sorted_choices, display_map = i18n.get_languages(self.language_list)
        except FileNotFoundError as file_error:
            logging.error(file_error)
            sys.exit(1)

        current_language = self.langcode_to_lang(display_map)
        for lang in sorted_choices:
            box = Gtk.VBox()
            label = Gtk.Label()
            label.set_markup(lang)
            box.add(label)
            self.listbox.add(box)
            if current_language == lang:
                self.select_default_row(current_language)

    def set_language(self, locale_code):
        if locale_code is None:
            locale_code, encoding = locale.getdefaultlocale()

        try:
            lang = gettext.translation(APP_NAME, LOCALE_DIR, [locale_code])
            lang.install()
            self.translate_ui()
        except IOError:
            logging.warning(_("Can't find translation file for the %s language"), locale_code)

    def select_default_row(self, language):
        for listbox_row in self.listbox.get_children():
            for vbox in listbox_row.get_children():
                label = vbox.get_children()[0]
                if language == label.get_text():
                    self.listbox.select_row(listbox_row)
                    return

    def store_values(self):
        lang = ""
        listbox_row = self.listbox.get_selected_row()
        if listbox_row is not None:
            for vbox in listbox_row:
                for label in vbox.get_children():
                    lang = label.get_text()

        current_language, sorted_choices, display_map = i18n.get_languages(self.language_list)

        if len(lang) > 0:
            self.settings.set("language_name", display_map[lang][0])
            self.settings.set("language_code", display_map[lang][1])

        return True

    def prepare(self, direction):
        self.translate_ui()
        # Enable forward button
        self.forward_button.set_sensitive(True)
        self.show_all()

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

if __name__ == '__main__':
    from test_screen import _, run

    run('Language')
