#!/usr/bin/env python3
# coding=utf-8
#
#   pycman.pkginfo - A Python implementation of Pacman
#
#   Copyright © 2011 Rémy Oudompheng <remy@archlinux.org>
#   Copyright © 2015 Apricity
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

"""
Package information formatting

This module defines utility function to format package information
for terminal output.
"""

import sys
import time
import textwrap

import struct
import fcntl
import termios

import pyalpm

ATTRNAME_FORMAT = '%-14s : '
ATTR_INDENT = 17 * ' '


def get_term_size():
    if sys.stdout.isatty():
        height, width = struct.unpack("HH", fcntl.ioctl(1, termios.TIOCGWINSZ, 4 * b"\x00"))
        return width
    else:
        return 80


def format_attr(attrname, value, attrformat=None):
    if isinstance(value, list):
        if len(value) == 0:
            valuestring = 'None'
        else:
            valuestring = '  '.join(str(v) for v in value)
    else:
        if attrformat == "time":
            valuestring = time.strftime("%a %d %b %Y %X %Z", time.localtime(value))
        else:
            valuestring = str(value)

    return textwrap.fill(valuestring, width=get_term_size(),
                         initial_indent=ATTRNAME_FORMAT % attrname,
                         subsequent_indent=ATTR_INDENT,
                         break_on_hyphens=False,
                         break_long_words=False)


def format_attr_oneperline(attrname, value):
    if len(value) == 0:
        value = ['None']
    s = ATTRNAME_FORMAT % attrname
    s += ('\n' + ATTR_INDENT).join(value)
    return s


def display_pkginfo(pkg, level=1, style='local'):
    """
    Displays pretty-printed package information.

    Args :
      pkg -- the package to display
      level -- the level of detail (1 or 2)
      style -- 'local' or 'sync'
    """

    if style not in ['local', 'sync', 'file']:
        raise ValueError('Invalid style for package info formatting')

    if style == 'sync':
        print(format_attr('Repository', pkg.db.name))

    print(format_attr('Name', pkg.name))
    print(format_attr('Version', pkg.version))
    print(format_attr('URL', pkg.url))
    print(format_attr('Licenses', pkg.licenses))
    print(format_attr('Groups', pkg.groups))
    print(format_attr('Provides', pkg.provides))
    print(format_attr('Depends On', pkg.depends))
    print(format_attr_oneperline('Optional Deps', pkg.optdepends))

    if style == 'local' or level == 2:
        print(format_attr('Required By', pkg.compute_requiredby()))

    print(format_attr('Conflicts With', pkg.conflicts))
    print(format_attr('Replaces', pkg.replaces))

    if style == 'sync':
        print(format_attr('Download Size', '%.2f K' % (pkg.size / 1024)))

    if style == 'file':
        print(format_attr('Compressed Size', '%.2f K' % (pkg.size / 1024)))

    print(format_attr('Installed Size', '%.2f K' % (pkg.isize / 1024)))
    print(format_attr('Packager', pkg.packager))
    print(format_attr('Architecture', pkg.arch))
    print(format_attr('Build Date', pkg.builddate, attrformat='time'))

    if style == 'local':
        # local installation information
        print(format_attr('Install Date', pkg.installdate, attrformat='time'))
        if pkg.reason == pyalpm.PKG_REASON_EXPLICIT:
            reason = 'Explicitly installed'
        elif pkg.reason == pyalpm.PKG_REASON_DEPEND:
            reason = 'Installed as a dependency for another package'
        else:
            reason = 'N/A'
        print(format_attr('Install Reason', reason))

    if style != 'sync':
        print(format_attr('Install Script', 'Yes' if pkg.has_scriptlet else 'No'))

    if style == 'sync':
        print(format_attr('MD5 Sum', pkg.md5sum))
        print(format_attr('SHA256 Sum', pkg.sha256sum))
        print(format_attr('Signatures', 'Yes' if pkg.base64_sig else 'No'))

    print(format_attr('Description', pkg.desc))

    if level >= 2 and style == 'local':
        # print backup information
        print('Backup files:')
        if len(pkg.backup) == 0:
            print('(none)')
        else:
            print('\n'.join(["%s %s" % (md5, filename) for (filename, md5) in pkg.backup]))
    print('')


def get_pkginfo(pkg, level=1, style='local'):
    if style not in ['local', 'sync', 'file']:
        raise ValueError('Invalid style for package info formatting')

    info = {}

    if style == 'sync':
        info['repository'] = pkg.db.name

    info['name'] = pkg.name
    info['version'] = pkg.version
    info['url'] = pkg.url
    info['licenses'] = pkg.licenses
    info['groups'] = pkg.groups
    info['provides'] = pkg.provides
    info['depends on'] = pkg.depends
    info['optional deps'] = pkg.optdepends

    if style == 'local' or level == 2:
        info['required by'] = pkg.compute_requiredby()

    info['conflicts with'] = pkg.conflicts
    info['replaces'] = pkg.replaces

    if style == 'sync':
        info['download size'] = pkg.size / 1024

    if style == 'file':
        info['compressed size'] = pkg.size / 1024

    info['installed size'] = pkg.isize / 1024
    info['packager'] = pkg.packager
    info['architecture'] = pkg.arch
    info['build date'] = pkg.builddate

    if style == 'local':
        # local installation information
        info['install date'] = pkg.installdate
        if pkg.reason == pyalpm.PKG_REASON_EXPLICIT:
            reason = _('Explicitly installed')
        elif pkg.reason == pyalpm.PKG_REASON_DEPEND:
            reason = _('Installed as a dependency for another package')
        else:
            reason = 'N/A'
        info['install reason'] = reason

    if style != 'sync':
        if pkg.has_scriptlet:
            info['install script'] = 'Yes'
        else:
            info['install script'] = 'No'

    if style == 'sync':
        info['md5 sum'] = pkg.md5sum
        info['sha256 sum'] = pkg.sha256sum
        if pkg.base64_sig:
            info['signatures'] = 'Yes'
        else:
            info['signatures'] = 'No'

    info['description'] = pkg.desc

    if level >= 2 and style == 'local':
        if len(pkg.backup) == 0:
            info['backup files'] = None
        else:
            info['backup files'] = [(md5, filename) for (filename, md5) in pkg.backup]

    return info
