#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  etouchscreen.py
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

""" eGalax Touch Screen driver installation """

# References
# - http://www.x.org/archive/X11R7.5/doc/man/man4/evdev.4.html
# - https://bbs.archlinux.org/viewtopic.php?id=126208

from hardware.hardware import Hardware

import subprocess
import os

CLASS_NAME = "ETouchScreen"
CLASS_ID = ""
VENDOR_ID = "0x0eef"
DEVICES = [('0x0001', "ETouchScreen")]


class ETouchScreen(Hardware):
    def __init__(self):
        Hardware.__init__(self)

    def get_packages(self):
        return ["xinput_calibrator", "xournal"]

    def post_install(self, dest_dir):
        subprocess.check_call(["rmmod", "usbtouchscreen"])
        # Do not load the 'usbtouchscreen' module, as it conflicts with eGalax
        path = os.path.join(dest_dir, "etc/modprobe.d/blacklist-usbtouchscreen.conf")
        with open(path, 'w') as conf_file:
            conf_file.write("blacklist usbtouchscreen\n")

        # TODO: This should be computer specific
        path = os.path.join(dest_dir, "etc/X11/xorg.conf.d/99-calibration.conf")
        with open(path, 'w') as conf_file:
            conf_file.write('Section "InputClass"\n')
            conf_file.write('\tIdentifier      "calibration"\n')
            conf_file.write('\tMatchProduct    "eGalax Inc. USB TouchController"\n')
            conf_file.write('\tOption          "Calibration"   "3996 122 208 3996"\n')
            conf_file.write('\tOption          "InvertY" "1"\n')
            conf_file.write('\tOption          "SwapAxes" "0"\n')
            conf_file.write('EndSection\n')

    def check_device(self, class_id, vendor_id, product_id):
        """ Checks if the driver supports this device """
        if vendor_id == VENDOR_ID:
            for (product, description) in DEVICES:
                if product_id == product:
                    return True
        return False

    def get_name(self):
        return CLASS_NAME
