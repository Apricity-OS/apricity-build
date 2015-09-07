#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
#  Copyright (c) 2012 Canonical Ltd.
#  Copyright © 2015 Apricity
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

import cairo
from gi.repository import Gtk, GLib, Gdk, GObject, Pango

try:
    import misc.misc as misc
except ImportError:
    import misc


def refresh():
    while Gtk.events_pending():
        Gtk.main_iteration()


def draw_round_rect(c, r, x, y, w, h):
    c.move_to(x + r, y)
    c.line_to(x + w - r, y)
    c.curve_to(x + w, y, x + w, y, x + w, y + r)
    c.line_to(x + w, y + h - r)
    c.curve_to(x + w, y + h, x + w, y + h, x + w - r, y + h)
    c.line_to(x + r, y + h)
    c.curve_to(x, y + h, x, y + h, x, y + h - r)
    c.line_to(x, y + r)
    c.curve_to(x, y, x, y, x + r, y)
    c.close_path()


def gtk_to_cairo_color(c):
    color = Gdk.RGBA()
    color.parse(c)
    return color.red, color.green, color.blue


class StylizedFrame(Gtk.Bin):
    __gtype_name__ = 'StylizedFrame'
    __gproperties__ = {
        'radius': (
            GObject.TYPE_INT, 'Radius', 'The radius of the rounded corners.',
            0, GLib.MAXINT, 10, GObject.ParamFlags.READWRITE),
        'width': (
            GObject.TYPE_INT, 'Width', 'The width of the outline.',
            0, GLib.MAXINT, 1, GObject.ParamFlags.READWRITE),
    }

    def __init__(self):
        # Gtk.Alignment.__init__(self)
        Gtk.Bin.__init__(self)
        self.radius = 10
        self.width = 1

    def do_get_property(self, prop):
        if prop.name in ('radius', 'width'):
            return getattr(self, prop.name)
        else:
            # return Gtk.Alignment.do_get_property(self, prop)
            return Gtk.Bin.do_get_property(self, prop)

    def do_set_property(self, prop, value):
        if prop.name in ('radius', 'width'):
            setattr(self, prop.name, value)
            self.queue_draw()
        else:
            # Gtk.Alignment.do_set_property(self, prop, value)
            Gtk.Bin.do_set_property(self, prop, value)

    def paint_background(self, c):
        c.set_source_rgb(*gtk_to_cairo_color('#fbfbfb'))
        alloc = self.get_allocation()
        draw_round_rect(c, self.radius,
                        self.width / 2, self.width / 2,
                        alloc.width - self.width,
                        alloc.height - self.width)
        c.fill_preserve()

    def do_draw(self, c):
        # Background
        self.paint_background(c)
        # Edge
        c.set_source_rgb(*gtk_to_cairo_color('#c7c7c6'))
        c.set_line_width(self.width)
        c.stroke()
        if self.get_child():
            left = self.get_margin_start()
            top = self.get_margin_top()
            c.translate(left, top)
            self.get_child().draw(c)

GObject.type_register(StylizedFrame)


class DiskBox(Gtk.Box):
    __gtype_name__ = 'DiskBox'

    def add(self, partition, size):
        Gtk.Box.add(self, partition, expand=False)
        partition.set_size_request(size, -1)

    def clear(self):
        self.forall(lambda x: self.remove(x))

GObject.type_register(DiskBox)


