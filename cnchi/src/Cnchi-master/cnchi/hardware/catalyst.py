#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  catalyst.py
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

""" AMD/ATI driver installation """

from hardware.hardware import Hardware

import os

CLASS_NAME = "Catalyst"
CLASS_ID = "0x0300"
VENDOR_ID = "0x1002"
DEVICES = []

'''
[xorg115]
Server = http://catalyst.wirephire.com/repo/xorg115/$arch
SigLevel = Optional TrustAll

[catalyst]
Server = http://catalyst.wirephire.com/repo/catalyst/$arch
SigLevel = Optional TrustAll
'''


class Catalyst(Hardware):
    def __init__(self):
        Hardware.__init__(self)

    def get_packages(self):
        pkgs = ["catalyst-hook", "catalyst-libgl",  "catalyst-utils", "acpid", "qt4", "acpid"]
        if os.uname()[-1] == "x86_64":
            pkgs.extend(["lib32-catalyst-libgl", "lib32-catalyst-utils", "lib32-opencl-catalyst"])
        return pkgs

    def post_install(self, dest_dir):
        pass

    def check_device(self, class_id, vendor_id, product_id):
        """ Checks if the driver supports this device """
        if class_id == CLASS_ID and vendor_id == VENDOR_ID:
            # At the time of writing Catalyst is not compatible with xorg 1.16,
            # we will need to downgrade to 1.15 before we can install the Catalyst drivers.
            return False
            # return True
        return False

    def get_name(self):
        return CLASS_NAME
