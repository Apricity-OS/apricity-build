#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  auto_partition.py
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

import os
import subprocess
import logging
import math

from misc.misc import InstallError

'''
NOTE: Exceptions in this file

On a warning situation, Cnchi should try to continue, so we need to catch the exception here.
If we don't catch the exception here, it will be caught in process.py and managed as a fatal error.
On the other hand, if we want to clarify the exception message we can catch it here
and then raise an InstallError exception.
'''

""" AutoPartition module """

# Partition sizes are in MiB
MAX_ROOT_SIZE = 30000

# KDE needs 4.5 GB for its files. Need to leave extra space also.
MIN_ROOT_SIZE = 10000


def get_info(part):
    """ Get partition info using blkid """
    cmd = ['blkid', part]
    ret = subprocess.check_output(cmd).decode().strip()

    partdic = {}

    for info in ret.split():
        if '=' in info:
            info = info.split('=')
            partdic[info[0]] = info[1].strip('"')

    return partdic


def check_output(command):
    """ Calls subprocess.check_output, decodes its exit and removes trailing \n """
    return subprocess.check_output(command.split()).decode().strip("\n")


def printk(enable):
    """ Enables / disables printing kernel messages to console """
    with open("/proc/sys/kernel/printk", "w") as fpk:
        if enable:
            fpk.write("4")
        else:
            fpk.write("0")


def unmount(directory):
    logging.warning(_("Unmounting %s"), directory)
    try:
        subprocess.call(["umount", directory])
    except subprocess.CalledProcessError:
        logging.warning(_("Unmounting %s failed. Trying lazy arg."), directory)
        try:
            subprocess.call(["umount", "-l", directory])
        except subprocess.CalledProcessError:
            logging.warning(_("Unmounting %s failed."), directory)


def unmount_all(dest_dir):
    """ Unmounts all devices that are mounted inside dest_dir """
    try:
        cmd = ["swapon", "--show=NAME", "--noheadings"]
        swaps = subprocess.check_output(cmd).decode().split("\n")
        for name in filter(None, swaps):
            if "/dev/zram" not in name:
                subprocess.check_call(["swapoff", name])
    except subprocess.CalledProcessError as err:
        logging.warning(_("Command %s failed"), err.cmd)
        logging.warning(_("Output: %s"), err.output)

    try:
        mount_result = subprocess.check_output("mount").decode().split("\n")
    except subprocess.CalledProcessError as err:
        logging.warning(_("Command %s failed"), err.cmd)
        logging.warning(_("Output: %s"), err.output)
        return

    # Umount all devices mounted inside dest_dir (if any)
    dirs = []
    for mount in mount_result:
        if dest_dir in mount:
            directory = mount.split()[0]
            # Do not unmount dest_dir now (we will do it later)
            if directory is not dest_dir:
                dirs.append(directory)

    for directory in dirs:
        unmount(directory)

    # Now is the time to unmount the device that is mounted in dest_dir (if any)
    if dest_dir in mount_result:
        unmount(dest_dir)

    # Remove all previous LVM volumes
    # (it may have been left created due to a previous failed installation)
    try:
        lvolumes = check_output("lvs -o lv_name,vg_name --noheading").split("\n")
        if len(lvolumes[0]) > 0:
            for lvolume in lvolumes:
                if len(lvolume) > 0:
                    (lvolume, vgroup) = lvolume.split()
                    lvdev = "/dev/" + vgroup + "/" + lvolume
                    subprocess.check_call(["wipefs", "-a", lvdev])
                    subprocess.check_call(["lvremove", "-f", lvdev])

        vgnames = check_output("vgs -o vg_name --noheading").split("\n")
        if len(vgnames[0]) > 0:
            for vgname in vgnames:
                vgname = vgname.strip()
                if len(vgname) > 0:
                    subprocess.check_call(["vgremove", "-f", vgname])

        pvolumes = check_output("pvs -o pv_name --noheading").split("\n")
        if len(pvolumes[0]) > 0:
            for pvolume in pvolumes:
                pvolume = pvolume.strip(" ")
                subprocess.check_call(["pvremove", "-f", pvolume])
    except subprocess.CalledProcessError as err:
        logging.warning(_("Can't delete existent LVM volumes"))
        logging.warning(_("Command %s failed"), err.cmd)
        logging.warning(_("Output: %s"), err.output)

    # Close LUKS devices (they may have been left open because of a previous failed installation)
    try:
        if os.path.exists("/dev/mapper/cryptApricity"):
            subprocess.check_call(["cryptsetup", "luksClose", "/dev/mapper/cryptApricity"])
        if os.path.exists("/dev/mapper/cryptApricityHome"):
            subprocess.check_call(["cryptsetup", "luksClose", "/dev/mapper/cryptApricityHome"])
    except subprocess.CalledProcessError as err:
        logging.warning(_("Can't close already opened LUKS devices"))
        logging.warning(_("Command %s failed"), err.cmd)
        logging.warning(_("Output: %s"), err.output)


