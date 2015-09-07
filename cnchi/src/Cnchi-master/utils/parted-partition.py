#
# partition.py
# Python bindings for libparted (built on top of the _ped Python module).
#
# Copyright (C) 2009-2013 Red Hat, Inc.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions of
# the GNU General Public License v.2, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY expressed or implied, including the implied warranties of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.  You should have received a copy of the
# GNU General Public License along with this program; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.  Any Red Hat trademarks that are incorporated in the
# source code or documentation are not subject to the GNU General Public
# License and may only be used or replicated with the express permission of
# Red Hat, Inc.
#
# Author(s): Chris Lumens <clumens@redhat.com>
#            David Cantrell <dcantrell@redhat.com>
#            Alex Skinner <alex@lx.lc>
#

import math
import string
import warnings

import _ped
import parted

from .decorators import localeC

# XXX: add docstrings

class Partition(object):
    @localeC
    def __init__(self, disk=None, type=None, fs=None, geometry=None, PedPartition=None):
        if PedPartition is None:
            if disk is None:
                raise parted.PartitionException("no disk specified")
            elif type is None:
                raise parted.PartitionException("no type specified")
            elif geometry is None:
                raise parted.PartitionException("no geometry specified")

            self._fileSystem = fs
            self._geometry = geometry
            self._disk = disk

            if fs is None:
                self.__partition = _ped.Partition(disk.getPedDisk(), type, geometry.start, geometry.end)
            else:
                self.__partition = _ped.Partition(disk.getPedDisk(), type, geometry.start, geometry.end, parted.fileSystemType[fs.type])
        else:
            self.__partition = PedPartition
            self._geometry = parted.Geometry(PedGeometry=self.__partition.geom)

            if disk is None:
                self._disk = parted.Disk(PedDisk=self.__partition.disk)
            else:
                self._disk = disk

            if self.__partition.fs_type is None:
                self._fileSystem = None
            else:
                self._fileSystem = parted.FileSystem(type=self.__partition.fs_type.name, geometry=self._geometry)

    def __eq__(self, other):
        return not self.__ne__(other)

    def __ne__(self, other):
        if hash(self) == hash(other):
            return False

        if type(self) != type(other):
            return True

        return self.path != other.path or self.type != other.type or self.geometry != other.geometry or self.fileSystem != other.fileSystem

    def __str__(self):
        try:
            name = self.name
        except parted.PartitionException:
            name = None

        s = ("parted.Partition instance --\n"
             "  disk: %(disk)r  fileSystem: %(fileSystem)r\n"
             "  number: %(number)s  path: %(path)s  type: %(type)s\n"
             "  name: %(name)s  active: %(active)s  busy: %(busy)s\n"
             "  geometry: %(geometry)r  PedPartition: %(ped)r" %
             {"disk": self.disk, "fileSystem": self.fileSystem, "geometry": self.geometry,
              "number": self.number, "path": self.path,
              "type": self.type, "name": name, "active": self.active,
              "busy": self.busy, "ped": self.__partition})
        return s

    def __writeOnly(self, property):
        raise parted.WriteOnlyProperty(property)

    @property
    @localeC
    def active(self):
        """True if the partition is active, False otherwise."""
        return bool(self.__partition.is_active())

    @property
    @localeC
    def busy(self):
        """True if the partition is active, False otherwise."""
        return bool(self.__partition.is_busy())

    @property
    def disk(self):
        """The Disk this partition belongs to."""
        return self._disk

    @property
    @localeC
    def path(self):
        """The filesystem path to this partition's device node."""
        return self.__partition.get_path()

    @property
    @localeC
    def name(self):
        """The name of this partition."""
        try:
            return self.__partition.get_name()
        except parted.PartitionException as msg:
            return None

    @property
    def number(self):
        """The partition number."""
        return self.__partition.num

    fileSystem = property(lambda s: s._fileSystem, lambda s, v: setattr(s, "_fileSystem", v))
    geometry = property(lambda s: s._geometry, lambda s, v: setattr(s, "_geometry", v))
    system = property(lambda s: s.__writeOnly("system"), lambda s, v: s.__partition.set_system(v))
    type = property(lambda s: s.__partition.type, lambda s, v: setattr(s.__partition, "type", v))

    @localeC
    def getFlag(self, flag):
        """Get the value of a particular flag on the partition.  Valid flags
           are the _ped.PARTITION_* constants.  See _ped.flag_get_name() and
           _ped.flag_get_by_name() for more help working with partition flags.
        """
        return self.__partition.get_flag(flag)

    @localeC
    def setFlag(self, flag):
        """Set the flag on a partition to the provided value.  On error, a
           PartitionException will be raised.  See getFlag() for more help on
           working with partition flags."""
        return self.__partition.set_flag(flag, 1)

    @localeC
    def unsetFlag(self, flag):
        """Unset the flag on this Partition.  On error, a PartitionException
           will be raised.  See getFlag() for more help on working with
           partition flags."""
        return self.__partition.set_flag(flag, 0)

    @localeC
    def getMaxGeometry(self, constraint):
        """Given a constraint, return the maximum Geometry that self can be
           grown to.  Raises Partitionexception on error."""
        return parted.Geometry(PedGeometry=self.disk.getPedDisk().get_max_partition_geometry(self.__partition, constraint.getPedConstraint()))

    @localeC
    def isFlagAvailable(self, flag):
        """Return True if flag is available on this Partition, False
           otherwise."""
        return self.__partition.is_flag_available(flag)

    @localeC
    def nextPartition(self):
        """Return the Partition following this one on the Disk."""
        partition = self.disk.getPedDisk().next_partition(self.__partition)

        if partition is None:
            return None
        else:
            return parted.Partition(disk=self.disk, PedPartition=partition)

    @localeC
    def getSize(self, unit="MB"):
        """Return the size of the partition in the unit specified.  The unit
           is given as a string corresponding to one of the following
           abbreviations:  b (bytes), KB (kilobytes), MB (megabytes), GB
           (gigabytes), TB (terabytes).  An invalid unit string will raise a
           SyntaxError exception.  The default unit is MB."""
        warnings.warn("use the getLength method", DeprecationWarning)
        return self.geometry.getSize(unit)

    @localeC
    def getLength(self, unit='sectors'):
        """Return the length of the partition in sectors. Optionally, a SI or
           IEC prefix followed by a 'B' may be given in order to convert the
           length into bytes. The allowed values include B, kB, MB, GB, TB, KiB,
           MiB, GiB, and TiB."""
        return self.geometry.getLength(unit)

    def getFlagsAsString(self):
        """Return a comma-separated string representing the flags
           on this partition."""
        flags = []

        for flag in partitionFlag.keys():
            if self.getFlag(flag):
                flags.append(partitionFlag[flag])

        return string.join(flags, ', ')

    def getMaxAvailableSize(self, unit="MB"):
        """Return the maximum size this Partition can grow to by looking
           at contiguous freespace partitions.  The size is returned in
           the unit specified (default is megabytes).  The unit is a
           string corresponding to one of the following abbreviations:
           b (bytes), KB (kilobytes), MB (megabytes), GB (gigabytes),
           TB (terabytes).  An invalid unit string will raise a
           SyntaxError exception."""
        lunit = unit.lower()

        if lunit not in parted._exponent.keys():
            raise SyntaxError("invalid unit %s given" % (unit))

        maxLength = self.geometry.length
        sectorSize = self.geometry.device.sectorSize

        for partition in self.disk.partitions:
            if partition.type & parted.PARTITION_FREESPACE:
                maxLength += partition.geometry.length
            else:
                break

        return math.floor(maxLength * math.pow(sectorSize, parted._exponent[lunit]))

    def getDeviceNodeName(self):
        """Return the device name for this Partition."""
        return self.path[5:]

    def getPedPartition(self):
        """Return the _ped.Partition object contained in this Partition.
           For internal module use only."""
        return self.__partition

# collect all partition flags and store them in a hash
partitionFlag = {}
__flag = _ped.partition_flag_next(0)
partitionFlag[__flag] = _ped.partition_flag_get_name(__flag)
__readFlags = True

while __readFlags:
    try:
        __flag = _ped.partition_flag_next(__flag)
        if not __flag:
            __readFlags = False
        else:
            partitionFlag[__flag] = _ped.partition_flag_get_name(__flag)
    except ValueError:
        __readFlags = False