class PartitionBox(StylizedFrame):
    __gtype_name__ = 'PartitionBox'
    __gproperties__ = {
        'title': (
            GObject.TYPE_STRING, 'Title', None, 'Title', GObject.ParamFlags.READWRITE),
        'icon-name': (
            GObject.TYPE_STRING, 'Icon Name', None, 'distributor-logo', GObject.ParamFlags.READWRITE),
        'icon-file': (
            GObject.TYPE_STRING, 'Icon File', None, 'distributor-logo', GObject.ParamFlags.READWRITE),
        'extra': (
            GObject.TYPE_STRING, 'Extra Text', None, '', GObject.ParamFlags.READWRITE),
    }

    def do_get_property(self, prop):
        if prop.name == 'title':
            return self.ostitle.get_text()
        elif prop.name == 'icon-name':
            return self.logo.get_icon_name()
        elif prop.name == 'icon-file':
            return self.icon_file
        elif prop.name == 'extra':
            return self.extra.get_text()
        return getattr(self, prop.name)

    def do_set_property(self, prop, value):
        if prop.name == 'title':
            self.ostitle.set_markup('<b>{0}</b>'.format(value))
        elif prop.name == 'icon-name':
            self.logo.set_from_icon_name(value, Gtk.IconSize.DIALOG)
        elif prop.name == 'icon-file':
            self.icon_file = value
            self.logo.set_from_file(value)
        elif prop.name == 'extra':
            self.extra.set_markup('<small>{0}</small>'.format(value and value or ' '))
        else:
            setattr(self, prop.name, value)

    def __init__(self, title="", extra="", icon_name="", icon_file=""):
        StylizedFrame.__init__(self)
        vbox = Gtk.Box()
        vbox.set_orientation(Gtk.Orientation.VERTICAL)

        if len(icon_file) > 0:
            self.logo = Gtk.Image.new_from_file(icon_file)
        else:
            self.logo = Gtk.Image.new_from_icon_name(icon_name, Gtk.IconSize.DIALOG)

        self.icon_file = icon_file

        self.logo.set_halign(Gtk.Align.CENTER)
        vbox.pack_start(self.logo, False, True, 0)

        self.ostitle = Gtk.Label()
        self.ostitle.set_ellipsize(Pango.EllipsizeMode.END)
        vbox.pack_start(self.ostitle, False, True, 0)

        self.extra = Gtk.Label()
        self.extra.set_ellipsize(Pango.EllipsizeMode.END)
        self.extra.set_halign(Gtk.Align.CENTER)
        vbox.pack_start(self.extra, False, True, 0)

        self.size = Gtk.Label()
        self.size.set_ellipsize(Pango.EllipsizeMode.END)
        self.size.set_halign(Gtk.Align.CENTER)
        vbox.pack_start(self.size, False, True, 0)
        self.add(vbox)

        self.ostitle.set_markup('<b>{0}</b>'.format(title))

        # Take up the space that would otherwise be used to create symmetry.
        self.extra.set_markup('<small>{0}</small>'.format(extra and extra or ' '))
        self.show_all()

    def set_size_in_mb(self, size):
        self.set_size(size * 1000.0)

    def set_size(self, size):
        size = misc.format_size(size)
        self.size.set_markup('<span size="x-large">{0}</span>'.format(size))

    @staticmethod
    def render_dots():
        # FIXME: Dots are rendered over the frame.
        s = cairo.ImageSurface(cairo.FORMAT_ARGB32, 2, 2)
        cr = cairo.Context(s)
        cr.set_source_rgb(*gtk_to_cairo_color('#b6b0a9'))
        cr.rectangle(1, 1, 1, 1)
        cr.fill()
        pattern = cairo.SurfacePattern(s)
        return pattern

    def paint_background(self, c):
        StylizedFrame.paint_background(self, c)
        a = self.get_allocation()
        pattern = self.render_dots()
        pattern.set_extend(cairo.EXTEND_REPEAT)
        c.set_source(pattern)
        c.fill_preserve()

        g = cairo.RadialGradient(a.width / 2, a.height / 2, 0, a.width / 2,
                                 a.height / 2,
                                 a.width > a.height and a.width or a.height)
        g.add_color_stop_rgba(0.00, 1, 1, 1, 1.00)
        g.add_color_stop_rgba(0.25, 1, 1, 1, 0.75)
        g.add_color_stop_rgba(0.40, 1, 1, 1, 0.00)
        c.set_source(g)
        c.fill_preserve()

GObject.type_register(PartitionBox)


