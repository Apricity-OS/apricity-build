#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
#  Copyright (C) 2012 Canonical Ltd.
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


def utf8(s, errors="strict"):
    """Decode a string as UTF-8 if it isn't already Unicode."""
    if isinstance(s, str):
        return s
    else:
        return str(s, "utf-8", errors)


def get_languages(language_list="data/languagelist.data.gz", current_language_index=-1):
    """ Returns a tuple of (current language, sorted choices, display map). """
    import gzip
    # import icu

    current_language = "English"

    languagelist = gzip.open(language_list)
    language_display_map = {}
    i = 0
    for line in languagelist:
        line = utf8(line)
        if line == '' or line == '\n':
            continue
        code, name, trans = line.strip('\n').split(':')[1:]
        if code in ('C', 'dz', 'km'):
            i += 1
            continue
        # KDE fails to round-trip strings containing U+FEFF ZERO WIDTH
        # NO-BREAK SPACE, and we don't care about the NBSP anyway, so strip
        # it.
        #   https://bugs.launchpad.net/bugs/1001542
        #   (comment #5 and on)
        trans = trans.strip(" \ufeff")

        language_display_map[trans] = (name, code)
        if i == current_language_index:
            current_language = trans

        i += 1
    languagelist.close()

    # try:
    # Note that we always collate with the 'C' locale.  This is far
    # from ideal.  But proper collation always requires a specific
    # language for its collation rules (languages frequently have
    # custom sorting).  This at least gives us common sorting rules,
    # like stripping accents.
    # collator = icu.Collator.createInstance(icu.Locale('C'))
    # except:
    #       collator = None

    collator = None

    def compare_choice(x):
        if language_display_map[x][1] == 'C':
            return None  # place C first
        if collator:
            try:
                return collator.getCollationKey(x).getByteArray()
            except Exception:
                pass
        # Else sort by unicode code point, which isn't ideal either,
        # but also has the virtue of sorting like-glyphs together
        return x

    sorted_choices = sorted(language_display_map, key=compare_choice)

    return current_language, sorted_choices, language_display_map
