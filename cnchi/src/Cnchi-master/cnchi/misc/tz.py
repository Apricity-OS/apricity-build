#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  tz.py
#
#  Copyright (C) 2006, 2007 Canonical Ltd.
#  Written by Colin Watson <cjwatson@ubuntu.com>.
#  New modifications Copyright © 2015 Apricity
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
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

from __future__ import print_function

import os
import datetime
import time
import xml.dom.minidom
import hashlib
import sys
import logging

from gi.repository import GObject, GLib

TZ_DATA_FILE = '/usr/share/zoneinfo/zone.tab'
ISO_3166_FILE = '/usr/share/xml/iso-codes/iso_3166.xml'


def _seconds_since_epoch(dt):
    # TODO cjwatson 2006-02-23: %s escape is not portable
    return int(dt.replace(tzinfo=None).strftime('%s'))


class SystemTzInfo(datetime.tzinfo):
    def __init__(self, tz=None):
        self.tz = tz

    def _select_tz(self):
        tzbackup = None
        if 'TZ' in os.environ:
            tzbackup = os.environ['TZ']
        if self.tz is not None:
            os.environ['TZ'] = self.tz
        time.tzset()
        return tzbackup

    def _restore_tz(self, tzbackup):
        if tzbackup is None:
            if 'TZ' in os.environ:
                del os.environ['TZ']
        else:
            os.environ['TZ'] = tzbackup
        time.tzset()

    def utcoffset(self, dt):
        tzbackup = self._select_tz()
        try:
            if time.daylight == 0:
                # no DST information
                dstminutes = -time.timezone / 60
            else:
                localtime = time.localtime(_seconds_since_epoch(dt))
                if localtime.tm_isdst != 1:
                    # not in DST
                    dstminutes = -time.timezone / 60
                else:
                    # in DST
                    dstminutes = -time.altzone / 60
            return datetime.timedelta(minutes=int(dstminutes))
        finally:
            self._restore_tz(tzbackup)

    def get_daylight(self):
        tzbackup = self._select_tz()
        daylight = time.daylight
        self._restore_tz(tzbackup)
        return daylight

    def is_dst(self, dt):
        tzbackup = self._select_tz()
        localtime = time.localtime(_seconds_since_epoch(dt))
        isdst = localtime.tm_isdst
        self._restore_tz(tzbackup)
        return isdst

    def rawutcoffset(self, unused_dt):
        tzbackup = self._select_tz()
        try:
            dstminutes = -time.timezone / 60
            return datetime.timedelta(minutes=int(dstminutes))
        finally:
            self._restore_tz(tzbackup)

    def dst(self, dt):
        tzbackup = self._select_tz()
        try:
            if time.daylight == 0:
                # no DST information, so assume no DST; None would be more
                # accurate but causes awkwardness in fromutc()
                return datetime.timedelta(0)
            else:
                localtime = time.localtime(_seconds_since_epoch(dt))
                if localtime.tm_isdst != 1:
                    # not in DST
                    return datetime.timedelta(0)
                else:
                    dstminutes = (time.timezone - time.altzone) / 60
                    return datetime.timedelta(minutes=int(dstminutes))
        finally:
            self._restore_tz(tzbackup)

    def tzname(self, unused_dt):
        return self.tz

    def tzname_letters(self, dt):
        tzbackup = self._select_tz()
        try:
            localtime = time.localtime(_seconds_since_epoch(dt))
            return time.strftime('%Z', localtime)
        finally:
            self._restore_tz(tzbackup)


class Iso3166(object):
    def __init__(self):
        self.names = {}
        document = xml.dom.minidom.parse(ISO_3166_FILE)
        entries = document.getElementsByTagName('iso_3166_entries')[0]
        self.handle_entries(entries)

    def handle_entries(self, entries):
        for entry in entries.getElementsByTagName('iso_3166_entry'):
            self.handle_entry(entry)

    def handle_entry(self, entry):
        if (entry.hasAttribute('alpha_2_code') and
                (entry.hasAttribute('common_name') or
                 entry.hasAttribute('name'))):
            alpha_2_code = entry.getAttribute('alpha_2_code')
            if entry.hasAttribute('common_name'):
                name = entry.getAttribute('common_name')
            else:
                name = entry.getAttribute('name')
            self.names[alpha_2_code] = name


# Much of the Location and Database classes are a rough translation of
# gnome-system-tools/src/time/tz.c. Thanks to Hans Petter Jansson
# <hpj@ximian.com> for that.

def _parse_position(position, wholedigits):
    if position == '' or len(position) < 4 or wholedigits > 9:
        return 0.0
    wholestr = position[:wholedigits + 1]
    fractionstr = position[wholedigits + 1:]
    whole = float(wholestr)
    fraction = float(fractionstr)
    if whole >= 0.0:
        return whole + fraction / pow(10.0, len(fractionstr))
    else:
        return whole - fraction / pow(10.0, len(fractionstr))


