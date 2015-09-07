#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  keyboard_widget.py
#
#  Copyright 2013 Manjaro (QT version)
#  Copyright © 2015 Apricity (GTK version)
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

""" Keyboard widget that shows keyboard layout and variant types to the user """

from gi.repository import Gtk, GObject
import cairo
import subprocess
import math


def unicode_to_string(raw):
    """ U+ , or +U+ ... to string """
    if raw[0:2] == "U+":
        return chr(int(raw[2:], 16))
    elif raw[0:2] == "+U":
        return chr(int(raw[3:], 16))
    return ""


class KeyboardWidget(Gtk.DrawingArea):
    __gtype_name__ = 'KeyboardWidget'

    kb_104 = {
        "extended_return": False,
        "keys": [
            (0x29, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd),
            (0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x2b),
            (0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28),
            (0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35),
            ()]
    }

    kb_105 = {
        "extended_return": True,
        "keys": [
            (0x29, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd),
            (0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b),
            (0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x2b),
            (0x54, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35),
            ()]
    }

    kb_106 = {
        "extended_return": True,
        "keys": [
            (0x29, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe),
            (0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b),
            (0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29),
            (0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36),
            ()]
    }

    def __init__(self):
        Gtk.DrawingArea.__init__(self)

        self.set_size_request(460, 130)

        self.codes = []

        self.layout = "us"
        self.variant = ""
        self.font = "Helvetica"

        self.space = 6

        self.kb = None

    def set_layout(self, layout):
        self.layout = layout

    def set_font(self):
        """ Font depends on the keyboard layout """
        # broken: ad (Andorra), lk (Sri Lanka), brai (Braille)
        # ?!?: us:chr

        self.font = "Helvetica"

        # Load fonts from ttf-aboriginal-sans package

        # us:chr
        if self.variant == "chr":
            self.font = "Aboriginal Sans"

        # Load fonts from:
        # ttf-indic-otf, ttf-khmer, ttf-lohit-fonts, ttf-myanmar3
        # ttf-thaana-fonts, ttf-tlwg

        # Font: Akaash
        if self.layout == "bd":
            self.font = "Akaash"

        # Font: Gari
        if self.layout == "np" or self.layout == "in":
            self.font = "Gargi"

        # Font: KhmerOS
        if self.layout == "kh":
            self.font = "KhmerOS"

        # Font: Bengali
        if self.variant == "ben_probhat" or self.variant == "ben":
            self.font = "Lohit Bengali"

        # Font: Padmaa
        if self.variant == "guj":  # not all keys
            self.font = "Padmaa"

        # Font: Punjabi
        if self.variant == "guru" or self.variant == "jhelum":
            self.font = "Lohit Punjabi"

        # Font: Kannada
        if self.variant == "kan":
            self.font = "Lohit Kannada"

        # Font: Malayalam
        if self.variant == "mal" or self.variant == "mal_lalitha":
            self.font = "Malayalam"

        # Font: Tamil
        if self.variant == "tam_keyboard_with_numerals" or self.variant == "tam":
            self.font = "Lohit Tamil"

        # Font: TSCu Times
        lst = ["tam_TAB", "tam_TSCII", "tam_unicode"]
        for i in lst:
            if self.variant == i:
                self.font = "TSCu_Times"

        # Font: Telugu
        if self.variant == "tel":
            self.font = "Lohit Telugu"

        # Font: Oriya
        lst = ["af", "ara", "am", "cn", "ge", "gr", "gn", "ir", "iq", "ie", "il", "la", "ma", "pk", "lk", "sy"]
        for i in lst:
            if self.layout == i:
                self.font = "Oriya"

        lst = ["geo", "urd-phonetic3", "urd-phonetic", "urd-winkeys"]
        for i in lst:
            if self.variant == i:
                self.font = "Oriya"

        if self.variant == "ori":
            self.font = "Lohit Oriya"

        # Font: Mv Boli
        if self.layout == "mv":
            self.font = "MVBoli"

        # Font: Myanmar
        if self.layout == "mm":
            self.font = "Myanmar3"

        # Font: Tlwg
        if self.layout == "th":
            self.font = "Tlwg Mono"

    def set_variant(self, variant):
        self.variant = variant
        self.load_codes()
        self.load_info()
        self.set_font()
        # Force repaint
        self.queue_draw()

    def load_info(self):
        kbl_104 = ["us", "th"]
        kbl_106 = ["jp"]

        # Most keyboards are 105 key so default to that
        if self.layout in kbl_104:
            self.kb = self.kb_104
        elif self.layout in kbl_106:
            self.kb = self.kb_106
        elif self.kb != self.kb_105:
            self.kb = self.kb_105

    @staticmethod
    def rounded_rectangle(cr, x, y, width, height, aspect=1.0):
        corner_radius = height / 10.0
        radius = corner_radius / aspect
        degrees = math.pi / 180.0

        cr.new_sub_path()
        cr.arc(x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees)
        cr.arc(x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees)
        cr.arc(x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees)
        cr.arc(x + radius, y + radius, radius, 180 * degrees, 270 * degrees)
        cr.close_path()

        cr.set_source_rgb(0.5, 0.5, 0.5)
        cr.fill_preserve()
        cr.set_source_rgba(0.2, 0.2, 0.2, 0.5)
        cr.set_line_width(2)
        cr.stroke()

    def do_draw(self, cr):
        """ The 'cr' variable is the current Cairo context """
        # alloc = self.get_allocation()
        # real_width = alloc.width
        # real_height = alloc.height

        width = 460
        # height = 130

        usable_width = width - 6
        key_w = (usable_width - 14 * self.space) / 15

        # Set background color to transparent
        cr.set_source_rgba(1.0, 1.0, 1.0, 0.0)
        cr.paint()

        cr.set_source_rgb(0.84, 0.84, 0.84)
        cr.set_line_width(2)

        cr.rectangle(0, 0, 640, 640)
        cr.stroke()

        cr.set_source_rgb(0.22, 0.22, 0.22)

        rx = 3

        space = self.space
        w = usable_width
        kw = key_w

        # Use this to show real widget size (useful when debugging this widget)
        # cr.rectangle(0, 0, real_width, real_height)

        def draw_row(row, sx, sy, last_end=False):
            x = sx
            y = sy
            keys = row
            rw = w - sx
            i = 0
            for k in keys:
                rect = (x, y, kw, kw)

                if i == len(keys) - 1 and last_end:
                    rect = (rect[0], rect[1], rw, rect[3])

                self.rounded_rectangle(cr, rect[0], rect[1], rect[2], rect[3])

                px = rect[0] + 5
                py = rect[1] + rect[3] - (rect[3] / 4)

                if len(self.codes) > 0:
                    # Draw lower character
                    cr.set_source_rgb(1.0, 1.0, 1.0)
                    cr.select_font_face(self.font, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_BOLD)
                    cr.set_font_size(10)
                    cr.move_to(px, py)
                    cr.show_text(self.regular_text(k))

                    px = rect[0] + 5
                    py = rect[1] + (rect[3] / 3)

                    # Draw upper character
                    cr.set_source_rgb(0.82, 0.82, 0.82)
                    cr.select_font_face(self.font, cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
                    cr.set_font_size(8)
                    cr.move_to(px, py)
                    cr.show_text(self.shift_text(k))

                rw = rw - space - kw
                x = x + space + kw
                i += 1
            return x, rw

        x = 6
        y = 6

        keys = self.kb["keys"]
        ext_return = self.kb["extended_return"]

        first_key_w = 0

        rows = 4
        remaining_x = [0, 0, 0, 0]
        remaining_widths = [0, 0, 0, 0]

        for i in range(0, rows):
            if first_key_w > 0:
                first_key_w *= 1.375

                if self.kb == self.kb_105 and i == 3:
                    first_key_w = kw * 1.275

                self.rounded_rectangle(cr, 6, y, first_key_w, kw)
                x = 6 + first_key_w + space
            else:
                first_key_w = kw

            x, rw = draw_row(keys[i], x, y, i == 1 and not ext_return)

            remaining_x[i] = x
            remaining_widths[i] = rw

            if i != 1 and i != 2:
                self.rounded_rectangle(cr, x, y, rw, kw)

            x = .5
            y = y + space + kw

        if ext_return:
            # rx = rx * 2
            x1 = remaining_x[1]
            y1 = 6 + kw * 1 + space * 1
            w1 = remaining_widths[1]
            x2 = remaining_x[2]
            y2 = 6 + kw * 2 + space * 2

            # this is some serious crap... but it has to be so
            # maybe one day keyboards won't look like this...
            # one can only hope
            degrees = math.pi / 180.0

            cr.new_sub_path()

            cr.move_to(x1, y1 + rx)
            cr.arc(x1 + rx, y1 + rx, rx, 180 * degrees, -90 * degrees)
            cr.line_to(x1 + w1 - rx, y1)
            cr.arc(x1 + w1 - rx, y1 + rx, rx, -90 * degrees, 0)
            cr.line_to(x1 + w1, y2 + kw - rx)
            cr.arc(x1 + w1 - rx, y2 + kw - rx, rx, 0 * degrees, 90 * degrees)
            cr.line_to(x2 + rx, y2 + kw)
            cr.arc(x2 + rx, y2 + kw - rx, rx, 90 * degrees, 180 * degrees)
            cr.line_to(x2, y1 + kw)
            cr.line_to(x1 + rx, y1 + kw)
            cr.arc(x1 + rx, y1 + kw - rx, rx, 90 * degrees, 180 * degrees)

            cr.close_path()

            cr.set_source_rgb(0.5, 0.5, 0.5)
            cr.fill_preserve()
            cr.set_source_rgba(0.2, 0.2, 0.2, 0.5)
            cr.set_line_width(2)
            cr.stroke()
        else:
            x = remaining_x[2]
            # Changed .5 to 6 because return key was out of line
            y = 6 + kw * 2 + space * 2
            self.rounded_rectangle(cr, x, y, remaining_widths[2], kw)

    def regular_text(self, index):
        try:
            return self.codes[index - 1][0]
        except IndexError:
            return " "

    def shift_text(self, index):
        try:
            return self.codes[index - 1][1]
        except IndexError:
            return " "

    def ctrl_text(self, index):
        try:
            return self.codes[index - 1][2]
        except IndexError:
            return " "

    def alt_text(self, index):
        try:
            return self.codes[index - 1][3]
        except IndexError:
            return " "

    def load_codes(self):
        if self.layout is None:
            return

        variant_param = ""
        if self.variant:
            variant_param = "-variant {0}".format(self.variant)

        cmd = "/usr/share/cnchi/scripts/ckbcomp -model pc106 -layout {0} {1} -compact".format(self.layout,
                                                                                              variant_param)

        pipe = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=None)
        cfile = pipe.communicate()[0].decode("utf-8").split('\n')

        # Clear current codes
        del self.codes[:]

        for line in cfile:
            if line[:7] != "keycode":
                continue

            codes = line.split('=')[1].strip().split(' ')

            plain = unicode_to_string(codes[0])
            shift = unicode_to_string(codes[1])
            ctrl = unicode_to_string(codes[2])
            alt = unicode_to_string(codes[3])

            if ctrl == plain:
                ctrl = ""

            if alt == plain:
                alt = ""

            self.codes.append((plain, shift, ctrl, alt))


GObject.type_register(KeyboardWidget)
