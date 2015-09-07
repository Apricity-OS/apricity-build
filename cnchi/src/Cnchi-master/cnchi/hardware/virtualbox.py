#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  virtualbox.py
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

""" Virtualbox driver installation """

from hardware.hardware import Hardware

import os

CLASS_NAME = "Virtualbox"
CLASS_ID = ""
VENDOR_ID = "0x80ee"
DEVICES = [('0xbeef', "InnoTek Systemberatung GmbH VirtualBox Graphics Adapter")]


class Virtualbox(Hardware):
    def __init__(self):
        Hardware.__init__(self)

    def get_packages(self):
        return ["virtualbox-guest-modules", "virtualbox-guest-utils"]

    def post_install(self, dest_dir):
        path = os.path.join(dest_dir, "etc/modules-load.d")
        if not os.path.exists(path):
            os.makedirs(path)
        path = os.path.join(dest_dir, "etc/modules-load.d/virtualbox-guest.conf")
        with open(path, 'w') as modules:
            modules.write('# Virtualbox modules added by Cnchi - Apricity OS Installer\n')
            modules.write("vboxguest\n")
            modules.write("vboxsf\n")
            modules.write("vboxvideo\n")
        super().chroot(self, ["systemctl", "disable", "openntpd"], dest_dir)
        super().chroot(self, ["systemctl", "-f", "enable", "vboxservice"], dest_dir)
        
        # This fixes bug in virtualbox-guest-modules package
        super().chroot(self, ["depmod", "-a"], dest_dir)

    def check_device(self, class_id, vendor_id, product_id):
        """ Checks if the driver supports this device """
        if vendor_id == VENDOR_ID:
            for (product, description) in DEVICES:
                if product_id == product:
                    return True
        return False

    def get_name(self):
        return CLASS_NAME
