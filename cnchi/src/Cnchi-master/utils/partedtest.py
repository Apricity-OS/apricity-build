#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  partedtest.py
#
#  Copyright 2013 Cinnarch
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#

import parted

def get_devices():
    device_list = parted.getAllDevices()
    disk_dic = {}
    for dev in device_list:
        #I left all of the below here but commented out to see some use cases
        #isbusy = in use/mounted.  Needs to flag if 'yes' to prompt user to umount
        #isbusy = dev.busy
        #path gives /dev/sda or something similar
        #myname = dev.path
        #Hard drives measure themselves assuming kilo=1000, mega=1mil, etc
        limiter = 1000
        #Some disk size calculations
        #byte_size = dev.length * dev.sectorSize
        #megabyte_size = byte_size / (limiter * limiter)
        #gigabyte_size = megabyte_size / limiter
        #print(byte_size)
        #print(dev.length)
        #Must create disk object to drill down
        diskob = parted.Disk(dev)
        disk_dic[dev.path] = diskob
    return disk_dic

def get_partitions(diskob):
    part_dic = {}
    #Do not let user specify more than this number of primary partitions
    disk_max_pri = diskob.maxPrimaryPartitionCount
    #create list of partitions for this device(/dev/sda for example)
    partition_list = diskob.partitions
    dev = diskob.device
    limiter = 1000
    for p in partition_list:
        part_dic[p.path] = p
        #this is start sector, end sector, and length
        #startbyte = p.geometry.start
        #endbyte = p.geometry.end
        #plength = p.geometry.length
        #print(startbyte, endbyte)
        #lets calcule its size in something ppl understand
        #psize = plength * dev.sectorSize
        #just calculating it in more sane formats
        #should probably add in something like
        #if psizemb < 1000 then display as MB, else as GB
        #I can't think of a case of less than 1mb partition
        #psizemb = psize / (limiter * limiter)
        #psizegb = psizemb / limiter
        #grabs the filesystem type
        #if p.fileSystem:
        #    ptype = p.fileSystem.type
        #else:
        #    ptype = None
        #print(ptype)
        #print(p.type)
    free_list = diskob.getFreeSpacePartitions()
    fcount = 0
    #because I honestly don't know the right answer, let's reserve the first 2048 sectors.
    for f in free_list:
        if f.geometry.end < 2048:
            continue
        else:
            if f.geometry.start < 2048:
                f.geometry.start = 2048
        part_dic['free%s' % str(fcount)] = f
        fcount += 1
    return part_dic


def delete_partition(diskob, part):
    diskob.deletePartition(part)


def create_partition(diskob, part_type, geom):
    #A lot of this is similar to Anaconda, but customized to fit our needs
    nstart = geom.start
    nend = geom.end
    # Just in case you try to create partition larger than disk.
    # This case should be caught in the frontend!
    # Never let user specify a length exceeding the free space.
    if nend > diskob.device.length - 1:
        nend = diskob.device.length -1
    nalign = diskob.partitionAlignment
    if not nalign.isAligned(geom, nstart):
        nstart = nalign.alignNearest(geom, nstart)
    if not nalign.isAligned(geom, nend):
        nstart = nalign.alignNearest(geom, nend)
    if part_type == 1:
        nstart += nalign.grainSize
    ngeom = parted.Geometry(device=diskob.device, start=nstart, end=nend)
    if diskob.maxPartitionLength < ngeom.length:
        print('Partition is too large!')
        sys.exit(1)
    npartition = parted.Partition(disk=diskob, type=part_type, geometry=ngeom, fs=fs)
    nconstraint = parted.Constraint(exactGeom=ngeom)
    ncont = diskob.addPartition(partition=npartition, constraint=nconstraint)

def geom_builder(diskob, start_sector, size_in_mbytes):
    #let's use kb = 1000b, mb = 10000000b, etc etc
    dev = diskob.device
    sec_size = dev.sectorSize
    length = (size_in_mbytes * 1000000 // sec_size)
    end_sector = start_sector + length - 1
    ngeom = parted.Geometry(device=dev, start=start_sector, end=end_sector)
    return ngeom

def check_mounted(part):
    #Simple check to see if partition is mounted
    if part.busy:
        return 1
    else:
        return 0

def get_largest_size(diskob, part):
    #Call this to set the initial size of new partition in frontend, but also the MAX to which user may enter.
    dev = diskob.device
    sec_size = dev.sectorSize
    mbs = (sec_size * part.length) / 1000000


def finalize_changes(diskob):
    diskob.commit()

def main():
    #This builds a dictionary to map disk objects to the common name
    #So for example, disk_dic['/dev/sda'] is that diskobject.
    #This should make it easy to translate from frontend to backend.
    disk_dic = get_devices()

    #This does the same thing, but for partitions.
    #In this example just using /dev/sdb as the device
    #Any useable free space is returned as a partition object, only for 'fluency' sake.  It's name will always be 'free#' where # is an incrementing number.
    part_dic = get_partitions(disk_dic['/dev/sdb'])

    #To delete a partition, pass to delete_partition the disk and the partition object.  So, to delete /dev/sdb1, do this.

    #First, make sure it's not mounted
    if not check_mounted(part_dic['/dev/sdb1']):
        delete_partition(disk_dic['/dev/sdb'], part_dic['/dev/sdb1'])
    else:
        print('/dev/sdb1 is mounted, unmount first')
        sys.exit(1)

    #Creating is a little tougher.  I give you two options here.  You may specify the geometry yourself, or use the geometry helper.  The arguments for this are diskobject, start sector, and size in mb.
    my_geometry = geom_builder(disk_dic['/dev/sdb'], 123456, 1000)

    #The above is optional, i'll try to explain why.  In part_dic, I return free regions as partitions of type 'free space'.  So, if a user wants to create a partition exactly in area of free space, you can use part_dic['free0'].geometry as the geometry, and skip building the geometry yourself.

    #The second argument here is the type.  Here is cheat sheet.
    #NORMAL            = 0
    #LOGICAL           = 1
    #EXTENDED          = 2
    #FREESPACE         = 4
    #METADATA          = 8
    #PROTECTED         = 10

    #On frontend, make sure that their 'normal' partition doesn't exceed allowed count if trying to create a new one.
    #Any disk may only have one extended partition, filled with logical.  There is a ton of checks to do on exceeding counts there...but for now let's assume our users aren't idiots.  The program would only crash if trying to break these rules anyhow.
    create_partition(disk_dic['/dev/sdb'], 0, my_geometry)

    #As you go, you can and should call get_partitions again.  Much like a database, changes you've made thus far aren't actually written.  So if you 'create' the above partition, it will show up if you were to again call get_partitions.  However, if you exit, no changes are made.  To finalize, use
    finalize_changes(disk_dic['/dev/sdb'])


