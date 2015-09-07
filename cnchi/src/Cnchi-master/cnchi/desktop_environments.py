#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  desktop_environments.py
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

""" DE information """

# Enabled desktops

DESKTOPS = ["gnome"]

DESKTOPS_DEV = DESKTOPS + []

DESKTOP_ICONS_PATH = "/usr/share/cnchi/data/icons"

'''
MENU - Size appropriate for menus (16px).
SMALL_TOOLBAR - Size appropriate for small toolbars (16px).
LARGE_TOOLBAR - Size appropriate for large toolbars (24px)
BUTTON - Size appropriate for buttons (16px )
DND - Size appropriate for drag and drop (32px )
DIALOG - Size appropriate for dialogs (48px )
'''

# Descriptive names
NAMES = {
    'gnome': "Gnome"}

LIBS = {
    'gtk': ["gnome"],
    'qt': []}

# LTS DOES NOT WORK ATM
# ALL_FEATURES = ["aur", "bluetooth", "cups", "firefox", "fonts", "lts", "office", "visual", "firewall", "smb"]
ALL_FEATURES = []

# Each desktop has its own available features
# LTS DOES NOT WORK ATM
'''
FEATURES = {'gnome' : []}
'''
FEATURES = {'gnome' : []}

# Session names for lightDM setup
SESSIONS = {'gnome': 'gnome'}


# See http://docs.python.org/2/library/gettext.html "22.1.3.4. Deferred translations"
def _(message):
    return message


DESCRIPTIONS = {
    'gnome': _("Gnome 3 is an easy and elegant way to use your "
               "computer. It features the Activities Overview which "
               "is an easy way to access all your basic tasks. GNOME 3 is "
               "the default desktop in Apricity OS.")}

# Delete previous _() dummy declaration
del _

