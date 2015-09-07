#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  hardware.py
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

""" Hardware related packages installation """

import subprocess
import os
import logging

DEVICES = []

CLASS_NAME = ""

_HARDWARE_PATH = '/usr/share/cnchi/cnchi/hardware'


class Hardware(object):
    """ This is an abstract class. You need to use this as base """
    def __init__(self):
        Hardware.__init__(self)

    def get_packages(self):
        """ Returns all necessary packages to install """
        raise NotImplementedError("get_packages is not implemented")

    def post_install(self, dest_dir):
        """ Runs post install commands """
        raise NotImplementedError("postinstall is not implemented")

    def check_device(self, class_id, vendor_id, product_id):
        """ Checks if the driver supports this device """
        raise NotImplementedError("check_device is not implemented")

    def is_proprietary(self):
        """ Proprietary drivers are drivers for your hardware devices
            that are not freely-available or open source, and must be
            obtained from the hardware manufacturer. """
        return False

    def get_name(self):
        raise NotImplementedError("get_name is not implemented")

    @staticmethod
    def chroot(cmd, dest_dir, stdin=None, stdout=None):
        """ Runs command inside the chroot """
        run = ['chroot', dest_dir]

        for element in cmd:
            run.append(element)

        try:
            proc = subprocess.Popen(run,
                                    stdin=stdin,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT)
            out = proc.communicate()[0]
            logging.debug(out.decode())
        except OSError as err:
            logging.error(_("Error running command: %s"), err.strerror)


class HardwareInstall(object):
    """ This class checks user's hardware """
    def __init__(self):
        # All available objects
        self.all_objects = []

        # All objects that support devices found (can have more than one object for each device)
        self.objects_found = {}

        # All objects that are really used
        self.objects_used = []

        dirs = os.listdir(_HARDWARE_PATH)

        # We scan the folder for py files.
        # This is unsafe, but we don't care if somebody wants Cnchi to run code arbitrarily.
        for filename in dirs:
            if filename.endswith(".py") and "__init__" not in filename and "hardware" not in filename:
                filename = filename[:-len(".py")]
                name = ""
                try:
                    if __name__ == "__main__":
                        package = filename
                    else:
                        package = "hardware." + filename
                    name = filename.capitalize()
                    # This instruction is the same as "from package import name"
                    class_name = getattr(__import__(package, fromlist=[name]), "CLASS_NAME")
                    self.all_objects.append(getattr(__import__(package, fromlist=[class_name]), class_name))
                except ImportError as err:
                    logging.error(_("Error importing %s from %s : %s"), name, package, err)
                except Exception as err:
                    logging.error(_("Unexpected error importing %s: %s"), package, err)

        # Detect devices
        devices = []

        # Get PCI devices
        lines = subprocess.check_output(["lspci", "-n"]).decode().split("\n")
        for line in lines:
            if len(line) > 0:
                class_id = line.split()[1].rstrip(":")
                dev = line.split()[2].split(":")
                devices.append(("0x" + class_id, "0x" + dev[0], "0x" + dev[1]))

        # Get USB devices
        lines = subprocess.check_output(["lsusb"]).decode().split("\n")
        for line in lines:
            if len(line) > 0:
                dev = line.split()[5].split(":")
                devices.append(("0", "0x" + dev[0], "0x" + dev[1]))

        # Find objects that support the devices we've found.
        self.objects_found = {}
        for obj in self.all_objects:
            for device in devices:
                (class_id, vendor_id, product_id) = device
                check = obj.check_device(
                    self=obj,
                    class_id=class_id,
                    vendor_id=vendor_id,
                    product_id=product_id)
                if check:
                    if device not in self.objects_found:
                        self.objects_found[device] = [obj]
                    else:
                        self.objects_found[device].append(obj)

        self.objects_used = []
        for device in self.objects_found:
            objects = self.objects_found[device]
            if len(objects) > 1:
                # We have more than one driver for this device!
                for obj in objects:
                    # As we have more than one driver, only add
                    # non proprietary ones
                    if not obj.is_proprietary(obj):
                        self.objects_used.append(obj)
            else:
                self.objects_used.append(objects[0])

    def get_packages(self):
        """ Get pacman package list for all detected devices """
        packages = []
        for obj in self.objects_used:
            packages.extend(obj.get_packages(obj))

        # Remove duplicates (not necessary but it's cleaner)
        packages = list(set(packages))
        return packages

    def get_found_driver_names(self):
        driver_names = []
        for obj in self.objects_used:
            driver_names.append(obj.get_name(obj))
        return driver_names

    def post_install(self, dest_dir):
        """ Run post install commands for all detected devices """
        for obj in self.objects_used:
            obj.post_install(obj, dest_dir)

''' Test case '''
if __name__ == "__main__":
    hardware_install = HardwareInstall()
    hardware_pkgs = hardware_install.get_packages()
    print(hardware_install.get_found_driver_names())
    if len(hardware_pkgs) > 0:
        txt = " ".join(hardware_pkgs)
        print("Hardware module added these packages : ", txt)
