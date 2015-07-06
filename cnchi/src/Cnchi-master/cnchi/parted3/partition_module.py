#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  partition_module.py
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

""" Interacts with pyparted """

import subprocess
import os
import logging

import misc.misc as misc
import show_message as show

import parted

OK = 0
UNRECOGNISED_DISK_LABEL = -1
UNKNOWN_ERROR = -2

# Partition types
PARTITION_PRIMARY = 0
PARTITION_LOGICAL = 1
PARTITION_EXTENDED = 2
PARTITION_FREESPACE = 4
PARTITION_FREESPACE_EXTENDED = 5
PARTITION_METADATA = 8
PARTITION_PROTECTED = 10

# Disk types
DISK_EXTENDED = 1

# Partition flags
PED_PARTITION_BOOT = 1
PED_PARTITION_ROOT = 2
PED_PARTITION_SWAP = 3
PED_PARTITION_HIDDEN = 4
PED_PARTITION_RAID = 5
PED_PARTITION_LVM = 6
PED_PARTITION_LBA = 7
PED_PARTITION_HPSERVICE = 8
PED_PARTITION_PALO = 9
PED_PARTITION_PREP = 10
PED_PARTITION_MSFT_RESERVED = 11
PED_PARTITION_BIOS_GRUB = 12
PED_PARTITION_APPLE_TV_RECOVERY = 13
PED_PARTITION_DIAG = 14
PED_PARTITION_LEGACY_BOOT = 15


@misc.raise_privileges
def get_devices():
    device_list = parted.getAllDevices()
    disk_dic = {}

    myhomepath = '/run/archiso/bootmnt'
    if os.path.exists(myhomepath):
        myhome = subprocess.check_output(["df", "-P", myhomepath]).decode()
    else:
        myhome = ""

    for dev in device_list:
        if dev.path in myhome:
            continue

        # I left all of the below here but commented out to see some use cases
        # isbusy = in use/mounted.  Needs to flag if 'yes' to prompt user to umount
        # isbusy = dev.busy
        # path gives /dev/sda or something similar
        # myname = dev.path
        # Hard drives measure themselves assuming kilo=1000, mega=1mil, etc
        # limiter = 1000
        # Some disk size calculations
        # byte_size = dev.length * dev.sectorSize
        # megabyte_size = byte_size / (limiter * limiter)
        # gigabyte_size = megabyte_size / limiter
        # print(byte_size)
        # print(dev.length)
        # Must create disk object to drill down

        # Skip cd drive and special devices like LUKS and LVM
        disk_obj = None
        if not dev.path.startswith("/dev/sr") and not dev.path.startswith("/dev/mapper"):
            try:
                disk_obj = parted.Disk(dev)
                result = OK
            except parted.DiskLabelException:
                # logging.warning(_('Unrecognised disk label in device %s.'), dev.path)
                result = UNRECOGNISED_DISK_LABEL
            except Exception as general_error:
                logging.error(general_error)
                msg = _("Exception: {0}.\nFor more information take a look at /tmp/cnchi.log").format(general_error)
                show.error(None, msg)
                result = UNKNOWN_ERROR
            finally:
                disk_dic[dev.path] = (disk_obj, result)

    return disk_dic


def make_new_disk(dev_path, new_type):
    new_dev = parted.Device(dev_path)
    new_disk = parted.freshDisk(new_dev, new_type)
    return new_disk


@misc.raise_privileges
def get_partitions(diskob):
    part_dic = {}
    # Do not let user specify more than this number of primary partitions
    # disk_max_pri = diskob.maxPrimaryPartitionCount
    # Create list of partitions for this device(/dev/sda for example)
    if not diskob:
        return part_dic
    partition_list = diskob.partitions
    # dev = diskob.device
    # limiter = 1000
    for partition in partition_list:
        part_dic[partition.path] = partition
        # this is start sector, end sector, and length
        # startbyte = partition.geometry.start
        # endbyte = partition.geometry.end
        # plength = partition.geometry.length
        # print(startbyte, endbyte)
        # lets calcule its size in something ppl understand
        # psize = plength * dev.sectorSize
        # just calculating it in more sane formats
        # should probably add in something like
        # if psizemb < 1000 then display as MB, else as GB
        # I can't think of a case of less than 1mb partition
        # psizemb = psize / (limiter * limiter)
        # psizegb = psizemb / limiter
        # grabs the filesystem type
        # if partition.fileSystem:
        #    ptype = partition.fileSystem.type
        # else:
        #    ptype = None
        # print(ptype)
        # print(partition.type)
    free_list = diskob.getFreeSpacePartitions()
    fcount = 0
    # Because I honestly don't know the right answer, let's reserve the first 2048 sectors.
    for free in free_list:
        if free.geometry.end < 2048:
            continue
        else:
            if free.geometry.start < 2048:
                free.geometry.start = 2048
        if free.geometry.end - free.geometry.start < 2:
            continue
        # Is this str conversion necessary?
        part_dic['free{0}'.format(str(fcount))] = free
        fcount += 1

    return part_dic