class Location(object):
    __gtype_name__ = "Location"
    __gproperties__ = {
        'zone': (GObject.TYPE_STRING, 'zone', None, 'zone', GObject.ParamFlags.READWRITE),
        'latitude': (GObject.TYPE_FLOAT, 'latitude', 'latitude', 0, GLib.MAXFLOAT, 1, GObject.ParamFlags.READWRITE),
        'longitude': (GObject.TYPE_FLOAT, 'longitude', 'longitude', 0, GLib.MAXFLOAT, 1, GObject.ParamFlags.READWRITE),
        'human_country': (GObject.TYPE_STRING, 'human_country', None, 'human_country', GObject.ParamFlags.READWRITE)}

    def get_info(self):
        return self.info

    def is_dst(self):
        return self.isdst

    def get_utc_offset(self):
        return self.utc_offset

    def get_raw_utc_offset(self):
        return self.raw_utc_offset

    def __init__(self, zonetab_line, iso3166):
        bits = zonetab_line.rstrip().split('\t', 3)
        latlong = bits[1]
        latlongsplit = latlong.find('-', 1)

        if latlongsplit == -1:
            latlongsplit = latlong.find('+', 1)

        if latlongsplit != -1:
            latitude = latlong[:latlongsplit]
            longitude = latlong[latlongsplit:]
        else:
            latitude = latlong
            longitude = '+0'

        self.country = bits[0]
        if self.country in iso3166.names:
            self.human_country = iso3166.names[self.country]
        else:
            self.human_country = self.country
        self.zone = bits[2]
        self.human_zone = self.zone.replace('_', ' ').split('/')[-1]
        if len(bits) > 3:
            self.comment = bits[3]
        else:
            self.comment = None

        self.latitude = _parse_position(latitude, 2)
        self.longitude = _parse_position(longitude, 3)

        # Grab md5sum of the timezone file for later comparison
        try:
            zone_path = os.path.join('/usr/share/zoneinfo', self.zone)
            with open(zone_path, 'rb') as tz_file:
                self.md5sum = hashlib.md5(tz_file.read()).digest()
        except IOError:
            self.md5sum = None

        try:
            today = datetime.datetime.today()
        except (ValueError, OverflowError):
            # Some versions of Python have problems with clocks set before
            # the epoch (http://python.org/sf/1646728). Assuming that the
            # time is set to the epoch will at least let us avoid crashing,
            # although the UTC offset and zone letters may be wrong.
            today = datetime.datetime.fromtimestamp(0)

        self.info = SystemTzInfo(self.zone)
        self.utc_offset = self.info.utcoffset(today)
        self.raw_utc_offset = self.info.rawutcoffset(today)
        self.zone_letters = self.info.tzname_letters(today)
        self.isdst = self.info.is_dst(today)

    def get_property(self, prop):
        return getattr(self, prop)

    def set_property(self, prop, value):
        setattr(self, prop, value)


class _Database(object):
    def __init__(self):
        self.locations = []
        iso3166 = Iso3166()
        with open(TZ_DATA_FILE) as tzdata:
            for line in tzdata:
                if line.startswith('#'):
                    continue
                self.locations.append(Location(line, iso3166))

        # Build mappings from timezone->location and country->locations
        self.cc_to_locs = {}
        self.tz_to_loc = {}
        for loc in self.locations:
            self.tz_to_loc[loc.zone] = loc
            if loc.country in self.cc_to_locs:
                # self.cc_to_locs[loc.country] += [loc]
                self.cc_to_locs[loc.country].append(loc)
            else:
                self.cc_to_locs[loc.country] = [loc]

    def get_loc(self, tz):
        try:
            return self.tz_to_loc[tz]
        except:
            # Sometimes we'll encounter timezones that aren't really
            # city-zones, like "US/Eastern" or "Mexico/General".  So first,
            # we check if the timezone is known.  If it isn't, we search for
            # one with the same md5sum and make a reference to it
            try:
                zone_path = os.path.join('/usr/share/zoneinfo', tz)
                with open(zone_path, 'rb') as tz_file:
                    md5sum = hashlib.md5(tz_file.read()).digest()

                for loc in self.locations:
                    if md5sum == loc.md5sum:
                        self.tz_to_loc[tz] = loc
                        return loc
            except IOError:
                pass

            # If not found, oh well, just warn and move on.
            logging.error('Could not understand timezone %s', tz)
            self.tz_to_loc[tz] = None  # save it for the future
            return None

    def get_locations(self):
        return self.locations


_database = None


def Database():
    global _database
    if not _database:
        _database = _Database()
    return _database