def setup_luks(luks_device, luks_name, luks_pass=None, luks_key=None):
    """ Setups a luks device """

    if (luks_pass is None or luks_pass == "") and luks_key is None:
        txt = _("Can't setup LUKS in device {0}. A password or a key file are needed")
        txt = txt.format(luks_device)
        logging.error(txt)
        return

    # For now, we we'll use the same password for root and /home
    # If instead user wants to use a key file, we'll have two different key files.

    logging.debug(_("Cnchi will setup LUKS on device %s"), luks_device)

    # Wipe LUKS header (just in case we're installing on a pre LUKS setup)
    # For 512 bit key length the header is 2MiB
    # If in doubt, just be generous and overwrite the first 10MiB or so
    dd("/dev/zero", luks_device, bs=512, count=20480)

    if luks_pass is None or luks_pass == "":
        # No key password given, let's create a random keyfile
        dd("/dev/urandom", luks_key, bs=1024, count=4)

        # Set up luks with a keyfile
        try:
            cmd = ["cryptsetup", "luksFormat", "-q", "-c", "aes-xts-plain", "-s", "512", luks_device, luks_key]
            subprocess.check_call(cmd)
            cmd = ["cryptsetup", "luksOpen", luks_device, luks_name, "-q", "--key-file", luks_key]
            subprocess.check_call(cmd)
        except subprocess.CalledProcessError as err:
            txt = _("Can't format and open the LUKS device {0}").format(luks_device)
            logging.error(txt)
            logging.error(_("Command %s failed"), err.cmd)
            logging.error(_("Output: %s"), err.output)
            raise InstallError(txt)

    else:
        # Set up luks with a password key

        luks_pass_bytes = bytes(luks_pass, 'UTF-8')

        # https://code.google.com/p/cryptsetup/wiki/Cryptsetup160
        # aes-xts-plain
        # aes-cbc-essiv:sha256
        try:
            cmd = ["cryptsetup", "luksFormat", "-q", "-c", "aes-xts-plain64", "-s", "512", "--key-file=-", luks_device]
            proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
            proc.communicate(input=luks_pass_bytes)

            cmd = ["cryptsetup", "luksOpen", luks_device, luks_name, "-q", "--key-file=-"]
            proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
            proc.communicate(input=luks_pass_bytes)
        except subprocess.CalledProcessError as err:
            txt = _("Can't format and open the LUKS device {0}").format(luks_device)
            logging.error(txt)
            logging.error(_("Command %s failed"), err.cmd)
            logging.error(_("Output: %s"), err.output)
            raise InstallError(txt)


def wipefs(device):
    try:
        subprocess.check_call(["wipefs", "-a", device])
    except subprocess.CalledProcessError as err:
        logging.warning(_("Can't wipe filesystem of device %s"), device)
        logging.warning(_("Command %s failed"), err.cmd)
        logging.warning(_("Output: %s"), err.output)


def dd(input_device, output_device, bs=512, count=2048):
    """ Helper function to call dd """
    cmd = ['dd', 'if={0}'.format(input_device), 'of={0}'.format(output_device), 'bs={0}'.format(bs),
           'count={0}'.format(count), 'status=noxfer']
    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError as err:
        logging.warning(_("Can't run dd command"))
        logging.warning(_("Command %s failed"), err.cmd)
        logging.warning(_("Output: %s"), err.output)


def sgdisk(command, device):
    """ Helper function to call sgdisk (GPT) """
    cmd = ['sgdisk', "--{0}".format(command), device]
    subprocess.check_call(cmd)