@misc.raise_privileges
def delete_partition(diskob, part):
    try:
        diskob.deletePartition(part)
    except Exception as general_error:
        txt = _("Can't delete partition {0}").format(part)
        logging.error(txt)
        logging.error(general_error)
        debug_txt = "{0}\n{1}".format(txt, general_error)
        show.error(None, debug_txt)


def get_partition_size(diskob, part):
    dev = diskob.device
    sec_size = dev.sectorSize
    mbs = (sec_size * part.length) / 1000000
    return mbs

# length : geometry length


def get_size_txt(length, sector_size):
    size = length * sector_size
    size_txt = "%dk" % size

    if size >= 1000000:
        size /= 1000000
        size_txt = "%dM" % size

    if size >= 1000:
        size /= 1000
        size_txt = "%dG" % size

    return size_txt


@misc.raise_privileges
def create_partition(diskob, part_type, geom):
    # A lot of this is similar to Anaconda, but customized to fit our needs
    nstart = geom.start
    nend = geom.end
    if nstart < 2048:
        nstart = 2048
    # Just in case you try to create partition larger than disk.
    # This case should be caught in the frontend!
    # Never let user specify a length exceeding the free space.
    if nend > diskob.device.length - 1:
        nend = diskob.device.length - 1
    nalign = diskob.partitionAlignment
    if not nalign.isAligned(geom, nstart):
        nstart = nalign.alignNearest(geom, nstart)
    if not nalign.isAligned(geom, nend):
        nend = nalign.alignDown(geom, nend)
    if part_type == 1:
        nstart = nstart + nalign.grainSize
    mingeom = parted.Geometry(device=diskob.device, start=nstart, end=nend-1)
    maxgeom = parted.Geometry(device=diskob.device, start=nstart, end=nend)
    if diskob.maxPartitionLength < maxgeom.length:
        txt = _('Partition is too large!')
        logging.error(txt)
        show.error(None, txt)
        return None
    else:
        npartition = parted.Partition(disk=diskob, type=part_type, geometry=maxgeom)
        nconstraint = parted.Constraint(minGeom=mingeom, maxGeom=maxgeom)
        diskob.addPartition(partition=npartition, constraint=nconstraint)
        return npartition


def geom_builder(diskob, first_sector, last_sector, size_in_mbytes,
                 beginning=True):
    # OK, two new specs.  First, you must specify the first sector
    # and the last sector of the free space.  This way we can prevent out of
    # bound and math problems.  Currently, I use 5 sectors to 'round' to limits
    # We can change later to be the minimum allowed size for partition
    # However, if user is advanced and purposely wants to NOT include some
    # area of disk between 5 and smallest allowed partition, we should let him.
    #
    # beginning defaults to True.  This starts partition at beginning of
    # free space.  Specify to False to instead start at end
    # let's use kb = 1000b, mb = 10000000b, etc etc

    dev = diskob.device
    sec_size = dev.sectorSize
    mb = 1000000 / sec_size
    length = int(size_in_mbytes * 1000000 / sec_size)
    if length > (last_sector - first_sector + 1):
        length = last_sector - first_sector + 1
    if beginning:
        start_sector = first_sector
        end_sector = start_sector + length - 1
        if last_sector - end_sector < mb:
            end_sector = last_sector
    else:
        end_sector = last_sector
        start_sector = end_sector - length + 1
        if start_sector - first_sector < mb:
            start_sector = first_sector
    ngeom = parted.Geometry(device=dev, start=start_sector, end=end_sector)
    return ngeom


def check_mounted(part):
    # Simple check to see if partition is mounted (or busy)
    if part.busy:
        return 1
    else:
        return 0


def get_used_space(part):
    return get_used_space_from_path(part.path)


def get_used_space_from_path(path):
    try:
        cmd = ["df", "-H", path]
        result = subprocess.check_output(cmd).decode()
        lines = result.split('\n')
        used_space = lines[1].split()[2]
    except subprocess.CalledProcessError as process_error:
        used_space = 0
        txt = _("Can't detect used space from {0}").format(path)
        logging.error(txt)
        logging.error(process_error)
        debug_txt = "{0}\n{1}".format(txt, process_error)
        show.error(None, debug_txt)

    return used_space


def get_largest_size(diskob, part):
    # Call this to set the initial size of new partition in frontend, but also
    # the MAX to which user may enter.
    dev = diskob.device
    sec_size = dev.sectorSize
    mbs = (sec_size * part.length) / 1000000
    return mbs

# The return value is tuple.  First arg is 0 for success, 1 for fail
# Second arg is either None if successful
# or exception if failure


def set_flag(flagno, part):
    ret = (0, None)
    try:
        part.setFlag(flagno)
    except Exception as e:
        ret = (1, e)
    return ret


