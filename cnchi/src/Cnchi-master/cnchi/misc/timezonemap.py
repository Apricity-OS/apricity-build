#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  timezonemap.py
#
#  Original author: Thomas Wood <thomas.wood@intel.com>
#  Portions from Ubiquity, Copyright (C) 2009 Canonical Ltd.
#  Written in C by Evan Dandrea <evand@ubuntu.com>
#  Python port Copyright © 2015 Apricity
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

""" Custom widget to show world time zones """

from datetime import datetime
import os
import math
import sys
import logging

import gi
gi.require_version('PangoCairo', '1.0')

from gi.repository import GObject, GLib, Gdk, Gtk, GdkPixbuf, Pango, PangoCairo

import misc.tz as tz

try:
    import xml.etree.cElementTree as elementTree
except ImportError:
    import xml.etree.ElementTree as elementTree

PIN_HOT_POINT_X = 8
PIN_HOT_POINT_Y = 15
LOCATION_CHANGED = 0

G_PI_4 = 0.78539816339744830961566084581987572104929234984378

TIMEZONEMAP_IMAGES_PATH = "/usr/share/cnchi/data/images/timezonemap"
OLSEN_MAP_TIMEZONES_PATH = "/usr/share/cnchi/data/locale/timezones.xml"

BUBBLE_TEXT_FONT = "Sans 9"

# color_codes is (offset, red, green, blue, alpha)
color_codes = [
    (-11.0, 43, 0, 0, 255),
    (-10.0, 85, 0, 0, 255),
    (-9.5, 102, 255, 0, 255),
    (-9.0, 128, 0, 0, 255),
    (-8.0, 170, 0, 0, 255),
    (-7.0, 212, 0, 0, 255),
    (-6.0, 255, 0, 1, 255),  # north
    (-6.0, 255, 0, 0, 255),  # south
    (-5.0, 255, 42, 42, 255),
    (-4.5, 192, 255, 0, 255),
    (-4.0, 255, 85, 85, 255),
    (-3.5, 0, 255, 0, 255),
    (-3.0, 255, 128, 128, 255),
    (-2.0, 255, 170, 170, 255),
    (-1.0, 255, 213, 213, 255),
    (0.0, 43, 17, 0, 255),
    (1.0, 85, 34, 0, 255),  # eastern Europe
    (2.0, 128, 51, 0, 255),
    (3.0, 170, 68, 0, 255),
    (3.5, 0, 255, 102, 255),
    (4.0, 212, 85, 0, 255),
    (4.5, 0, 204, 255, 255),
    (5.0, 255, 102, 0, 255),
    (5.5, 0, 102, 255, 255),
    (5.75, 0, 238, 207, 247),
    (6.0, 255, 127, 42, 255),
    (6.5, 204, 0, 254, 254),
    (7.0, 255, 153, 85, 255),
    (8.0, 255, 179, 128, 255),
    (9.0, 255, 204, 170, 255),
    (9.5, 170, 0, 68, 250),
    (10.0, 255, 230, 213, 255),
    (10.5, 212, 124, 21, 250),
    (11.0, 212, 170, 0, 255),
    (11.5, 249, 25, 87, 253),
    (12.0, 255, 204, 0, 255),
    (12.75, 254, 74, 100, 248),
    (13.0, 255, 85, 153, 250)]