def sgdisk_new(device, part_num, label, size, hex_code):
    """ Helper function to call sgdisk --new (GPT) """
    cmd = ['sgdisk', '--new={0}:0:+{1}M'.format(part_num, size), '--typecode={0}:{1}'.format(part_num, hex_code),
           '--change-name={0}:{1}'.format(part_num, label), device]
    # --new: Create a new partition, numbered partnum, starting at sector start and ending at sector end.
    # Parameters: partnum:start:end (zero in start or end means using default value)
    # --typecode: Change a partition's GUID type code to the one specified by hexcode.
    #             Note that hexcode is a gdisk/sgdisk internal two-byte hexadecimal code.
    #             You can obtain a list of codes with the -L option.
    # Parameters: partnum:hexcode
    # --change-name: Change the name of the specified partition.
    # Parameters: partnum:name
    logging.debug(" ".join(cmd))

    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError as err:
        txt = _("Error creating a new partition on device {0}").format(device)
        logging.error(txt)
        logging.error(_("Command %s failed"), err.cmd)
        logging.error(_("Output: %s"), err.output)
        raise InstallError(txt)


def parted_set(device, number, flag, state):
    """ Helper function to call set parted command """
    cmd = ['parted', '--align', 'optimal', '--script', device, 'set', number, flag, state]
    subprocess.check_call(cmd)


def parted_mkpart(device, ptype, start, end, filesystem=""):
    """ Helper function to call mkpart parted command """
    # If start is < 0 we assume we want to mkpart at the start of the disk
    if start < 0:
        start_str = "1"
    else:
        start_str = "{0}MiB".format(start)

    end_str = "{0}MiB".format(end)

    cmd = ['parted', '--align', 'optimal', '--script', device, 'mkpart', ptype, filesystem, start_str, end_str]

    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError as err:
        txt = _("Error creating a new partition on device {0}").format(device)
        logging.error(txt)
        logging.error(_("Command %s failed"), err.cmd)
        logging.error(_("Output: %s"), err.output)
        raise InstallError(txt)


def parted_mktable(device, table_type="msdos"):
    """ Helper function to call mktable parted command """
    cmd = ["parted", "--align", "optimal", "--script", device, "mktable", table_type]
    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError as err:
        txt = _("Error creating a new partition table on device {0}").format(device)
        logging.error(txt)
        logging.error(_("Command %s failed"), err.cmd)
        logging.error(_("Output: %s"), err.output)
        raise InstallError(txt)


''' AutoPartition Class '''


