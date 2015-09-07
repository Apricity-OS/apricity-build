#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  gtkbasebox.py
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

import os
import logging


class GtkBaseBox(Gtk.Box):
    """ Base class for our screens """
    def __init__(self, child, params, name, prev_page, next_page):
        self.alternate_package_list = params['alternate_package_list']
        self.backwards_button = params['backwards_button']
        self.callback_queue = params['callback_queue']
        self.disable_tryit = params['disable_tryit']
        self.forward_button = params['forward_button']
        self.header = params['header']
        self.main_progressbar = params['main_progressbar']
        self.settings = params['settings']
        self.testing = params['testing']
        self.ui_dir = params['ui_dir']

        self.prev_page = prev_page
        self.next_page = next_page

        Gtk.Box.__init__(self)

        self.set_name(name)
        self.name = name

        logging.debug("Loading '%s' screen", name)

        self.ui = Gtk.Builder()
        self.ui_file = os.path.join(self.ui_dir, "{}.ui".format(name))
        self.ui.add_from_file(self.ui_file)
        
        # Connect UI signals
        self.ui.connect_signals(child)

        child.add(self.ui.get_object(name))

    def get_prev_page(self):
        return self.prev_page

    def get_next_page(self):
        return self.next_page

    def translate_ui(self):
        raise NotImplementedError

    def prepare(self, direction):
        raise NotImplementedError

    def store_values(self):
        raise NotImplementedError

    def get_name(self):
        return self.name
