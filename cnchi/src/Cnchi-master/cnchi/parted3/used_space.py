#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  used_space.py
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

""" Get partition used space """

import subprocess
import shlex
import logging

import misc.misc as misc


@misc.raise_privileges
def get_used_ntfs(part):
    """ Gets used space in a NTFS partition """
    used = 0
    try:
        result = subprocess.check_output(["ntfsinfo", "-mf", part])
    except subprocess.CalledProcessError as err:
        result = None
        txt = _("Can't detect used space of NTFS partition %s")
        logging.error(txt, part)
        logging.error(err)

    if result:
        csize, vsize, fsize = (0, 0, 0)
        result = result.decode()
        lines = result.split('\n')
        for line in lines:
            if 'Cluster Size:' in line:
                # csize = int(line.split(':')[-1].strip())
                pass
            elif 'Volume Size in Clusters' in line:
                vsize = int(line.split(':')[-1].strip())
            elif 'Free Clusters:' in line:
                fsize = int(line.strip().split()[2])
        # FIXME: Is this ok? csize is never used!
        used = (vsize - fsize) / vsize
    return used


@misc.raise_privileges
def get_used_ext(part):
    """ Gets used space in an ext4 partition """
    used = 0
    try:
        result = subprocess.check_output(["dumpe2fs", "-h", part])
    except subprocess.CalledProcessError as err:
        result = None
        txt = _("Can't detect used space of EXTFS partition %s")
        logging.error(txt, part)
        logging.error(err)

    if result:
        csize, vsize, fsize = (0, 0, 0)
        result = result.decode()
        lines = result.split('\n')
        for line in lines:
            if "Block count:" in line:
                vsize = int(line.split(':')[-1].strip())
            elif "Free blocks:" in line:
                fsize = int(line.split(':')[-1].strip())
            elif "Block size:" in line:
                # csize = int(line.split(':')[-1].strip())
                pass
        # FIXME: Is this ok? csize is never used!
        used = (vsize - fsize) / vsize
    return used


@misc.raise_privileges
def get_used_fat(part):
    """ Gets used space in a FAT partition """
    used = 0
    try:
        result = subprocess.check_output(["dosfsck", "-n", "-v", part])
    except subprocess.CalledProcessError as err:
        if b'Dirty bit is set' in err.output:
            result = err.output
        else:
            result = None
            txt = _("Can't detect used space of FAT partition %s")
            logging.error(txt, part)
            logging.error(err)

    if result:
        bperc = 0
        cl = 0
        sbyte = 0
        ucl = 0
        result = result.decode()
        lines = result.split('\n')
        for line in lines:
            if 'bytes per cluster' in line:
                bperc = int(line.split()[0].strip())
            elif 'Data area starts at' in line:
                sbyte = int(line.split()[5])
            elif part in line:
                cl = int(line.split()[3].split('/')[1])
                ucl = int(line.split()[3].split('/')[0])
        used = (sbyte + (bperc * ucl)) / (bperc * cl)
    return used


@misc.raise_privileges
def get_used_jfs(part):
    """ Gets used space in a JFS partition """
    used = 0
    try:
        result = subprocess.check_output(["jfs_fsck", "-n", part])
    except subprocess.CalledProcessError as err:
        result = None
        txt = _("Can't detect used space of JFS partition %s")
        logging.error(txt, part)
        logging.error(err)

    if result:
        vsize, fsize = (0, 0)
        result = result.decode()
        lines = result.split('\n')
        for line in lines:
            if "kilobytes total disk space" in line:
                vsize = int(line.split()[0].strip())
            elif "kilobytes are available for use" in line:
                fsize = int(line.split()[0].strip())
        used = (vsize - fsize) / vsize
    return used


@misc.raise_privileges
def get_used_reiser(part):
    """ Gets used space in a REISER partition """
    used = 0
    try:
        result = subprocess.check_output(["debugreiserfs", "-d", part])
    except subprocess.CalledProcessError as err:
        result = None
        txt = _("Can't detect used space of REISERFS partition %s")
        logging.error(txt, part)
        logging.error(err)

    if result:
        vsize, fsize = (0, 0)

        # Added 'replace' parameter (not tested) as it fails decoding. See issue #90
        result = result.decode('utf-8', 'replace')

        lines = result.split('\n')
        for line in lines:
            if "Count of blocks on the device" in line:
                vsize = int(line.split()[-1].strip())
            elif "Free blocks (count of blocks" in line:
                fsize = int(line.split()[-1].strip())
        used = (vsize - fsize) / vsize
    return used


@misc.raise_privileges
def get_used_btrfs(part):
    """ Gets used space in a Btrfs partition """
    used = 0
    try:
        result = subprocess.check_output(["btrfs", "filesystem", "show", part])
    except Exception as err:
        result = None
        txt = _("Can't detect used space of BTRFS partition %s")
        logging.error(txt, part)
        logging.error(err)

    if result:
        vsize, usize, umult, vmult = (1, 1, 1, 1)
        result = result.decode()
        result = result.split('\n')
        szmap = {"K": 1000,
                 "M": 1000000,
                 "G": 1000000000,
                 "T": 1000000000000,
                 "P": 1000000000000000}
        for z in result:
            if part in z:
                vsize = z.split()[3]
                usize = z.split()[5]
                for i in szmap:
                    if i in vsize:
                        vmult = szmap[i]
                    if i in usize:
                        umult = szmap[i]
                usize = float(usize.strip("KMGTPBib")) * umult
                vsize = float(vsize.strip("KMGTPBib")) * vmult
        used = usize / vsize
    return used


@misc.raise_privileges
def get_used_xfs(part):
    """ Gets used space in a XFS partition """
    used = 0
    try:
        command = shlex.split("xfs_db -c 'sb 0' -c 'print dblocks' -c 'print fdblocks' -r {0}".format(part))
        result = subprocess.check_output(command)
    except subprocess.CalledProcessError as err:
        result = None
        txt = _("Can't detect used space of XFS partition %s")
        logging.error(txt, part)
        logging.error(err)

    if result:
        vsize, fsize = (1, 0)
        result = result.decode()
        lines = result.split('\n')
        for line in lines:
            if "fdblocks" in line:
                fsize = int(line.split()[-1].strip())
            elif "dblocks" in line:
                vsize = int(line.split()[-1].strip())
        used = (vsize - fsize) / vsize
    return used


@misc.raise_privileges
def get_used_f2fs(part):
    # TODO: Use a f2fs installation to check the output format when getting part info.
    used = 0
    return used


def is_btrfs(part):
    """ Checks if part is a Btrfs partition """
    space = get_used_btrfs(part)
    if not space:
        return False
    else:
        return True


def get_used_space(part, part_type):
    """ Get used space in a partition """

    part_type = part_type.lower()

    if 'ntfs' in part_type:
        space = get_used_ntfs(part)
    elif 'ext' in part_type:
        space = get_used_ext(part)
    elif 'fat' in part_type:
        space = get_used_fat(part)
    elif 'jfs' in part_type:
        space = get_used_jfs(part)
    elif 'reiser' in part_type:
        space = get_used_reiser(part)
    elif 'btrfs' in part_type:
        space = get_used_btrfs(part)
    elif 'xfs' in part_type:
        space = get_used_xfs(part)
    elif 'f2fs' in part_type:
        space = get_used_f2fs(part)
    else:
        space = 0
    return space
