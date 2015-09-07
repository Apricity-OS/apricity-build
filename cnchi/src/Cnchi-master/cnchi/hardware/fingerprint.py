#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  fingerprint.py
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

""" Various Fingerprint devices driver installation """

# Support for consumer fingerprint reader devices.

from hardware.hardware import Hardware

CLASS_NAME = "FingerPrint"
CLASS_ID = ""
VENDOR_ID = ""
DEVICES = [
    ('0x045e', '0x00bb', ""),
    ('0x045e', '0x00bc', ""),
    ('0x045e', '0x00bd', ""),
    ('0x045e', '0x00ca', ""),
    ('0x0483', '0x2015', ""),
    ('0x0483', '0x2016', ""),
    ('0x05ba', '0x000a', ""),
    ('0x05ba', '0x0007', ""),
    ('0x05ba', '0x0008', ""),
    ('0x061a', '0x0110', ""),
    ('0x08ff', '0x1600', ""),
    ('0x08ff', '0x2550', ""),
    ('0x08ff', '0x2580', ""),
    ('0x08ff', '0x5501', ""),
    ('0x147e', '0x2016', "")]


class FingerPrint(Hardware):
    def __init__(self):
        Hardware.__init__(self)

    def get_packages(self):
        return ["fprintd"]

    def post_install(self, dest_dir):
        pass

    def check_device(self, class_id, vendor_id, product_id):
        """ Checks if the driver supports this device """
        for (vendor, product, description) in DEVICES:
            if (vendor_id, product_id) == (vendor, product):
                return True
        return False

    def get_name(self):
        return CLASS_NAME
