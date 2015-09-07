#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  vmware.py
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

""" Vmware driver installation """

from hardware.hardware import Hardware

CLASS_NAME = "Vmware"
CLASS_ID = ""
VENDOR_ID = "0x15ad"
DEVICES = [('0x0405', "VMware Virtual SVGA II"), ('0x0710', "VMware Virtual SVGA")]


class Vmware(Hardware):
    """ Vmware class definition """
    def __init__(self):
        Hardware.__init__(self)

    def get_packages(self):
        return ["xf86-video-vmware", "xf86-input-vmmouse", "open-vm-tools"]

    def post_install(self, dest_dir):
        pass

    def check_device(self, class_id, vendor_id, product_id):
        """ Checks if the driver supports this device """
        if vendor_id == VENDOR_ID:
            for (product, description) in DEVICES:
                if product_id == product:
                    return True
        return False

    def get_name(self):
        return CLASS_NAME