def unset_flag(flagno, part):
    ret = (0, None)
    try:
        part.setFlag(flagno)
    except Exception as e:
        ret = (1, e)
    return ret


def get_flags(part):
    return part.getFlagsAsString


def get_flag(part, flag):
    return part.getFlag(flag)


@misc.raise_privileges
def finalize_changes(diskob):
    diskob.commit()


def order_partitions(partdic):
    """ Pass the result of get_partitions here and it will return list
        of partitions in order as they are on disk """
    return sorted(partdic, key=lambda key: partdic[key].geometry.start)

# To shrink a partition:
# 1. Shrink fs
# 2. Shrink partition (resize)

# To expand a partition:
# 1. Expand partition
# 2. Expand fs (resize)


@misc.raise_privileges
def split_partition(device_path, partition_path, new_size_in_mb):
    """ Shrinks partition and splits it in two.
        ALERT: The file system must be resized before trying this! """

    disk_dic = get_devices()
    disk = disk_dic[device_path]

    part_dic = get_partitions(disk)
    part = part_dic[partition_path]

    if not check_mounted(part):
        delete_partition(disk, part)
    else:
        print(partition_path + ' is mounted, unmount first')
        return False

    # ok, partition deleted. Now we must create a new partition with
    # the new size

    sec_size = disk.sectorSize
    logging.debug("Sec size: %d", sec_size)

    # Get old info
    units = 1000000
    start_sector = part.geometry.start
    old_end_sector = part.gemotry.end
    old_length = part.geometry.length
    old_size_in_mb = old_length * sec_size / units

    # Create new partition (the one for the otherOS)
    new_length = int(new_size_in_mb * units / sec_size)
    new_end_sector = start_sector + new_length
    my_geometry = geom_builder(disk, start_sector, new_end_sector, new_size_in_mb)
    logging.debug("create_partition %s", my_geometry)
    create_partition(disk, 0, my_geometry)

    # Create new partition (for Antergos)
    new_size_in_mb = old_size_in_mb - new_size_in_mb
    start_sector = new_end_sector + 1
    end_sector = old_end_sector
    my_geometry = geom_builder(disk, start_sector, end_sector, new_size_in_mb)
    logging.debug("create_partition %s", my_geometry)
    create_partition(disk, 0, my_geometry)

    finalize_changes(disk)

# ----------------------------------------------------------------------------


def example():
    """ Usage example """
    # This builds a dictionary to map disk objects to the common name
    # So for example, disk_dic['/dev/sda'] is that diskobject.
    # This should make it easy to translate from frontend to backend.
    disk_dic = get_devices()

    # This does the same thing, but for partitions.
    # In this example just using /dev/sdb as the device
    # Any useable free space is returned as a partition object, only for 'fluency'
    # sake.  It's name will always be 'free#' where # is an incrementing number.
    (diskob, result) = disk_dic['/dev/sdb']
    part_dic = get_partitions(diskob)

    # To delete a partition, pass to delete_partition the disk and the partition
    # object.  So, to delete /dev/sdb1, do this.

    # First, make sure it's not mounted
    if not check_mounted(part_dic['/dev/sdb1']):
        delete_partition(disk_dic['/dev/sdb'], part_dic['/dev/sdb1'])
    else:
        logging.error('/dev/sdb1 is mounted, unmount it first!')
        return

    # Creating is a little tougher.  I give you two options here.  You may
    # specify the geometry yourself, or use the geometry helper.  The arguments
    # for this are diskobject, first sector of free space, the last
    # sector of free space, size in mb, and optionally, whether
    # to start at beginning or end using beginning=True or False
    # defaults to True
    my_geometry = geom_builder(disk_dic['/dev/sdb'], 123456, 567890, 1000)

    # The above is optional, i'll try to explain why.  In part_dic,
    # I return free regions as partitions of type 'free space'.  So, if a user
    # wants to create a partition exactly in area of free space, you can use
    # part_dic['free0'].geometry as the geometry, and skip building
    # the geometry yourself.

    # The second argument here is the type.  Here is cheat sheet.
    # NORMAL            = 0
    # LOGICAL           = 1
    # EXTENDED          = 2
    # FREESPACE         = 4
    # METADATA          = 8
    # PROTECTED         = 10

    # On frontend, make sure that their 'normal' partition doesn't exceed
    #  allowed count if trying to create a new one.
    # Any disk may only have one extended partition, filled with logical.
    # There is a ton of checks to do on exceeding counts there...but for now
    # let's assume our users aren't idiots.  The program would only crash if
    # trying to break these rules anyhow.
    create_partition(disk_dic['/dev/sdb'], 0, my_geometry)

    # As you go, you can and should call get_partitions again.  Much like a
    # database, changes you've made thus far aren't actually written.  So if
    # you 'create' the above partition, it will show up if you were to again
    # call get_partitions.  However, if you exit, no changes are made.
    # To finalize, use
    finalize_changes(disk_dic['/dev/sdb'])