class TimezoneMap(Gtk.Widget):
    __gtype_name__ = 'TimezoneMap'

    __gsignals__ = {'location-changed': (GObject.SignalFlags.RUN_LAST, None, (object,))}

    def __init__(self):
        Gtk.Widget.__init__(self)

        self.set_size_request(40, 40)

        self._background = None
        self._color_map = None
        self._olsen_map = None

        self._selected_offset = 0.0
        self._show_offset = False

        self._tz_location = None

        self._bubble_text = ""

        self.olsen_map_timezones = []
        self.load_olsen_map_timezones()

        try:
            self._orig_background = GdkPixbuf.Pixbuf.new_from_file(
                os.path.join(TIMEZONEMAP_IMAGES_PATH, "bg.png"))

            self._orig_background_dim = GdkPixbuf.Pixbuf.new_from_file(
                os.path.join(TIMEZONEMAP_IMAGES_PATH, "bg_dim.png"))

            self._orig_color_map = GdkPixbuf.Pixbuf.new_from_file(
                os.path.join(TIMEZONEMAP_IMAGES_PATH, "cc.png"))

            self._olsen_map = GdkPixbuf.Pixbuf.new_from_file(
                os.path.join(TIMEZONEMAP_IMAGES_PATH, "olsen_map.png"))

            self._pin = GdkPixbuf.Pixbuf.new_from_file(
                os.path.join(TIMEZONEMAP_IMAGES_PATH, "pin.png"))
        except Exception as err:
            print(err)
            sys.exit(1)

        self.tzdb = tz.Database()

    def load_olsen_map_timezones(self):
        try:
            tree = elementTree.parse(OLSEN_MAP_TIMEZONES_PATH)
            self.olsen_map_timezones = []
            root = tree.getroot()
            for tz_name in root.iter("timezone_name"):
                self.olsen_map_timezones.append(tz_name.text)
        except FileNotFoundError as err:
            logging.error(err)
            print(err)

    def do_get_preferred_width(self):
        """ Retrieves a widget’s initial minimum and natural width. """
        width = self._orig_background.get_width()

        # Images are bigger but we need this widget to stay small as Cnchi's window
        # is small (so it works with low res systems)
        if width > 400:
            width = 400

        return width, width

    def do_get_preferred_height(self):
        """ Retrieves a widget’s initial minimum and natural height. """
        height = self._orig_background.get_height()

        # Images are bigger but we need this widget to stay small as Cnchi's window
        # is small (so it works with low res systems)
        if height > 200:
            height = 200

        return height, height

    def do_size_allocate(self, allocation):
        """ The do_size_allocate is called by when the actual size is known
         and the widget is told how much space could actually be allocated """
        self.set_allocation(allocation)

        if self._background is not None:
            del self._background
            self._background = None

        if self.is_sensitive():
            self._background = self._orig_background.scale_simple(
                allocation.width,
                allocation.height,
                GdkPixbuf.InterpType.BILINEAR)
        else:
            self._background = self._orig_background_dim.scale_simple(
                allocation.width,
                allocation.height,
                GdkPixbuf.InterpType.BILINEAR)

        if self._color_map is not None:
            del self._color_map
            self._color_map = None

        self._color_map = self._orig_color_map.scale_simple(
            allocation.width,
            allocation.height,
            GdkPixbuf.InterpType.BILINEAR)

        # self._visible_map_pixels = self._color_map.get_pixels()
        # self._visible_map_rowstride = self._color_map.get_rowstride()

        if self.get_realized():
            self.get_window().move_resize(
                allocation.x,
                allocation.y,
                allocation.width,
                allocation.height)

    def do_realize(self):
        """ Called when the widget should create all of its
        windowing resources.  We will create our window here """
        self.set_realized(True)
        allocation = self.get_allocation()
        attr = Gdk.WindowAttr()
        attr.window_type = Gdk.WindowType.CHILD
        attr.wclass = Gdk.WindowWindowClass.INPUT_OUTPUT
        attr.width = allocation.width
        attr.height = allocation.height
        attr.x = allocation.x
        attr.y = allocation.y
        attr.visual = self.get_visual()
        attr.event_mask = self.get_events() | Gdk.EventMask.EXPOSURE_MASK | Gdk.EventMask.BUTTON_PRESS_MASK
        wat = Gdk.WindowAttributesType
        mask = wat.X | wat.Y | wat.VISUAL
        window = Gdk.Window(self.get_parent_window(), attr, mask)
        # Associate the gdk.Window with ourselves,
        # Gtk+ needs a reference between the widget and the gdk window
        window.set_user_data(self)

        display = Gdk.Display.get_default()
        cursor = Gdk.Cursor.new_for_display(display, Gdk.CursorType.HAND2)
        window.set_cursor(cursor)

        self.set_window(window)

    def draw_text_bubble(self, cr, pointx, pointy):
        corner_radius = 9.0
        margin_top = 12.0
        margin_bottom = 12.0
        margin_left = 24.0
        margin_right = 24.0

        if len(self._bubble_text) <= 0:
            return

        alloc = self.get_allocation()

        layout = PangoCairo.create_layout(cr)

        font_description = Pango.font_description_from_string(BUBBLE_TEXT_FONT)
        layout.set_font_description(font_description)

        layout.set_alignment(Pango.Alignment.CENTER)
        layout.set_spacing(3)
        layout.set_markup(self._bubble_text)
        (ink_rect, logical_rect) = layout.get_pixel_extents()

        # Calculate the bubble size based on the text layout size
        width = logical_rect.width + margin_left + margin_right
        height = logical_rect.height + margin_top + margin_bottom

        if pointx < alloc.width / 2:
            x = pointx + 20
        else:
            x = pointx - width - 20

        y = pointy - height / 2

        # Make sure it fits in the visible area
        x = self.clamp(x, 0, alloc.width - width)
        y = self.clamp(y, 0, alloc.height - height)

        cr.save()
        cr.translate(x, y)

        # Draw the bubble
        cr.new_sub_path()
        cr.arc(width - corner_radius, corner_radius, corner_radius, math.radians(-90), math.radians(0))
        cr.arc(width - corner_radius, height - corner_radius, corner_radius, math.radians(0), math.radians(90))
        cr.arc(corner_radius, height - corner_radius, corner_radius, math.radians(90), math.radians(180))
        cr.arc(corner_radius, corner_radius, corner_radius, math.radians(180), math.radians(270))

        cr.close_path()

        cr.set_source_rgba(0.2, 0.2, 0.2, 0.7)
        cr.fill()

        # And finally draw the text
        cr.set_source_rgb(1, 1, 1)
        cr.move_to(margin_left, margin_top)
        PangoCairo.show_layout(cr, layout)
        cr.restore()

    def do_draw(self, cr):
        alloc = self.get_allocation()

        # Paint background
        if self._background is not None:
            Gdk.cairo_set_source_pixbuf(cr, self._background, 0, 0)
            cr.paint()

        if not self._show_offset:
            return

        # Paint highlight
        offset = self._selected_offset

        if self.is_sensitive():
            filename = "timezone_%g.png" % offset
        else:
            filename = "timezone_%g_dim.png" % offset

        path = os.path.join(TIMEZONEMAP_IMAGES_PATH, filename)
        try:
            orig_highlight = GdkPixbuf.Pixbuf.new_from_file(path)
        except Exception as err:
            print("Can't load {0} image file".format(path))
            print(err)
            return

        highlight = orig_highlight.scale_simple(
            alloc.width,
            alloc.height,
            GdkPixbuf.InterpType.BILINEAR)

        Gdk.cairo_set_source_pixbuf(cr, highlight, 0, 0)
        cr.paint()

        del highlight
        del orig_highlight

        if self._tz_location:
            longitude = self._tz_location.get_property('longitude')
            latitude = self._tz_location.get_property('latitude')

            pointx = self.convert_longitude_to_x(longitude, alloc.width)
            pointy = self.convert_latitude_to_y(latitude, alloc.height)

            # pointx = self.clamp(math.floor(pointx), 0, alloc.width)
            # pointy = self.clamp(math.floor(pointy), 0, alloc.height)

            if pointy > alloc.height:
                pointy = alloc.height

            # Draw text bubble
            self.draw_text_bubble(cr, pointx, pointy)

            # Draw pin
            if self._pin is not None:
                Gdk.cairo_set_source_pixbuf(
                    cr,
                    self._pin,
                    pointx - PIN_HOT_POINT_X,
                    pointy - PIN_HOT_POINT_Y)
                cr.paint()

    def set_location(self, tz_location):
        self._tz_location = tz_location

        if tz_location is not None:
            info = self._tz_location.get_info()

            daylight_offset = 0

            if self._tz_location.is_dst():
                if info.get_daylight() == 1:
                    daylight_offset = -1.0

            self._selected_offset = tz_location.get_utc_offset().total_seconds() / (60.0 * 60.0) + daylight_offset

            self.emit("location-changed", self._tz_location)

            self._show_offset = True
        else:
            self._show_offset = False
            self._selected_offset = 0.0

    def get_loc_for_xy(self, x, y):
        rowstride = self._color_map.get_rowstride()
        pixels = self._color_map.get_pixels()

        my_red = pixels[int(rowstride * y + x * 4)]
        my_green = pixels[int(rowstride * y + x * 4) + 1]
        my_blue = pixels[int(rowstride * y + x * 4) + 2]
        my_alpha = pixels[int(rowstride * y + x * 4) + 3]

        for color_code in color_codes:
            (offset, red, green, blue, alpha) = color_code
            if red == my_red and green == my_green and blue == my_blue and alpha == my_alpha:
                self._selected_offset = offset
                break

        self.queue_draw()

        # Work out the co-ordinates
        allocation = self.get_allocation()
        width = allocation.width
        height = allocation.height

        nearest_tz_location = None

        # Impossible distance
        small_dist = -1

        for tz_location in self.tzdb.get_locations():
            longitude = tz_location.get_property('longitude')
            latitude = tz_location.get_property('latitude')

            pointx = self.convert_longitude_to_x(longitude, width)
            pointy = self.convert_latitude_to_y(latitude, height)

            dx = pointx - x
            dy = pointy - y

            dist = dx * dx + dy * dy

            if small_dist == -1 or dist < small_dist:
                nearest_tz_location = tz_location
                small_dist = dist

        return nearest_tz_location

    def do_button_press_event(self, event):
        """ The button press event virtual method """

        # Make sure it was the first button
        if event.button == 1:
            x = int(event.x)
            y = int(event.y)

            nearest_tz_location = self.get_loc_for_xy(x, y)

            if nearest_tz_location is not None:
                self.set_bubble_text(nearest_tz_location)
                self.set_location(nearest_tz_location)
                self.queue_draw()
        return True

    def set_timezone(self, time_zone):
        real_tz = self.tzdb.get_loc(time_zone)

        ret = False

        if real_tz is not None:
            tz_to_compare = real_tz

            for tz_location in self.tzdb.get_locations():
                if tz_location.get_property('zone') == tz_to_compare.get_property('zone'):
                    self.set_bubble_text(tz_location)
                    self.set_location(tz_location)
                    self.queue_draw()
                    ret = True
                    break

        return ret

    def set_bubble_text(self, location):
        tzinfo = location.get_info()
        dt_now = datetime.now(tzinfo)
        current_time = "%02d:%02d" % (dt_now.hour, dt_now.minute)
        city_name = location.get_info().tzname("").split("/")[1]
        city_name = city_name.replace("_", " ")
        country_name = location.get_property('human_country')
        self._bubble_text = "{0}, {1}\n{2}".format(city_name, country_name, current_time)
        self.queue_draw()

    def get_location(self):
        return self._tz_location

    def get_timezone_at_coords(self, latitude, longitude):
        x = int(2048.0 / 360.0 * (180.0 + longitude))
        y = int(1024.0 / 180.0 * (90.0 - latitude))

        olsen_map_channels = self._olsen_map.get_n_channels()
        olsen_map_rowstride = self._olsen_map.get_rowstride()
        olsen_map_pixels = self._olsen_map.get_pixels()

        zone = -1
        offset = olsen_map_rowstride * y + x * olsen_map_channels

        if offset < len(olsen_map_pixels):
            color0 = olsen_map_pixels[offset]
            color1 = olsen_map_pixels[offset + 1]
            zone = ((color0 & 248) << 1) + ((color1 >> 4) & 15)

        if 0 <= zone < len(self.olsen_map_timezones):
            city = self.olsen_map_timezones[zone]
            return city
        else:
            alloc = self.get_allocation()
            x = self.convert_longitude_to_x(longitude, alloc.width)
            y = self.convert_latitude_to_y(latitude, alloc.height)
            location = self.get_loc_for_xy(x, y)
            zone = location.get_property('zone')
            return zone

    @staticmethod
    def convert_longitude_to_x(longitude, map_width):
        xdeg_offset = -6.0
        return (map_width * (180.0 + longitude) / 360.0) + (map_width * xdeg_offset / 180.0)

    @staticmethod
    def convert_latitude_to_y(latitude, map_height):
        bottom_lat = -59.0
        top_lat = 81.0

        top_per = top_lat / 180.0

        y = 1.25 * math.log(math.tan(G_PI_4 + 0.4 * math.radians(latitude)))

        full_range = 4.6068250867599998
        top_offset = full_range * top_per
        map_range = math.fabs(1.25 * math.log(math.tan(G_PI_4 + 0.4 * math.radians(bottom_lat))) - top_offset)
        y = math.fabs(y - top_offset)
        y /= map_range
        y = y * map_height
        return y

    @staticmethod
    def clamp(x, min_value, max_value):
        if x < min_value:
            x = min_value
        elif x > max_value:
            x = max_value
        return x

if __name__ == '__main__':
    win = Gtk.Window()
    tzmap = TimezoneMap()
    win.add(tzmap)
    win.show_all()

    # Test with Europe/London
    # timezone = tzmap.get_timezone_at_coords(latitude=51.3030, longitude=-0.00731)

    # Test with America/Montreal
    timezone = tzmap.get_timezone_at_coords(latitude=+45.31, longitude=-73.34)

    tzmap.set_timezone(timezone)

    import signal    # enable Ctrl-C since there is no menu to quit
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    Gtk.main()
