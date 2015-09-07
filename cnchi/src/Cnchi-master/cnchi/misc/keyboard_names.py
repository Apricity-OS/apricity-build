#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
#  Copyright (C) 2012 Canonical Ltd.
#  Written by Colin Watson <cjwatson@ubuntu.com>.
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

"""Parse the output of kbdnames-maker."""

from collections import defaultdict
import gzip
import io

"""
lang*layout*layout_code*layout_name
lang*variant*layout_code*variant_code*variant_name

C*layout*th*Thailand
C*variant*th**Thailand
C*variant*th*pat*Thailand - Pattachote
C*variant*th*tis*Thailand - TIS-820.2538
"""


class KeyboardNames():
    def __init__(self, filename):        
        self._current_lang = None
        self._filename = filename
        self._clear()

    def _clear(self):
        self.layouts = {}
        self.variants = defaultdict(dict)
        self.models = {}

    def _load_file(self, lang, kbdnames):
        for line in kbdnames:
            line = line.rstrip("\n")
            got_lang, element, layout_code, value = line.split("*", 3)
                
            if got_lang != lang:
                continue

            if element == "layout":
                self.layouts[layout_code] = value
            elif element == "variant":
                variant_code, variant_name = value.split("*", 1)
                self.variants[layout_code][variant_code] = variant_name
            elif element == "model":
                self.models[layout_code] = value

    def load(self, lang):
        if lang == self._current_lang:
            # Already loaded
            return

        # Saving memory is more important than parsing time in the
        # relatively rare case of changing languages, so we only keep data
        # around for a single language.
        self._clear()

        raw = gzip.open(self._filename)
        try:
            with io.TextIOWrapper(raw, encoding='utf-8') as kbdnames:
                self._load_file(lang, kbdnames)
        finally:
            raw.close()
        self._current_lang = lang

    '''
    layout: layout_code : layout_name
    variant: variant_code : variant_name
    '''

    def get_layouts(self, lang):
        self.load(lang)
        return self.layouts

    def get_variants(self, lang, layout_code):
        self.load(lang)
        try:
            return self.variants[layout_code]
        except KeyError:
            return None

    def has_language(self, lang):
        self.load(lang)
        return bool(self.layouts)
    
    def has_variants(self, lang, layout_code):
        self.load(lang)
        return bool(self.variants[layout_code])

    def has_layout(self, lang, layout_code):
        self.load(lang)
        return layout_code in self.layouts
    
    def get_layout_code(self, lang, layout_name):
        """ Returns layout code from layout name """
        self.load(lang)
        for code in self.layouts:
            if self.layouts[code] == layout_name:
                return code
        return None
    
    def get_layout_name(self, lang, code):
        """ Returns layout name from layout code """
        self.load(lang)
        try:
            return self.layouts[code]
        except KeyError:
            return None
    
    def get_variant_code(self, lang, layout_code, variant_name):
        """ Returns variant code from layout and variant names """
        self.load(lang)
        for variant_code in self.variants[layout_code]:
            if self.variants[layout_code][variant_code] == variant_name:
                return variant_code
        return None
    
    def get_variant_name(self, lang, layout_code, variant_code):
        """ Returns variant description from variant code """
        self.load(lang)
        try:
            return self.variants[layout_code][variant_code]
        except KeyError:
            return None
            
