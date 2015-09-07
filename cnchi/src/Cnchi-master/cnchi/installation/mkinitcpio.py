#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  mkinitcpio.py
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

""" Module to setup and run mkinitcpio """

import logging
import os

from installation import chroot


def run(dest_dir, settings, mount_devices, blvm):
    """ Runs mkinitcpio """

    cpu = get_cpu()

    # Add lvm and encrypt hooks if necessary
    hooks = ["base", "udev", "autodetect", "modconf", "block", "keyboard", "keymap"]
    modules = []

    # It is important that the plymouth hook comes before any encrypt hook

    plymouth_bin = os.path.join(dest_dir, "usr/bin/plymouth")
    if os.path.exists(plymouth_bin):
        hooks.append("plymouth")

    # It is important that the encrypt hook comes before the filesystems hook
    # (in case you are using LVM on LUKS, the order should be: encrypt lvm2 filesystems)

    if settings.get("use_luks"):
        if os.path.exists(plymouth_bin):
            hooks.append("plymouth-encrypt")
        else:
            hooks.append("encrypt")

        modules.extend(["dm_mod", "dm_crypt", "ext4"])

        arch = os.uname()[-1]
        if arch == 'x86_64':
            modules.extend(["aes_x86_64"])
        else:
            modules.extend(["aes_i586"])

        modules.extend(["sha256", "sha512"])

    if settings.get("f2fs"):
        modules.append("f2fs")

    if blvm or settings.get("use_lvm"):
        hooks.append("lvm2")

    if "swap" in mount_devices:
        hooks.append("resume")

    hooks.append("filesystems")

    if settings.get('btrfs') and cpu is not 'genuineintel':
        modules.append('crc32c')
    elif settings.get('btrfs') and cpu is 'genuineintel':
        modules.append('crc32c-intel')
    else:
        hooks.append("fsck")

    set_hooks_and_modules(dest_dir, hooks, modules)

    # Run mkinitcpio on the target system
    # Fix for bsdcpio error. See: http://forum.antergos.com/viewtopic.php?f=5&t=1378&start=20#p5450
    locale = settings.get('locale')
    chroot.mount_special_dirs(dest_dir)
    cmd = ['sh', '-c', 'LANG={0} /usr/bin/mkinitcpio -p linux'.format(locale)]
    chroot.run(cmd, dest_dir)
    if settings.get('feature_lts'):
        cmd = ['sh', '-c', 'LANG={0} /usr/bin/mkinitcpio -p linux-lts'.format(locale)]
        chroot.run(cmd, dest_dir)
    chroot.umount_special_dirs(dest_dir)


def set_hooks_and_modules(dest_dir, hooks, modules):
    """ Set up mkinitcpio.conf """
    logging.debug(_("Setting hooks and modules in mkinitcpio.conf"))
    logging.debug('HOOKS="%s"', ' '.join(hooks))
    logging.debug('MODULES="%s"', ' '.join(modules))

    with open("/etc/mkinitcpio.conf") as mkinitcpio_file:
        mklins = [x.strip() for x in mkinitcpio_file.readlines()]

    for i in range(len(mklins)):
        if mklins[i].startswith("HOOKS"):
            mklins[i] = 'HOOKS="{0}"'.format(' '.join(hooks))
        elif mklins[i].startswith("MODULES"):
            mklins[i] = 'MODULES="{0}"'.format(' '.join(modules))

    path = os.path.join(dest_dir, "etc/mkinitcpio.conf")
    with open(path, "w") as mkinitcpio_file:
        mkinitcpio_file.write("\n".join(mklins) + "\n")


def get_cpu():
    """ Gets CPU string definition """
    with open("/proc/cpuinfo") as proc_file:
        lines = proc_file.readlines()

    for line in lines:
        if "vendor_id" in line:
            return line.split(":")[1].replace(" ", "").lower()
    return ""