class AutoPartition(object):
    """ Class used by the automatic installation method """

    def __init__(self, dest_dir, auto_device, use_luks, luks_password, use_lvm, use_home, bootloader, callback_queue):
        """ Class initialization """
        self.dest_dir = dest_dir
        self.auto_device = auto_device
        self.luks_password = luks_password
        # Use LUKS encryption
        self.luks = use_luks
        # Use LVM
        self.lvm = use_lvm
        # Make home a different partition or if using LVM, a different volume
        self.home = use_home
        
        self.bootloader = bootloader.lower()

        # Will use these queue to show progress info to the user
        self.callback_queue = callback_queue

        if os.path.exists("/sys/firmware/efi"):
            # If UEFI use GPT by default
            self.UEFI = True
            self.GPT = True
        else:
            # If no UEFI, use MBR by default
            self.UEFI = False
            self.GPT = False

    def mkfs(self, device, fs_type, mount_point, label_name, fs_options="", btrfs_devices=""):
        """ We have two main cases: "swap" and everything else. """
        logging.debug(_("Will format device %s as %s"), device, fs_type)
        if fs_type == "swap":
            try:
                swap_devices = check_output("swapon -s")
                if device in swap_devices:
                    subprocess.check_call(["swapoff", device])
                subprocess.check_call(["mkswap", "-L", label_name, device])
                subprocess.check_call(["swapon", device])
            except subprocess.CalledProcessError as err:
                logging.warning(_("Can't activate swap in %s"), device)
                logging.warning(_("Command %s has failed."), err.cmd)
                logging.warning(_("Output : %s"), err.output)
        else:
            mkfs = {"xfs": "mkfs.xfs {0} -L {1} -f {2}".format(fs_options, label_name, device),
                    "jfs": "yes | mkfs.jfs {0} -L {1} {2}".format(fs_options, label_name, device),
                    "reiserfs": "yes | mkreiserfs {0} -l {1} {2}".format(fs_options, label_name, device),
                    "ext2": "mkfs.ext2 -q {0} -F -L {1} {2}".format(fs_options, label_name, device),
                    "ext3": "mkfs.ext3 -q {0} -F -L {1} {2}".format(fs_options, label_name, device),
                    "ext4": "mkfs.ext4 -q {0} -F -L {1} {2}".format(fs_options, label_name, device),
                    "btrfs": "mkfs.btrfs {0} -L {1} {2}".format(fs_options, label_name, btrfs_devices),
                    "nilfs2": "mkfs.nilfs2 {0} -L {1} {2}".format(fs_options, label_name, device),
                    "ntfs-3g": "mkfs.ntfs {0} -L {1} {2}".format(fs_options, label_name, device),
                    "vfat": "mkfs.vfat {0} -n {1} {2}".format(fs_options, label_name, device),
                    "f2fs": "mkfs.f2fs {0} -l {1} {2}".format(fs_options, label_name, device)}

            # Make sure the fs type is one we can handle
            if fs_type not in mkfs.keys():
                txt = _("Unknown filesystem type {0}").format(fs_type)
                raise InstallError(txt)

            command = mkfs[fs_type]

            try:
                subprocess.check_call(command.split())
            except subprocess.CalledProcessError as err:
                txt = _("Can't create filesystem {0}").format(fs_type)
                logging.error(txt)
                logging.error(_("Command %s failed"), err.cmd)
                logging.error(_("Output: %s"), err.output)
                raise InstallError(txt)

            # Flush filesystem buffers
            try:
                subprocess.check_call(["sync"])
            except subprocess.CalledProcessError as err:
                logging.warning(_("Command %s failed"), err.cmd)
                logging.warning(_("Output: %s"), err.output)

            # Create our mount directory
            path = self.dest_dir + mount_point
            if not os.path.exists(path):
                os.makedirs(path, mode=0o755)

            # Mount our new filesystem

            mopts = "rw,relatime"
            if fs_type == "ext4":
                mopts = "rw,relatime,data=ordered"
            elif fs_type == "btrfs":
                mopts = 'rw,relatime,space_cache,autodefrag,inode_cache'

            try:
                subprocess.check_call(["mount", "-t", fs_type, "-o", mopts, device, path])
            except subprocess.CalledProcessError as err:
                txt = _("Error trying to  mount {0} in {1}").format(device, path)
                logging.error(txt)
                logging.error(_("Command %s failed"), err.cmd)
                logging.error(_("Output: %s"), err.output)
                raise InstallError(txt)

            # Change permission of base directories to avoid btrfs issues
            if mount_point == "/tmp":
                mode = 0o1777
            elif mount_point == "/root":
                mode = 0o750
            else:
                mode = 0o755
            os.chmod(path, mode)

        fs_uuid = get_info(device)['UUID']
        fs_label = get_info(device)['LABEL']
        logging.debug(_("Device details: %s UUID=%s LABEL=%s"), device, fs_uuid, fs_label)

    @property
    def get_devices(self):
        """ Set (and return) all partitions on the device """
        devices = {}
        device = self.auto_device

        # device is of type /dev/sdX or /dev/hdX

        if self.GPT:
            if not self.UEFI:
                # Skip BIOS Boot Partition
                # We'll never get here as we use UEFI+GPT or BIOS+MBR
                part_num = 2
            else:
                part_num = 1

            if self.bootloader == "grub2":
                devices['efi'] = "{0}{1}".format(device, part_num)
                part_num += 1
            
            devices['boot'] = "{0}{1}".format(device, part_num)
            part_num += 1
            devices['root'] = "{0}{1}".format(device, part_num)
            part_num += 1
            if self.home:
                devices['home'] = "{0}{1}".format(device, part_num)
                part_num += 1
            devices['swap'] = "{0}{1}".format(device, part_num)
        else:
            devices['boot'] = "{0}{1}".format(device, 1)
            devices['root'] = "{0}{1}".format(device, 2)
            if self.home:
                devices['home'] = "{0}{1}".format(device, 3)
            devices['swap'] = "{0}{1}".format(device, 5)

        if self.luks:
            if self.lvm:
                # LUKS and LVM
                devices['luks'] = devices['root']
                devices['lvm'] = "/dev/mapper/cryptApricity"
            else:
                # LUKS and no LVM
                devices['luks'] = devices['root']
                devices['root'] = "/dev/mapper/cryptApricity"
                if self.home:
                    # In this case we'll have two LUKS devices, one for root
                    # and the other one for /home
                    devices['luks2'] = devices['home']
                    devices['home'] = "/dev/mapper/cryptApricityHome"
        elif self.lvm:
            # No LUKS but using LVM
            devices['lvm'] = devices['root']

        if self.lvm:
            devices['root'] = "/dev/ApricityVG/ApricityRoot"
            devices['swap'] = "/dev/ApricityVG/ApricitySwap"
            if self.home:
                devices['home'] = "/dev/ApricityVG/ApricityHome"

        return devices

    def get_mount_devices(self):
        """ Specify for each mount point which device we must mount there """

        devices = self.get_devices
        mount_devices = {}

        if self.GPT and self.bootloader == "grub2":
            mount_devices['/boot/efi'] = devices['efi']

        mount_devices['/boot'] = devices['boot']
        mount_devices['/'] = devices['root']

        if self.home:
            mount_devices['/home'] = devices['home']

        if self.luks:
            mount_devices['/'] = devices['luks']
            if self.home and not self.lvm:
                mount_devices['/home'] = devices['luks2']

        mount_devices['swap'] = devices['swap']

        for mount_device in mount_devices:
            logging.debug(_("%s will be mounted as %s"), mount_devices[mount_device], mount_device)

        return mount_devices

    def get_fs_devices(self):
        """ Return which filesystem is in a selected device """

        devices = self.get_devices

        fs_devices = {}

        if self.GPT and self.bootloader == "grub2":
            fs_devices[devices['efi']] = "vfat"

        if self.GPT and self.bootloader == "gummiboot":
            fs_devices[devices['boot']] = "vfat"
        else:
            fs_devices[devices['boot']] = "ext2"
        
        fs_devices[devices['swap']] = "swap"
        fs_devices[devices['root']] = "ext4"

        if self.home:
            fs_devices[devices['home']] = "ext4"

        if self.luks:
            fs_devices[devices['luks']] = "ext4"
            if self.home:
                if self.lvm:
                    # luks, lvm, home
                    fs_devices[devices['home']] = "ext4"
                else:
                    # luks, home
                    fs_devices[devices['luks2']] = "ext4"

        for device in fs_devices:
            logging.debug(_("Device %s will have a %s filesystem"), device, fs_devices[device])

        return fs_devices

    def get_part_sizes(self, disk_size, start_part_sizes=1):
        part_sizes = {'disk': disk_size, 'boot': 256, 'efi': 0}

        if self.GPT and self.bootloader == "grub2":
            part_sizes['efi'] = 200

        mem_total = check_output("grep MemTotal /proc/meminfo")
        mem_total = int(mem_total.split()[1])
        mem = mem_total / 1024

        # Suggested sizes from Anaconda installer
        if mem < 2048:
            part_sizes['swap'] = 2 * mem
        elif 2048 <= mem < 8192:
            part_sizes['swap'] = mem
        elif 8192 <= mem < 65536:
            part_sizes['swap'] = mem // 2
        else:
            part_sizes['swap'] = 4096

        # Max swap size is 10% of all available disk size
        max_swap = disk_size * 0.1
        if part_sizes['swap'] > max_swap:
            part_sizes['swap'] = max_swap

        part_sizes['swap'] = math.ceil(part_sizes['swap'])

        part_sizes['root'] = disk_size - (start_part_sizes + part_sizes['efi'] + part_sizes['boot'] + part_sizes['swap'])

        if self.home:
            # Decide how much we leave to root and how much we leave to /home
            new_root_part_size = part_sizes['root'] // 5
            if new_root_part_size > MAX_ROOT_SIZE:
                new_root_part_size = MAX_ROOT_SIZE
            elif new_root_part_size < MIN_ROOT_SIZE:
                new_root_part_size = MIN_ROOT_SIZE
            part_sizes['home'] = part_sizes['root'] - new_root_part_size
            part_sizes['root'] = new_root_part_size
        else:
            part_sizes['home'] = 0

        part_sizes['lvm_pv'] = part_sizes['swap'] + part_sizes['root'] + part_sizes['home']

        for part in part_sizes:
            part_sizes[part] = int(part_sizes[part])

        return part_sizes

    def log_part_sizes(self, part_sizes):
        logging.debug(_("Total disk size: %dMiB"), part_sizes['disk'])
        if self.GPT and self.bootloader == "grub2":
            logging.debug(_("EFI System Partition (ESP) size: %dMiB"), part_sizes['efi'])
        logging.debug(_("Boot partition size: %dMiB"), part_sizes['boot'])

        if self.lvm:
            logging.debug(_("LVM physical volume size: %dMiB"), part_sizes['lvm_pv'])

        logging.debug(_("Swap partition size: %dMiB"), part_sizes['swap'])
        logging.debug(_("Root partition size: %dMiB"), part_sizes['root'])

        if self.home:
            logging.debug(_("Home partition size: %dMiB"), part_sizes['home'])

    def run(self):
        key_files = ["/tmp/.keyfile-root", "/tmp/.keyfile-home"]

        # Partition sizes are expressed in MiB

        # Get just the disk size in MiB
        device = self.auto_device
        device_name = check_output("basename {0}".format(device))
        base_path = os.path.join("/sys/block", device_name)
        size_path = os.path.join(base_path, "size")
        if os.path.exists(size_path):
            logical_path = os.path.join(base_path, "queue/logical_block_size")
            with open(logical_path, 'r') as f:
                logical_block_size = int(f.read())
            with open(size_path, 'r') as f:
                size = int(f.read())
            disk_size = ((logical_block_size * size) / 1024) / 1024
        else:
            txt = _("Setup cannot detect size of your device, please use advanced "
                    "installation routine for partitioning and mounting devices.")
            logging.error(txt)
            raise InstallError(txt)

        start_part_sizes = 1

        part_sizes = self.get_part_sizes(disk_size, start_part_sizes)
        self.log_part_sizes(part_sizes)

        # Disable swap and all mounted partitions, umount / last!
        unmount_all(self.dest_dir)

        printk(False)

        # WARNING:
        # Our computed sizes are all in mebibytes (MiB) i.e. powers of 1024, not metric megabytes.
        # These are 'M' in sgdisk and 'MiB' in parted.
        # If you use 'M' in parted you'll get MB instead of MiB, and you're gonna have a bad time.

        if self.GPT:
            # Clean partition table to avoid issues!
            sgdisk("zap-all", device)

            # Clear all magic strings/signatures - mdadm, lvm, partition tables etc.
            dd("/dev/zero", device, bs=512, count=2048)
            wipefs(device)

            # Create fresh GPT
            sgdisk("clear", device)

            # Inform the kernel of the partition change. Needed if the hard disk had a MBR partition table.
            try:
                subprocess.check_call(["partprobe", device])
            except subprocess.CalledProcessError as err:
                txt = _("Error informing the kernel of the partition change.")
                logging.error(txt)
                logging.error(_("Command %s has failed."), err.cmd)
                logging.error(_("Output : %s"), err.output)
                raise InstallError(txt)

            part_num = 1

            if not self.UEFI:
                # We don't allow BIOS+GPT right now, so this code will be never executed
                # We leave here just for future reference
                # Create BIOS Boot Partition
                # GPT GUID: 21686148-6449-6E6F-744E-656564454649
                # This partition is not required if the system is UEFI based,
                # as there is no such embedding of the second-stage code in that case
                sgdisk_new(device, part_num, "BIOS_BOOT", gpt_bios_grub_part_size, "EF02")
                part_num += 1

            if self.bootloader == "grub2":
                # Create EFI System Partition (ESP)
                # GPT GUID: C12A7328-F81F-11D2-BA4B-00A0C93EC93B
                sgdisk_new(device, part_num, "UEFI_SYSTEM", part_sizes['efi'], "EF00")
                part_num += 1

            # Create Boot partition
            if self.bootloader == "gummiboot":
                sgdisk_new(device, part_num, "APRICITY_BOOT", part_sizes['boot'], "EF00")
            else:
                sgdisk_new(device, part_num, "APRICITY_BOOT", part_sizes['boot'], "8300")
            part_num += 1

            if self.lvm:
                # Create partition for lvm (will store root, swap and home (if desired) logical volumes)
                sgdisk_new(device, part_num, "APRICITY_LVM", part_sizes['lvm_pv'], "8E00")
                part_num += 1
            else:
                sgdisk_new(device, part_num, "APRICITY_ROOT", part_sizes['root'], "8300")
                part_num += 1
                if self.home:
                    sgdisk_new(device, part_num, "APRICITY_HOME", part_sizes['home'], "8302")
                    part_num += 1
                sgdisk_new(device, part_num, "APRICITY_SWAP", 0, "8200")

            logging.debug(check_output("sgdisk --print {0}".format(device)))
        else:
            # DOS MBR partition table
            # Start at sector 1 for 4k drive compatibility and correct alignment
            # Clean partitiontable to avoid issues!
            dd("/dev/zero", device, bs=512, count=2048)
            wipefs(device)

            # Create DOS MBR
            parted_mktable(device, "msdos")

            # Create boot partition (all sizes are in MiB)
            # if start is -1 parted_mkpart assumes that our partition starts at 1 (first partition in disk)
            start = -1
            end = part_sizes['boot']
            parted_mkpart(device, "primary", start, end)

            # Set boot partition as bootable
            parted_set(device, "1", "boot", "on")

            if self.lvm:
                # Create partition for lvm (will store root, home (if desired), and swap logical volumes)
                start = end
                end = start + part_sizes['lvm_pv']
                parted_mkpart(device, "primary", start, end)

                # Set lvm flag
                parted_set(device, "2", "lvm", "on")
            else:
                # Create root partition
                start = end
                end = start + part_sizes['root']
                parted_mkpart(device, "primary", start, end)

                if self.home:
                    # Create home partition
                    start = end
                    end = start + part_sizes['home']
                    parted_mkpart(device, "primary", start, end)

                # Create an extended partition where we will put our swap partition
                start = end
                end = start + part_sizes['swap']
                parted_mkpart(device, "extended", start, end)

                # Now create a logical swap partition
                start += 1
                parted_mkpart(device, "logical", start, end, "linux-swap")

        printk(True)

        # Wait until /dev initialized correct devices
        subprocess.check_call(["udevadm", "settle"])

        devices = self.get_devices

        if self.GPT and self.bootloader == "grub2":
            logging.debug("EFI: %s", devices['efi'])

        logging.debug("Boot: %s", devices['boot'])
        logging.debug("Root: %s", devices['root'])

        if self.home:
            logging.debug("Home: %s", devices['home'])

        logging.debug("Swap: %s", devices['swap'])

        if self.luks:
            setup_luks(devices['luks'], "cryptApricity", self.luks_password, key_files[0])
            if self.home and not self.lvm:
                setup_luks(devices['luks2'], "cryptApricityHome", self.luks_password, key_files[1])

        if self.lvm:
            logging.debug(_("Cnchi will setup LVM on device %s"), devices['lvm'])

            try:
                subprocess.check_call(["pvcreate", "-f", "-y", devices['lvm']])
            except subprocess.CalledProcessError as err:
                txt = _("Error creating LVM physical volume")
                logging.error(txt)
                logging.error(_("Command %s failed"), err.cmd)
                logging.error(_("Output: %s"), err.output)
                raise InstallError(txt)

            try:
                subprocess.check_call(["vgcreate", "-f", "-y", "ApricityVG", devices['lvm']])
            except subprocess.CalledProcessError as err:
                txt = _("Error creating LVM volume group")
                logging.error(txt)
                logging.error(_("Command %s failed"), err.cmd)
                logging.error(_("Output: %s"), err.output)
                raise InstallError(txt)

            # Fix issue 180
            # Check space we have now for creating logical volumes
            vg_info = check_output("vgdisplay -c ApricityVG")
            # Get column number 12: Size of volume group in kilobytes
            vg_size = int(vg_info.split(":")[11]) / 1024
            if part_sizes['lvm_pv'] > vg_size:
                logging.debug("Real ApricityVG volume group size: %d MiB", vg_size)
                logging.debug("Reajusting logical volume sizes")
                diff_size = part_sizes['lvm_pv'] - vg_size
                part_sizes = self.get_part_sizes(disk_size - diff_size, start_part_sizes)
                self.log_part_sizes(part_sizes)

            # Create LVM volumes
            try:
                size = str(int(part_sizes['root']))
                cmd = ["lvcreate", "--name", "ApricityRoot", "--size", size, "ApricityVG"]
                subprocess.check_call(cmd)

                if not self.home:
                    # Use the remainig space for our swap volume
                    cmd = ["lvcreate", "--name", "ApricitySwap", "--extents", "100%FREE", "ApricityVG"]
                    subprocess.check_call(cmd)
                else:
                    size = str(int(part_sizes['swap']))
                    cmd = ["lvcreate", "--name", "ApricitySwap", "--size", size, "ApricityVG"]
                    subprocess.check_call(cmd)
                    # Use the remaining space for our home volume
                    cmd = ["lvcreate", "--name", "ApricityHome", "--extents", "100%FREE", "ApricityVG"]
                    subprocess.check_call(cmd)
            except subprocess.CalledProcessError as err:
                txt = _("Error creating LVM logical volume")
                logging.error(txt)
                logging.error(_("Command %s failed"), err.cmd)
                logging.error(_("Output: %s"), err.output)
                raise InstallError(txt)

        # We have all partitions and volumes created. Let's create its filesystems with mkfs.

        mount_points = {
            'efi': '/boot/efi',
            'boot': '/boot',
            'root': '/',
            'home': '/home',
            'swap': ''}

        labels = {
            'efi': 'UEFI_SYSTEM',
            'boot': 'ApricityBoot',
            'root': 'ApricityRoot',
            'home': 'ApricityHome',
            'swap': 'ApricitySwap'}

        fs_devices = self.get_fs_devices()

        # Note: Make sure the "root" partition is defined first!
        self.mkfs(devices['root'], fs_devices[devices['root']], mount_points['root'], labels['root'])
        self.mkfs(devices['swap'], fs_devices[devices['swap']], mount_points['swap'], labels['swap'])

        if self.GPT and self.bootloader == "gummiboot":
            # Format EFI System Partition (ESP) with vfat (fat32)
            self.mkfs(devices['boot'], fs_devices[devices['boot']], mount_points['boot'], labels['boot'], "-F 32")
        else:
            self.mkfs(devices['boot'], fs_devices[devices['boot']], mount_points['boot'], labels['boot'])

        # Note: Make sure the "boot" partition is defined before the "efi" one!
        if self.GPT and self.bootloader == "grub2":
            # Format EFI System Partition (ESP) with vfat (fat32)
            self.mkfs(devices['efi'], fs_devices[devices['efi']], mount_points['efi'], labels['efi'], "-F 32")

        if self.home:
            self.mkfs(devices['home'], fs_devices[devices['home']], mount_points['home'], labels['home'])

        # NOTE: encrypted and/or lvm2 hooks will be added to mkinitcpio.conf in process.py if necessary
        # NOTE: /etc/default/grub, /etc/stab and /etc/crypttab will be modified in process.py, too.

        if self.luks and self.luks_password == "":
            # Copy root keyfile to boot partition and home keyfile to root partition
            # user will choose what to do with it
            # THIS IS NONSENSE (BIG SECURITY HOLE), BUT WE TRUST THE USER TO FIX THIS
            # User shouldn't store the keyfiles unencrypted unless the medium itself is reasonably safe
            # (boot partition is not)

            try:
                os.chmod(key_files[0], 0o400)
                cmd = ['mv', key_files[0], os.path.join(self.dest_dir, "boot")]
                subprocess.check_call(cmd)
                if self.home and not self.lvm:
                    os.chmod(key_files[1], 0o400)
                    luks_dir = os.path.join(self.dest_dir, 'etc/luks-keys')
                    os.makedirs(luks_dir, mode=0o755, exist_ok=True)
                    subprocess.check_call(['mv', key_files[1], luks_dir])
            except subprocess.CalledProcessError as err:
                txt = _("Can't copy LUKS keyfile to the installation device")
                logging.warning(txt)
                logging.warning(_("Command %s failed"), err.cmd)
                logging.warning(_("Output: %s"), err.output)


if __name__ == '__main__':
    import gettext

    _ = gettext.gettext

    logging.basicConfig(filename="/tmp/cnchi-autopartition.log", level=logging.DEBUG)

    auto = AutoPartition(
        dest_dir="/install",
        auto_device="/dev/sdb",
        use_luks=True,
        luks_password="luks",
        use_lvm=True,
        use_home=True,
        callback_queue=None)
    auto.run()
