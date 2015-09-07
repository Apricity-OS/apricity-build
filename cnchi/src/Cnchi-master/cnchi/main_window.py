#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  main_window.py
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

""" Main Cnchi Window """

from gi.repository import Gtk, Gdk, Atk

import os
import sys
import multiprocessing
import logging

import config
import welcome
import language
import location
import check
import desktop
import features
import keymap
import timezone
import user_info
import slides
import misc.misc as misc
import info
import show_message as show
import desktop_environments as desktops

from installation import ask as installation_ask
from installation import automatic as installation_automatic
from installation import alongside as installation_alongside
from installation import advanced as installation_advanced


def atk_set_image_description(widget, description):
    """ Sets the textual description for a widget that displays image/pixmap information onscreen. """
    atk_widget = widget.get_accessible()
    if atk_widget is not None:
        atk_widget.set_image_description(description)


class MainWindow(Gtk.ApplicationWindow):
    """ Cnchi main window """

    def __init__(self, app, cmd_line):
        Gtk.ApplicationWindow.__init__(self, title="Cnchi", application=app)

        self.MAIN_WINDOW_WIDTH = 875
        self.MAIN_WINDOW_HEIGHT = 550
        # Check if we have administrative privileges
        if os.getuid() != 0:
            msg = _('This installer must be run with administrative privileges, '
                    'and cannot continue without them.')
            show.error(self, msg)
            sys.exit(1)

        # Check if we're already running
        tmp_running = "/tmp/.setup-running"
        if os.path.exists(tmp_running):
            logging.error(_("File '%s' already exists."), tmp_running)
            msg = _("You cannot run two instances of this installer.\n\n"
                    "If you are sure that the installer is not already running\n"
                    "you can run this installer using the --force option\n"
                    "or you can manually delete the offending file.\n\n"
                    "Offending file: '{0}'").format(tmp_running)
            show.error(self, msg)
            sys.exit(1)

        

        self.settings = config.Settings()
        self.ui_dir = self.settings.get('ui')

        if not os.path.exists(self.ui_dir):
            cnchi_dir = os.path.join(os.path.dirname(__file__), './')
            self.settings.set('cnchi', cnchi_dir)

            ui_dir = os.path.join(os.path.dirname(__file__), 'ui/')
            self.settings.set('ui', ui_dir)

            data_dir = os.path.join(os.path.dirname(__file__), 'data/')
            self.settings.set('data', data_dir)

            self.ui_dir = self.settings.get('ui')

        if cmd_line.cache:
            logging.debug("Cnchi will use '%s' as a source directory for cached xz packages", cmd_line.cache)
            self.settings.set('cache', cmd_line.cache)

        data_dir = self.settings.get('data')

        # For things we are not ready for users to test
        self.settings.set('z_hidden', cmd_line.z_hidden)

        # Set enabled desktops
        if self.settings.get('z_hidden'):
            self.settings.set("desktops", desktops.DESKTOPS_DEV)
        else:
            self.settings.set("desktops", desktops.DESKTOPS)

        self.ui = Gtk.Builder()
        path = os.path.join(self.ui_dir, "cnchi.ui")
        self.ui.add_from_file(path)

        self.add(self.ui.get_object("main"))

        self.header_ui = Gtk.Builder()
        path = os.path.join(self.ui_dir, "header.ui")
        self.header_ui.add_from_file(path)
        self.header = self.header_ui.get_object("header")

        self.logo = self.header_ui.get_object("logo")
        path = os.path.join(data_dir, "images", "apricity", "apricity-logo-mini2.png")
        self.logo.set_from_file(path)

        # To honor our css
        self.header.set_name("header")
        self.logo.set_name("logo")

        self.main_box = self.ui.get_object("main_box")

        self.progressbar = self.ui.get_object("main_progressbar")
        self.progressbar.set_name('process_progressbar')

        self.forward_button = self.header_ui.get_object("forward_button")
        self.backwards_button = self.header_ui.get_object("backwards_button")

        atk_set_image_description(self.forward_button, _("Next step"))
        atk_set_image_description(self.backwards_button, _("Previous step"))

        self.forward_button.set_name('fwd_btn')
        self.forward_button.set_always_show_image(True)

        self.backwards_button.set_name('bk_btn')
        self.backwards_button.set_always_show_image(True)

        # Create a queue. Will be used to report pacman messages (pacman/pac.py)
        # to the main thread (installation/process.py)
        self.callback_queue = multiprocessing.JoinableQueue()

        # Save in config which download method we have to use
        if cmd_line.library:
            self.settings.set("download_library", cmd_line.library)
        else:
            # Use urllib by default
            self.settings.set("download_library", 'urllib')
        logging.info(
            _("Using %s to download packages"),
            self.settings.get("download_library"))

        self.set_titlebar(self.header)

        # Prepare params dict to pass common parameters to all screens
        self.params = dict()
        self.params['header'] = self.header
        self.params['ui_dir'] = self.ui_dir
        self.params['forward_button'] = self.forward_button
        self.params['backwards_button'] = self.backwards_button
        self.params['callback_queue'] = self.callback_queue
        self.params['settings'] = self.settings
        self.params['main_progressbar'] = self.progressbar

        if cmd_line.packagelist:
            self.params['alternate_package_list'] = cmd_line.packagelist
            logging.info(_("Using '%s' file as package list"), self.params['alternate_package_list'])
        else:
            self.params['alternate_package_list'] = ""

        self.params['checks_are_optional'] = cmd_line.no_check

        self.params['disable_tryit'] = cmd_line.disable_tryit
        self.params['testing'] = cmd_line.testing

        # Just load the first two screens (the other ones will be loaded later)
        # We do this so the user has not to wait for all the screens to be loaded
        self.pages = dict()
        self.pages["welcome"] = welcome.Welcome(self.params)

        if os.path.exists('/home/antergos/.config/openbox'):
            # In minimal iso, load language screen now
            self.pages["language"] = language.Language(self.params)

            # Fix bugy Gtk window size when using Openbox
            self.MAIN_WINDOW_WIDTH = 750
            self.MAIN_WINDOW_HEIGHT = 450

        self.connect('delete-event', self.on_exit_button_clicked)
        self.connect('key-release-event', self.check_escape)

        self.ui.connect_signals(self)
        self.header_ui.connect_signals(self)

        title = "Installer"
        self.set_title(title)
        self.header.set_title(title)
        self.header.set_subtitle(_("Apricity OS Installer"))
        self.header.set_show_close_button(True)

        self.set_geometry()

        # Set window icon
        icon_path = os.path.join(data_dir, "images", "apricity", "apricity-icon.png")
        self.set_icon_from_file(icon_path)

        # Set the first page to show

        # If minimal iso is detected, skip the welcome page.
        if os.path.exists('/home/antergos/.config/openbox'):
            self.current_page = self.pages["language"]
            self.settings.set('timezone_start', True)
        else:
            self.current_page = self.pages["welcome"]

        self.main_box.add(self.current_page)

        # Use our css file
        style_provider = Gtk.CssProvider()

        style_css = os.path.join(data_dir, "css", "gtk-style.css")

        with open(style_css, 'rb') as css:
            css_data = css.read()

        style_provider.load_from_data(css_data)

        Gtk.StyleContext.add_provider_for_screen(
            Gdk.Screen.get_default(), style_provider,
            Gtk.STYLE_PROVIDER_PRIORITY_USER
        )

        # Show main window
        self.show_all()

        self.current_page.prepare('forwards')

        # Hide backwards button
        self.backwards_button.hide()

        self.progressbar.set_fraction(0)
        self.progressbar_step = 0

        # Do not hide progress bar for minimal iso as it would break the widget alignment on language page.
        if not os.path.exists('/home/antergos/.config/openbox'):
            # Hide progress bar
            self.progressbar.hide()

        with open(tmp_running, "w") as tmp_file:
            tmp_file.write("Cnchi {0}\n".format(1234))

        misc.gtk_refresh()

    def load_pages(self):
        if not os.path.exists('/home/antergos/.config/openbox'):
            self.pages["language"] = language.Language(self.params)
        self.pages["location"] = location.Location(self.params)
        self.pages["check"] = check.Check(self.params)
        self.pages["desktop"] = desktop.DesktopAsk(self.params)
        self.pages["features"] = features.Features(self.params)
        self.pages["keymap"] = keymap.Keymap(self.params)
        self.pages["timezone"] = timezone.Timezone(self.params)
        self.pages["installation_ask"] = installation_ask.InstallationAsk(self.params)
        self.pages["installation_automatic"] = installation_automatic.InstallationAutomatic(self.params)

        if self.settings.get("enable_alongside"):
            self.pages["installation_alongside"] = installation_alongside.InstallationAlongside(self.params)
        else:
            self.pages["installation_alongside"] = None

        self.pages["installation_advanced"] = installation_advanced.InstallationAdvanced(self.params)
        self.pages["user_info"] = user_info.UserInfo(self.params)
        self.pages["slides"] = slides.Slides(self.params)

        diff = 2
        if os.path.exists('/home/antergos/.config/openbox'):
            # In minimal (openbox) we don't have a welcome screen
            diff = 3

        num_pages = len(self.pages) - diff

        if num_pages > 0:
            self.progressbar_step = 1.0 / num_pages

    def del_pages(self):
        """ When we get to user_info page we can't go back
        therefore we can delete all previous pages for good """
        # FIXME: As there are more references, this does nothing
        if self.current_page == self.pages["user_info"]:
            del self.pages["welcome"]
            del self.pages["language"]
            del self.pages["location"]
            del self.pages["check"]
            del self.pages["desktop"]
            del self.pages["features"]
            del self.pages["keymap"]
            del self.pages["timezone"]
            del self.pages["installation_ask"]
            del self.pages["installation_automatic"]
            if self.pages["installation_alongside"] is not None:
                del self.pages["installation_alongside"]
            del self.pages["installation_advanced"]

    def set_geometry(self):
        """ Sets Cnchi window geometry """
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_resizable(False)
        self.set_size_request(self.MAIN_WINDOW_WIDTH, self.MAIN_WINDOW_HEIGHT)
        self.set_default_size(self.MAIN_WINDOW_WIDTH, self.MAIN_WINDOW_HEIGHT)

        geom = Gdk.Geometry()
        geom.min_width = self.MAIN_WINDOW_WIDTH
        geom.min_height = self.MAIN_WINDOW_HEIGHT
        geom.max_width = self.MAIN_WINDOW_WIDTH
        geom.max_height = self.MAIN_WINDOW_HEIGHT
        geom.base_width = self.MAIN_WINDOW_WIDTH
        geom.base_height = self.MAIN_WINDOW_HEIGHT
        geom.width_inc = 0
        geom.height_inc = 0

        hints = Gdk.WindowHints.MIN_SIZE | Gdk.WindowHints.MAX_SIZE | Gdk.WindowHints.BASE_SIZE | Gdk.WindowHints.RESIZE_INC
        
        self.set_geometry_hints(None, geom, hints)

    def check_escape(self, widget, event, data=None):
        """ Params: GtkWidget *widget, GdkEventKey *event, gpointer data """
        if event.keyval == 65307:
            response = self.confirm_quitting()
            if response == Gtk.ResponseType.YES:
                self.on_exit_button_clicked(self)
                self.destroy()

    def confirm_quitting(self):
        message = Gtk.MessageDialog(
            transient_for=self,
            modal=True,
            destroy_with_parent=True,
            message_type=Gtk.MessageType.QUESTION,
            buttons=Gtk.ButtonsType.YES_NO,
            text=_("Do you really want to quit the installer?"))
        response = message.run()
        message.destroy()
        return response

    def on_exit_button_clicked(self, widget, data=None):
        """ Quit Cnchi """
        try:
            misc.remove_temp_files()
            logging.info(_("Quiting installer..."))
            self.settings.set('stop_all_threads', True)
            logging.shutdown()
        except KeyboardInterrupt:
            pass

    def set_progressbar_step(self, add_value):
        new_value = self.progressbar.get_fraction() + add_value
        if new_value > 1:
            new_value = 1
        if new_value < 0:
            new_value = 0
        self.progressbar.set_fraction(new_value)
        if new_value > 0:
            self.progressbar.show()
        else:
            self.progressbar.hide()

    def on_forward_button_clicked(self, widget, data=None):
        """ Show next screen """
        next_page = self.current_page.get_next_page()

        if next_page is not None:
            # self.logo.hide()
            if next_page not in self.pages.keys():
                # Load all pages
                self.load_pages()
                self.progressbar_step = 1.0 / (len(self.pages) - 2)

            stored = self.current_page.store_values()

            if stored:
                self.set_progressbar_step(self.progressbar_step)
                self.main_box.remove(self.current_page)

                self.current_page = self.pages[next_page]

                if self.current_page is not None:
                    if next_page == "user_info":
                        self.del_pages()
                    self.current_page.prepare('forwards')
                    self.main_box.add(self.current_page)
                    if self.current_page.get_prev_page() is not None:
                        # There is a previous page, show back button
                        self.backwards_button.show()
                        self.backwards_button.set_sensitive(True)
                    else:
                        # We can't go back, hide back button
                        self.backwards_button.hide()
                        if self.current_page == "slides":
                            # Show logo in slides screen
                            self.logo.show_all()

    def on_backwards_button_clicked(self, widget, data=None):
        """ Show previous screen """
        prev_page = self.current_page.get_prev_page()

        if prev_page is not None:
            self.set_progressbar_step(-self.progressbar_step)

            # If we go backwards, don't store user changes
            # self.current_page.store_values()

            self.main_box.remove(self.current_page)

            self.current_page = self.pages[prev_page]

            if self.current_page is not None:
                self.current_page.prepare('backwards')
                self.main_box.add(self.current_page)

                if self.current_page.get_prev_page() is None:
                    # We're at the first page
                    self.backwards_button.hide()
                    self.progressbar.hide()
                    self.logo.show_all()
