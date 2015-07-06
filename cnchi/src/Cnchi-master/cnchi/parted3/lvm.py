#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  lvm.py
#
#  Copyright © 2013-2015 Antergos
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

""" Manage lvm volumes """

import subprocess
import logging

import misc.misc as misc
import show_message as show


@misc.raise_privileges
def get_lvm_partitions():
    """ Get all partition volumes """
    vgmap = {}
    result = subprocess.getoutput("pvdisplay")
    for line in result.split("\n"):
        if "PV Name" in line:
            pvn = line.split()[-1]
        if "VG Name" in line:
            vgn = line.split()[-1]
            if vgn in vgmap:
                vgmap[vgn].append(pvn)
            else:
                vgmap[vgn] = [pvn]
    return vgmap


@misc.raise_privileges
def get_volume_groups():
    """ Get all volume groups """
    volume_groups = []
    result = subprocess.getoutput("vgdisplay")
    for line in result.split("\n"):
        if "VG Name" in line:
            volume_groups.append(line.split()[-1])
    return volume_groups


@misc.raise_privileges
def get_logical_volumes(volume_group):
    """ Get all logical volumes from a volume group """
    logical_volumes = []
    result = subprocess.getoutput("lvdisplay {0}".format(volume_group))
    for line in result.split("\n"):
        if "LV Name" in line:
            logical_volumes.append(line.split()[-1])
    return logical_volumes

# When removing, we use -f flag to avoid warnings and confirmation messages


@misc.raise_privileges
def remove_logical_volume(logical_volume):
    """ Removes a logical volume """
    try:
        subprocess.check_call(["lvremove", "-f", logical_volume])
    except subprocess.CalledProcessError as err:
        txt = _("Can't remove logical volume {0}").format(logical_volume)
        logging.error(txt)
        logging.error(err)
        debugtxt = "{0}\n{1}".format(txt, err)
        show.error(None, debugtxt)


@misc.raise_privileges
def remove_volume_group(volume_group):
    """ Removes an entire volume group """
    # Before removing the volume group, remove its logical volumes
    logical_volumes = get_logical_volumes(volume_group)
    for logical_volume in logical_volumes:
        remove_logical_volume(logical_volume)

    # Now, remove the volume group
    try:
        subprocess.check_call(["vgremove", "-f", volume_group])
    except subprocess.CalledProcessError as err:
        txt = _("Can't remove volume group {0}").format(volume_group)
        logging.error(txt)
        logging.error(err)
        debugtxt = "{0}\n{1}".format(txt, err)
        show.error(None, debugtxt)


@misc.raise_privileges
def remove_physical_volume(physical_volume):
    """ Removes a physical volume """
    try:
        subprocess.check_call(["pvremove", "-f", physical_volume])
    except subprocess.CalledProcessError as err:
        txt = _("Can't remove physical volume {0}").format(physical_volume)
        logging.error(txt)
        logging.error(err)
        debugtxt = "{0}\n{1}".format(txt, err)
        show.error(None, debugtxt)
