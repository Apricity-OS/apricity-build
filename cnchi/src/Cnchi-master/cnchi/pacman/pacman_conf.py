#!/usr/bin/env python3
#
#  pacman_conf.py
#
#  Based on pyalpm code Copyright (C) 2011 Rémy Oudompheng <remy@archlinux.org>
#  Copyright © 2015 Apricity
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

""" This module handles pacman.conf files """

import os
import glob
import collections
import warnings


class InvalidSyntax(Warning):
    """ Class to show warning when a pacman.conf parse error is issued """
    def __init__(self, filename, problem, arg):
        self.filename = filename
        self.problem = problem
        self.arg = arg

    def __str__(self):
        return "unable to parse {0}, {1}: {2}".format(self.filename, self.problem, self.arg)

# Options that may occur several times in a section. Their values should be accumulated in a list.
LIST_OPTIONS = (
    'CacheDir',
    'HoldPkg',
    'SyncFirst',
    'IgnoreGroup',
    'IgnorePkg',
    'NoExtract',
    'NoUpgrade',
    'Server'
)

SINGLE_OPTIONS = (
    'RootDir',
    'DBPath',
    'GPGDir',
    'LogFile',
    'Architecture',
    'XferCommand',
    'CleanMethod',
    'SigLevel',
    'LocalFileSigLevel',
    'RemoteFileSigLevel',
    'UseDelta',
)

BOOLEAN_OPTIONS = (
    'UseSyslog',
    'ShowSize',
    'UseDelta',
    'TotalDownload',
    'CheckSpace',
    'VerbosePkgLists',
    'ILoveCandy',
    'Color'
)


def pacman_conf_enumerator(path):
    filestack = []
    current_section = None
    filestack.append(open(path))
    while len(filestack) > 0:
        f = filestack[-1]
        line = f.readline()
        if len(line) == 0:
            # end of file
            f.close()
            filestack.pop()
            continue

        line = line.strip()
        if len(line) == 0:
            continue
        if line[0] == '#':
            continue
        if line[0] == '[' and line[-1] == ']':
            current_section = line[1:-1]
            continue
        if current_section is None:
            raise InvalidSyntax(f.name, 'statement outside of a section', line)
        # read key, value
        key, equal, value = [x.strip() for x in line.partition('=')]

        # include files
        if equal == '=' and key == 'Include':
            filestack.extend(open(f) for f in glob.glob(value))
            continue
        if current_section != 'options':
            # repos only have the Server, SigLevel, Usage options
            if key in ('Server', 'SigLevel', 'Usage') and equal == '=':
                yield (current_section, key, value)
            else:
                raise InvalidSyntax(f.name, 'invalid key for repository configuration', line)
            continue
        if equal == '=':
            if key in LIST_OPTIONS:
                for val in value.split():
                    yield (current_section, key, val)
            elif key in SINGLE_OPTIONS:
                yield (current_section, key, value)
            else:
                warnings.warn(InvalidSyntax(f.name, 'unrecognized option', key))
        else:
            if key in BOOLEAN_OPTIONS:
                yield (current_section, key, True)
            else:
                warnings.warn(InvalidSyntax(f.name, 'unrecognized option', key))


class PacmanConfig(collections.OrderedDict):
    def __init__(self, conf=None, options=None):
        super(PacmanConfig, self).__init__()
        self['options'] = collections.OrderedDict()
        self.options = self['options']
        self.repos = collections.OrderedDict()
        self.options["RootDir"] = "/"
        self.options["DBPath"] = "/var/lib/pacman"
        self.options["GPGDir"] = "/etc/pacman.d/gnupg/"
        self.options["LogFile"] = "/var/log/pacman.log"
        self.options["Architecture"] = os.uname()[-1]
        if conf is not None:
            self.load_from_file(conf)
        if options is not None:
            self.load_from_options(options)

    def load_from_file(self, filename):
        for section, key, value in pacman_conf_enumerator(filename):
            if section == 'options':
                if key == 'Architecture' and value == 'auto':
                    continue
                if key in LIST_OPTIONS:
                    self.options.setdefault(key, []).append(value)
                else:
                    self.options[key] = value
            else:
                servers = self.repos.setdefault(section, [])
                if key == 'Server':
                    servers.append(value)
        if "CacheDir" not in self.options:
            self.options["CacheDir"] = ["/var/cache/pacman/pkg"]

    def load_from_options(self, options):
        global _logmask
        if options.root is not None:
            self.options["RootDir"] = options.root
        if options.dbpath is not None:
            self.options["DBPath"] = options.dbpath
        if options.gpgdir is not None:
            self.options["GPGDir"] = options.gpgdir
        if options.arch is not None:
            self.options["Architecture"] = options.arch
        if options.logfile is not None:
            self.options["LogFile"] = options.logfile
        if options.cachedir is not None:
            self.options["CacheDir"] = [options.cachedir]
        if options.debug:
            _logmask = 0xffff

    def apply(self, handle):
        # File paths
        handle.logfile = self.options["LogFile"]
        handle.gpgdir = self.options["GPGDir"]
        # Strings
        handle.arch = self.options["Architecture"]
        # Lists
        handle.cachedirs = self.options["CacheDir"]
        if "NoUpgrade" in self.options:
            handle.noupgrades = self.options["NoUpgrade"]
        if "NoExtract" in self.options:
            handle.noextracts = self.options["NoExtract"]
        if "IgnorePkg" in self.options:
            handle.ignorepkgs = self.options["IgnorePkg"]
        if "IgnoreGroup" in self.options:
            handle.ignoregrps = self.options["IgnoreGroup"]

        # h.logcb = cb_log

        # set sync databases
        for repo, servers in self.repos.items():
            db = handle.register_syncdb(repo, 0)
            db_servers = []
            for raw_url in servers:
                url = raw_url.replace("$repo", repo)
                url = url.replace("$arch", self.options["Architecture"])
                db_servers.append(url)
            db.servers = db_servers

    def __str__(self):
        conf = ''
        for section, options in self.items():
            conf = '{0}[{1}]\n'.format(conf, section)
            for key, value in options.items():
                if key in LIST_OPTIONS:
                    for v in value:
                        conf = '{0}{1} = {2}\n'.format(conf, key, v)
                elif key in BOOLEAN_OPTIONS:
                    conf = '{0}{1}\n'.format(conf, key)
                else:
                    conf = '{0}{1} = {2}\n'.format(conf, key, value)
            conf += '\n'
        return conf