class ResizeWidget(Gtk.Frame):
    __gtype_name__ = 'ResizeWidget'
    __gproperties__ = {
        'part-size': (
            GObject.TYPE_UINT64, 'Partition size',
            'The size of the partition being resized',
            1, GLib.MAXUINT64, 100, GObject.ParamFlags.READWRITE),
        'min-size': (
            GObject.TYPE_UINT64, 'Minimum size',
            'The minimum size that the existing partition can be resized to',
            0, GLib.MAXUINT64, 0, GObject.ParamFlags.READWRITE),
        'max-size': (
            GObject.TYPE_UINT64, 'Maximum size',
            'The maximum size that the existing partition can be resized to',
            1, GLib.MAXUINT64, 100, GObject.ParamFlags.READWRITE)
    }

    def do_get_property(self, prop):
        if prop.name in ('part-size', 'min-size', 'max-size'):
            name = prop.name.replace('-', '_')
            return getattr(self, name)
        else:
            return Gtk.Alignment.do_get_property(self, prop)

    def do_set_property(self, prop, value):
        if prop.name in ('part-size', 'min-size', 'max-size'):
            name = prop.name.replace('-', '_')
            setattr(self, name, value)
            self.queue_draw()
        else:
            # print(prop.name, value)
            Gtk.Alignment.do_set_property(self, prop, value)

    def __init__(self, part_size, min_size, max_size):
        """
            part_size: The size (MB) of the existing partition.
            min_size: The min size (MB) that the existing partition can be resized to.
            max_size: The max size (MB) that the existing partition can be resized to.
        """

        assert min_size <= max_size <= part_size
        assert part_size > 0

        self.set_size_request_done = False

        self.part_size = part_size
        self.min_size = min_size
        self.max_size = max_size

        Gtk.Frame.__init__(self)

        self.set_size_request(600, -1)

        self.set_shadow_type(Gtk.ShadowType.NONE)

        self.paned = Gtk.Paned.new(Gtk.Orientation.HORIZONTAL)

        self.existing_part = PartitionBox()
        self.paned.pack1(self.existing_part, resize=True, shrink=False)

        self.new_part = PartitionBox()
        self.paned.pack2(self.new_part, resize=True, shrink=False)

        self.add(self.paned)

    def set_part_title(self, part, title, subtitle=None):
        if part == 'new':
            self.new_part.set_property('title', title)
            if subtitle:
                self.new_part.set_property('extra', subtitle)
        else:
            self.existing_part.set_property('title', title)
            if subtitle:
                self.existing_part.set_property('extra', subtitle)

    def get_part_title_and_subtitle(self, part):
        if part == 'new':
            title = self.new_part.get_property('title')
            subtitle = self.new_part.get_property('extra')
        else:
            title = self.new_part.get_property('title')
            subtitle = self.new_part.get_property('extra')
        return title, subtitle

    def set_part_icon(self, part, icon_name=None, icon_file=None):
        if icon_name:
            if part == 'new':
                self.new_part.set_property('icon-name', icon_name)
            else:
                self.existing_part.set_property('icon-name', icon_name)
        elif icon_file:
            if part == 'new':
                self.new_part.set_property('icon-file', icon_file)
            else:
                self.existing_part.set_property('icon-file', icon_file)

    def do_size_allocate(self, allocation):
        Gtk.Frame.do_size_allocate(self, allocation)

        self.set_allocation(allocation)

        if not self.set_size_request_done:
            s1 = self.existing_part.get_allocation().width
            s2 = self.new_part.get_allocation().width
            total_width = s1 + s2

            tmp = float(self.min_size) / self.part_size
            pixels = int(tmp * total_width)
            self.existing_part.set_size_request(pixels, -1)

            tmp = ((float(self.part_size) - self.max_size) / self.part_size)
            pixels = int(tmp * total_width)
            self.new_part.set_size_request(pixels, -1)

            self.set_size_request_done = True

    def do_draw(self, cr):
        Gtk.Frame.do_draw(self, cr)

        s1 = self.existing_part.get_allocation().width
        s2 = self.new_part.get_allocation().width
        total_width = s1 + s2

        percent = (float(s1) / float(total_width))
        self.existing_part.set_size_in_mb(percent * self.part_size)

        percent = (float(s2) / float(total_width))
        self.new_part.set_size_in_mb(percent * self.part_size)

    def set_pref_size(self, size):
        s1 = self.existing_part.get_allocation().width
        s2 = self.new_part.get_allocation().width
        total_width = s1 + s2

        percent = (float(size) / float(self.part_size))
        val = percent * total_width
        self.paned.set_position(int(val))

    def get_size(self):
        """Returns the size of the old partition,
           clipped to the minimum and maximum sizes."""

        s1 = self.existing_part.get_allocation().width
        s2 = self.new_part.get_allocation().width
        total_width = s1 + s2

        size = int(float(s1) * self.part_size / float(total_width))
        if size < self.min_size:
            return self.min_size
        elif size > self.max_size:
            return self.max_size
        else:
            return size

GObject.type_register(ResizeWidget)


class StateBox(StylizedFrame):
    __gtype_name__ = 'StateBox'
    __gproperties__ = {
        'label': (GObject.TYPE_STRING, 'Label', None, 'label', GObject.ParamFlags.READWRITE),
    }

    def do_get_property(self, prop):
        if prop.name == 'label':
            return self.label.get_text()
        return getattr(self, prop.name)

    def do_set_property(self, prop, value):
        if prop.name == 'label':
            self.label.set_text(value)
            return
        setattr(self, prop.name, value)

    def __init__(self, text=''):
        StylizedFrame.__init__(self)
        hbox = Gtk.Box()
        hbox.set_spacing(10)
        self.image = Gtk.Image()
        self.image.set_from_icon_name("yes", Gtk.IconSize.LARGE_TOOLBAR)
        self.image.set_margin_start(7)
        self.label = Gtk.Label(label=text)
        self.label.set_margin_end(15)
        self.label.set_margin_top(15)
        self.label.set_margin_bottom(15)
        self.label.set_halign(Gtk.Align.START)

        hbox.pack_start(self.image, False, True, 0)
        hbox.pack_start(self.label, True, True, 0)

        self.add(hbox)
        self.show_all()

        self.status = True

    def set_state(self, state):
        self.status = state
        if state:
            self.image.set_from_icon_name(Gtk.STOCK_YES, Gtk.IconSize.LARGE_TOOLBAR)
        else:
            self.image.set_from_icon_name(Gtk.STOCK_NO, Gtk.IconSize.LARGE_TOOLBAR)

    def get_state(self):
        return self.status

    def show(self):
        super().show()

    def hide(self):
        super().hide()

GObject.type_register(StateBox)


class Builder(Gtk.Builder):
    """ GtkBuilder should have .get_object_ids() method """
    def __init__(self):
        self._widget_ids = set()
        super().__init__()

    def add_from_file(self, filename):
        import xml.etree.cElementTree as eTree
        tree = eTree.parse(filename)
        root = tree.getroot()
        for widgets in root.iter('object'):
            self._widget_ids.add(widgets.attrib['id'])
        return super().add_from_file(filename)

    def get_object_ids(self):
        return self._widget_ids
