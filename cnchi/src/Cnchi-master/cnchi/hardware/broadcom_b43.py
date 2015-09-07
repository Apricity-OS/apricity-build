#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# broadcom_b43.py
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

""" Broadcom b43 driver installation """


from hardware.hardware import Hardware

CLASS_NAME = "BroadcomB43"
CLASS_ID = "0x0200"
VENDOR_ID = "0x14e4"

DEVICES = [
    ('0x0576', "BCM43224"),  # not tested
    ('0x4307', "BCM4306/3"),
    ('0x4311', "BCM4311"),
    ('0x4312', "BCM4311"),
    ('0x4315', "BCM4312"),  # LP-PHY https://github.com/dbb/scripts/blob/master/b43-lp-installer
    ('0x4318', "BCM4318"),
    ('0x4319', "BCM4318"),
    ('0x4320', "BCM4306/3"),
    ('0x4322', "BCM4322"),  # not tested
    ('0x4324', "BCM4306/3"),
    ('0x432a', "BCM4321"),  # not tested
    ('0x432c', "BCM4322"),
    ('0x432d', "BCM4322"),  # not tested
    ('0x4331', "BCM4331"),
    ('0x4350', "BCM43222"),
    ('0x4353', "BCM43224"),
    ('0x4357', "BCM43225"),
    ('0x4358', "BCM43227"),
    ('0x4359', "BCM43228"),
    ('0x43a9', "BCM43217"),
    ('0x43aa', "BCM43131"),
    ('0xa8d6', "BCM43222"),  # not tested
    ('0xa8d8', "BCM43224"),
    ('0xa8db', "BCM43217"),  # not tested
    ('0xa99d', "BCM43421")]  # not tested


class BroadcomB43(Hardware):
    def __init__(self):
        Hardware.__init__(self)

    def get_packages(self):
        return ["b43-firmware"]

    def post_install(self, dest_dir):
        with open("/etc/modprobe.d/blacklist", "a") as blacklist:
            blacklist.write("blacklist b43_legacy\n")

    def check_device(self, class_id, vendor_id, product_id):
        """ Checks if the driver supports this device """
        if vendor_id == VENDOR_ID:
            for (product, description) in DEVICES:
                if product_id == product:
                    return True
        return False

    def get_name(self):
        return CLASS_NAME
