# -*- coding: utf-8; Mode: Python; indent-tabs-mode: nil; tab-width: 4 -*-

#  Copyright (C) 2007 Canonical Ltd.
#  Written by Colin Watson <cjwatson@ubuntu.com>.
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

#  Functions that are spiritually similar to ones in the os module, but
#  aren't there because not many people need chrooted operations like this.

import os


def _resolve_link_root(root, path):
    """Helper for realpath_root. See posixpath._resolve_link."""
    paths_seen = set()
    fullpath = os.path.join(root, path[1:])
    while os.path.islink(fullpath):
        if path in paths_seen:
            # Already seen this path, so we must have a symlink loop
            return None
        paths_seen.add(path)
        # Resolve where the link points to
        resolved = os.readlink(fullpath)
        if not os.path.isabs(resolved):
            dirname = os.path.dirname(path)
            path = os.path.normpath(os.path.join(dirname, resolved))
        else:
            path = os.path.normpath(resolved)
        fullpath = os.path.join(root, path[1:])
    return path


def _realpath_root_recurse(root, filename):
    """Helper for realpath_root."""
    bits = ['/'] + filename.split('/')[1:]

    for i in range(2, len(bits) + 1):
        component = os.path.join(*bits[0:i])
        fullcomponent = os.path.join(root, component[1:])
        # Resolve symbolic links.
        if os.path.islink(fullcomponent):
            resolved = _resolve_link_root(root, component)
            if resolved is None:
                # Infinite loop -- return original component + rest of
                # the path
                return os.path.abspath(os.path.join(component, *bits[i:]))
            else:
                newpath = os.path.join(resolved, *bits[i:])
                return _realpath_root_recurse(root, newpath)

    return os.path.abspath(filename)


def realpath_root(root, filename):
    """Like os.path.realpath, but resolved relative to root.
    filename must be absolute."""
    chrooted_filename = _realpath_root_recurse(root, filename)
    if chrooted_filename.startswith('/'):
        chrooted_filename = chrooted_filename[1:]
    return os.path.join(root, chrooted_filename)


def find_on_path_root(root, command):
    """Is command on the executable search path relative to root?"""
    if 'PATH' not in os.environ:
        return False
    path = os.environ['PATH']
    for element in path.split(os.pathsep):
        if not element:
            continue
        filename = realpath_root(root, os.path.join(element, command))
        if os.path.isfile(filename) and os.access(filename, os.X_OK):
            return True
    return False


def find_on_path(command):
    """Is command on the executable search path?"""
    if 'PATH' not in os.environ:
        return False
    path = os.environ['PATH']
    for element in path.split(os.pathsep):
        if not element:
            continue
        filename = os.path.join(element, command)
        if os.path.isfile(filename) and os.access(filename, os.X_OK):
            return True
    return False


def unlink_force(path):
    """Unlink path, without worrying about whether it exists."""
    try:
        os.unlink(path)
    except OSError:
        pass


def glob_root(root, pathname):
    """Like glob.iglob, but resolved relative to root.
    pathname must be absolute."""
    import glob

    fullpathname = os.path.join(root, pathname[1:])
    for path in glob.iglob(fullpathname):
        if not path.startswith(root):
            continue
        path = path[len(root):]
        if path and path[0] != '/':
            continue
        yield path
