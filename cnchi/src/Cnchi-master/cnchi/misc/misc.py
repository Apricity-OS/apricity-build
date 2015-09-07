#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
#  Copyright (c) 2012 Canonical Ltd.
#  Copyright (c) 2015 Apricity
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

from collections import namedtuple
import contextlib
import grp
import os
import pwd
import re
import shutil
import subprocess
import syslog
import socket
import locale
import logging
import dbus
import urllib
from socket import timeout

import misc.osextras as osextras

NM = 'org.freedesktop.NetworkManager'
NM_STATE_CONNECTED_GLOBAL = 70

_dropped_privileges = 0


def copytree(src_dir, dst_dir, symlinks=False, ignore=None):
    for item in os.listdir(src_dir):
        s = os.path.join(src_dir, item)
        d = os.path.join(dst_dir, item)
        if os.path.isdir(s):
            shutil.copytree(s, d, symlinks, ignore)
        else:
            shutil.copy2(s, d)


def utf8(s, errors="strict"):
    """Decode a string as UTF-8 if it isn't already Unicode."""
    if isinstance(s, str):
        return s
    else:
        return str(s, "utf-8", errors)


def is_swap(device):
    try:
        with open('/proc/swaps') as fp:
            for line in fp:
                if line.startswith(device + ' '):
                    return True
    except OSError as os_error:
        logging.warning(os_error)
    return False


def set_groups_for_uid(uid):
    if uid == os.geteuid() or uid == os.getuid():
        return
    user = pwd.getpwuid(uid).pw_name
    try:
        os.setgroups([g.gr_gid for g in grp.getgrall() if user in g.gr_mem])
    except OSError:
        import traceback

        for line in traceback.format_exc().split('\n'):
            syslog.syslog(syslog.LOG_ERR, line)


def drop_all_privileges():
    # gconf needs both the UID and effective UID set.
    global _dropped_privileges
    uid = os.environ.get('SUDO_UID')
    gid = os.environ.get('SUDO_GID')
    if uid is not None:
        uid = int(uid)
        set_groups_for_uid(uid)
    if gid is not None:
        gid = int(gid)
        os.setregid(gid, gid)
    if uid is not None:
        uid = int(uid)
        os.setreuid(uid, uid)
        os.environ['HOME'] = pwd.getpwuid(uid).pw_dir
        os.environ['LOGNAME'] = pwd.getpwuid(uid).pw_name
    _dropped_privileges = None


def drop_privileges():
    global _dropped_privileges
    assert _dropped_privileges is not None
    if _dropped_privileges == 0:
        uid = os.environ.get('SUDO_UID')
        gid = os.environ.get('SUDO_GID')
        if uid is not None:
            uid = int(uid)
            set_groups_for_uid(uid)
        if gid is not None:
            gid = int(gid)
            os.setegid(gid)
        if uid is not None:
            os.seteuid(uid)
    _dropped_privileges += 1


def regain_privileges():
    global _dropped_privileges
    assert _dropped_privileges is not None
    _dropped_privileges -= 1
    if _dropped_privileges == 0:
        os.seteuid(0)
        os.setegid(0)
        os.setgroups([])


def drop_privileges_save():
    """Drop the real UID/GID as well, and hide them in saved IDs."""
    # At the moment, we only know how to handle this when effective
    # privileges were already dropped.
    assert _dropped_privileges is not None and _dropped_privileges > 0
    uid = os.environ.get('SUDO_UID')
    gid = os.environ.get('SUDO_GID')
    if uid is not None:
        uid = int(uid)
        set_groups_for_uid(uid)
    if gid is not None:
        gid = int(gid)
        os.setresgid(gid, gid, 0)
    if uid is not None:
        os.setresuid(uid, uid, 0)


def regain_privileges_save():
    """Recover our real UID/GID after calling drop_privileges_save."""
    assert _dropped_privileges is not None and _dropped_privileges > 0
    os.setresuid(0, 0, 0)
    os.setresgid(0, 0, 0)
    os.setgroups([])


@contextlib.contextmanager
def raised_privileges():
    """As regain_privileges/drop_privileges, but in context manager style."""
    regain_privileges()
    try:
        yield
    finally:
        drop_privileges()


def raise_privileges(func):
    """As raised_privileges, but as a function decorator."""
    from functools import wraps

    @wraps(func)
    def helper(*args, **kwargs):
        with raised_privileges():
            return func(*args, **kwargs)

    return helper


@raise_privileges
def grub_options():
    """ Generates a list of suitable targets for grub-installer
        @return empty list or a list of ['/dev/sda1','Ubuntu Hardy 8.04'] """
    from ubiquity.parted_server import PartedServer

    l = []
    try:
        oslist = {}
        subp = subprocess.Popen(
            ['os-prober'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            universal_newlines=True)
        result = subp.communicate()[0].splitlines()
        for res in result:
            res = res.split(':')
            oslist[res[0]] = res[1]
        p = PartedServer()
        for disk in p.disks():
            p.select_disk(disk)
            with open(p.device_entry('model')) as fp:
                mod = fp.readline()
            with open(p.device_entry('device')) as fp:
                dev = fp.readline()
            with open(p.device_entry('size')) as fp:
                size = fp.readline()
            if dev and mod:
                if size.isdigit():
                    size = format_size(int(size))
                    l.append([dev, '{0} ({1})'.format(mod, size)])
                else:
                    l.append([dev, mod])
            for part in p.partitions():
                ostype = ''
                if part[4] == 'linux-swap':
                    continue
                if part[4] == 'free':
                    continue
                if os.path.exists(p.part_entry(part[1], 'format')):
                    # Don't bother looking for an OS type.
                    pass
                elif part[5] in oslist.keys():
                    ostype = oslist[part[5]]
                l.append([part[5], ostype])
    except Exception:
        import traceback

        for line in traceback.format_exc().split('\n'):
            syslog.syslog(syslog.LOG_ERR, line)
    return l


@raise_privileges
def boot_device():
    from ubiquity.parted_server import PartedServer

    boot = None
    root = None
    try:
        p = PartedServer()
        for disk in p.disks():
            p.select_disk(disk)
            for part in p.partitions():
                part = part[1]
                if p.has_part_entry(part, 'mountpoint'):
                    mp = p.readline_part_entry(part, 'mountpoint')
                    if mp == '/boot':
                        boot = disk.replace('=', '/')
                    elif mp == '/':
                        root = disk.replace('=', '/')
    except Exception:
        import traceback

        for line in traceback.format_exc().split('\n'):
            syslog.syslog(syslog.LOG_ERR, line)
    if boot:
        return boot
    return root


def is_removable(device):
    if device is None:
        return None
    device = os.path.realpath(device)
    devpath = None
    is_partition = False
    removable_bus = False
    subp = subprocess.Popen(['udevadm', 'info', '-q', 'property', '-n', device],
                            stdout=subprocess.PIPE, universal_newlines=True)
    for line in subp.communicate()[0].splitlines():
        line = line.strip()
        if line.startswith('DEVPATH='):
            devpath = line[8:]
        elif line == 'DEVTYPE=partition':
            is_partition = True
        elif line == 'ID_BUS=usb' or line == 'ID_BUS=ieee1394':
            removable_bus = True

    if devpath is not None:
        if is_partition:
            devpath = os.path.dirname(devpath)
        is_device_removable = removable_bus
        try:
            removable_path = '/sys{0}/removable'.format(devpath)
            with open(removable_path) as removable:
                if removable.readline().strip() != '0':
                    is_device_removable = True
        except IOError:
            pass
        if is_device_removable:
            try:
                subp = subprocess.Popen(['udevadm', 'info', '-q', 'name', '-p', devpath],
                                        stdout=subprocess.PIPE,
                                        universal_newlines=True)
                return os.path.join('/dev', subp.communicate()[0].splitlines()[0].strip())
            except Exception:
                pass
    return None


def mount_info(path):
    """Return filesystem name, type, and ro/rw for a given mountpoint."""
    fsname = ''
    fstype = ''
    writable = ''
    with open('/proc/mounts') as fp:
        for line in fp:
            line = line.split()
            if line[1] == path:
                fsname = line[0]
                fstype = line[2]
                writable = line[3].split(',')[0]
    return fsname, fstype, writable


def udevadm_info(args):
    fullargs = ['udevadm', 'info', '-q', 'property']
    fullargs.extend(args)
    udevadm = {}
    subp = subprocess.Popen(
        fullargs, stdout=subprocess.PIPE, universal_newlines=True)
    for line in subp.communicate()[0].splitlines():
        line = line.strip()
        if '=' not in line:
            continue
        name, value = line.split('=', 1)
        udevadm[name] = value
    return udevadm


def partition_to_disk(partition):
    """Convert a partition device to its disk device, if any."""
    udevadm_part = udevadm_info(['-n', partition])
    if 'DEVPATH' not in udevadm_part or udevadm_part.get('DEVTYPE') != 'partition':
        return partition

    disk_syspath = os.path.join('/sys', udevadm_part['DEVPATH'].rsplit('/', 1)[0])
    udevadm_disk = udevadm_info(['-p', disk_syspath])
    return udevadm_disk.get('DEVNAME', partition)


def is_boot_device_removable(boot=None):
    if boot:
        return is_removable(boot)
    else:
        return is_removable(boot_device())


def cdrom_mount_info():
    """Return mount information for /cdrom.

    This is the same as mount_info, except that the partition is converted to
    its containing disk, and we don't care whether the mount point is
    writable.
    """
    cdsrc, cdfs, _ = mount_info('/cdrom')
    cdsrc = partition_to_disk(cdsrc)
    return cdsrc, cdfs


@raise_privileges
def grub_device_map():
    """Return the contents of the default GRUB device map."""
    subp = subprocess.Popen(['grub-mkdevicemap', '--no-floppy', '-m', '-'],
                            stdout=subprocess.PIPE, universal_newlines=True)
    return subp.communicate()[0].splitlines()


def grub_default(boot=None):
    """Return the default GRUB installation target."""

    # Much of this is intentionally duplicated from grub-installer, so that
    # we can show the user what device GRUB will be installed to before
    # grub-installer is run.  Pursuant to that, we intentionally run this in
    # the installer root as /target might not yet be available.

    bootremovable = is_boot_device_removable(boot=boot)
    if bootremovable is not None:
        return bootremovable

    devices = grub_device_map()
    target = None
    if devices:
        try:
            target = os.path.realpath(devices[0].split('\t')[1])
        except (IndexError, OSError):
            pass
    # last resort
    if target is None:
        target = '(hd0)'

    cdsrc, cdfs = cdrom_mount_info()
    try:
        # The target is usually under /dev/disk/by-id/, so string equality
        # is insufficient.
        same = os.path.samefile(cdsrc, target)
    except OSError:
        same = False
    if ((same or target == '(hd0)') and
            ((cdfs and cdfs != 'iso9660') or is_removable(cdsrc))):
        # Installing from removable media other than a CD.  Make sure that
        # we don't accidentally install GRUB to it.
        boot = boot_device()
        try:
            if boot:
                target = boot
            else:
                # Try the next disk along (which can't also be the CD source).
                target = os.path.realpath(devices[1].split('\t')[1])
            target = re.sub(r'(/dev/(cciss|ida)/c[0-9]d[0-9]|/dev/[a-z]+).*',
                            r'\1', target)
        except (IndexError, OSError):
            pass

    return target


_os_prober_oslist = {}
_os_prober_osvers = {}
_os_prober_called = False


def find_in_os_prober(device, with_version=False):
    """Look for the device name in the output of os-prober.

    Return the friendly name of the device, or the empty string on error.
    """
    try:
        oslist, osvers = os_prober()
        if device in oslist:
            ret = oslist[device]
        elif is_swap(device):
            ret = 'swap'
        else:
            syslog.syslog('Device {0} not found in os-prober output'.format(device))
            ret = ''
        ret = utf8(ret, errors='replace')
        ver = utf8(osvers.get(device, ''), errors='replace')
        if with_version:
            return ret, ver
        else:
            return ret
    except (KeyboardInterrupt, SystemExit):
        pass
    except Exception:
        import traceback

        syslog.syslog(syslog.LOG_ERR, "Error in find_in_os_prober:")
        for line in traceback.format_exc().split('\n'):
            syslog.syslog(syslog.LOG_ERR, line)
    return ''


@raise_privileges
def os_prober():
    global _os_prober_oslist
    global _os_prober_osvers
    global _os_prober_called

    if not _os_prober_called:
        _os_prober_called = True
        subp = subprocess.Popen(
            ['os-prober'], stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            universal_newlines=True)
        result = subp.communicate()[0].splitlines()
        for res in result:
            res = res.split(':')
            if res[2] == 'Ubuntu':
                version = [v for v in re.findall('[0-9.]*', res[1]) if v][0]
                # Get rid of the superfluous (development version) (11.04)
                text = re.sub('\s*\(.*\).*', '', res[1])
                _os_prober_oslist[res[0]] = text
                _os_prober_osvers[res[0]] = version
            else:
                # Get rid of the bootloader indication. It's not relevant here.
                _os_prober_oslist[res[0]] = res[1].replace(' (loader)', '')
    return _os_prober_oslist, _os_prober_osvers


@raise_privileges
def remove_os_prober_cache():
    osextras.unlink_force('/var/lib/ubiquity/os-prober-cache')
    shutil.rmtree('/var/lib/ubiquity/linux-boot-prober-cache',
                  ignore_errors=True)


def windows_startup_folder(mount_path):
    locations = [
        # Windows 8
        'ProgramData/Microsoft/Windows/Start Menu/Programs/StartUp',
        # Windows 7
        'ProgramData/Microsoft/Windows/Start Menu/Programs/Startup',
        # Windows XP
        'Documents and Settings/All Users/Start Menu/Programs/Startup',
        # Windows NT
        'Winnt/Profiles/All Users/Start Menu/Programs/Startup',
    ]
    for location in locations:
        path = os.path.join(mount_path, location)
        if os.path.exists(path):
            return path
    return ''


ReleaseInfo = namedtuple('ReleaseInfo', 'name, version')


def get_release():
    if get_release.release_info is None:
        try:
            with open('/cdrom/.disk/info') as fp:
                line = fp.readline()
                if line:
                    line = line.split()
                    if line[2] == 'LTS':
                        line[1] += ' LTS'
                    get_release.release_info = ReleaseInfo(name=line[0], version=line[1])
        except Exception:
            syslog.syslog(syslog.LOG_ERR, 'Unable to determine the release.')

        if not get_release.release_info:
            get_release.release_info = ReleaseInfo(name='Ubuntu', version='')
    return get_release.release_info


get_release.release_info = None


def get_release_name():
    import warnings

    warnings.warn('get_release_name() is deprecated, '
                  'use get_release().name instead.',
                  category=DeprecationWarning)

    if not get_release_name.release_name:
        try:
            with open('/cdrom/.disk/info') as fp:
                line = fp.readline()
                if line:
                    line = line.split()
                    if line[2] == 'LTS':
                        get_release_name.release_name = ' '.join(line[:3])
                    else:
                        get_release_name.release_name = ' '.join(line[:2])
        except Exception:
            syslog.syslog(
                syslog.LOG_ERR,
                "Unable to determine the distribution name from "
                "/cdrom/.disk/info")
        if not get_release_name.release_name:
            get_release_name.release_name = 'Ubuntu'
    return get_release_name.release_name


get_release_name.release_name = ''


@raise_privileges
def get_install_medium():
    if not get_install_medium.medium:
        try:
            if os.access('/cdrom', os.W_OK):
                get_install_medium.medium = 'USB'
            else:
                get_install_medium.medium = 'CD'
        except Exception:
            syslog.syslog(
                syslog.LOG_ERR, "Unable to determine install medium.")
            get_install_medium.medium = 'CD'
    return get_install_medium.medium


get_install_medium.medium = ''


def execute(*args):
    """runs args* in shell mode. Output status is taken."""

    log_args = ['log-output', '-t', 'ubiquity']
    log_args.extend(args)

    try:
        status = subprocess.call(log_args)
    except IOError as e:
        syslog.syslog(syslog.LOG_ERR, ' '.join(log_args))
        syslog.syslog(syslog.LOG_ERR, "OS error({0}): {1}".format(e.errno, e.strerror))
        return False
    else:
        if status != 0:
            syslog.syslog(syslog.LOG_ERR, ' '.join(log_args))
            return False
        syslog.syslog(' '.join(log_args))
        return True


@raise_privileges
def execute_root(*args):
    return execute(*args)


def format_size(size):
    """Format a partition size."""
    if size < 1000:
        unit = 'B'
        factor = 1
    elif size < 1000 * 1000:
        unit = 'kB'
        factor = 1000
    elif size < 1000 * 1000 * 1000:
        unit = 'MB'
        factor = 1000 * 1000
    elif size < 1000 * 1000 * 1000 * 1000:
        unit = 'GB'
        factor = 1000 * 1000 * 1000
    else:
        unit = 'TB'
        factor = 1000 * 1000 * 1000 * 1000
    return '%.1f %s' % (float(size) / factor, unit)


def debconf_escape(text):
    escaped = text.replace('\\', '\\\\').replace('\n', '\\n')
    return re.sub(r'(\s)', r'\\\1', escaped)


def create_bool(text):
    if text == 'true':
        return True
    elif text == 'false':
        return False
    else:
        return text


@raise_privileges
def dmimodel():
    model = ''
    kwargs = {}
    if os.geteuid() != 0:
        # Silence annoying warnings during the test suite.
        kwargs['stderr'] = open('/dev/null', 'w')
    try:
        proc = subprocess.Popen(
            ['dmidecode', '--string', 'system-manufacturer'],
            stdout=subprocess.PIPE, universal_newlines=True, **kwargs)
        manufacturer = proc.communicate()[0]
        if not manufacturer:
            return
        manufacturer = manufacturer.lower()
        if 'to be filled' in manufacturer:
            # Don't bother with products in development.
            return
        if 'bochs' in manufacturer or 'vmware' in manufacturer:
            model = 'virtual machine'
            # VirtualBox sets an appropriate system-product-name.
        else:
            if 'lenovo' in manufacturer or 'ibm' in manufacturer:
                key = 'system-version'
            else:
                key = 'system-product-name'
            proc = subprocess.Popen(['dmidecode', '--string', key],
                                    stdout=subprocess.PIPE,
                                    universal_newlines=True)
            model = proc.communicate()[0]
        if 'apple' in manufacturer:
            # MacBook4,1 - strip the 4,1
            model = re.sub('[^a-zA-Z\s]', '', model)
        # Replace each gap of non-alphanumeric characters with a dash.
        # Ensure the resulting string does not begin or end with a dash.
        model = re.sub('[^a-zA-Z0-9]+', '-', model).rstrip('-').lstrip('-')
        if model.lower() == 'not-available':
            return
    except Exception:
        syslog.syslog(syslog.LOG_ERR, 'Unable to determine the model from DMI')
    finally:
        if 'stderr' in kwargs:
            kwargs['stderr'].close()
    return model


def set_indicator_keymaps(lang):
    import xml.etree.cElementTree as ElementTree
    from gi.repository import Xkl, GdkX11
    # GdkX11.x11_get_default_xdisplay() segfaults if Gtk hasn't been
    # imported; possibly finer-grained than this, but anything using this
    # will already have imported Gtk anyway ...
    from gi.repository import Gtk
    from ubiquity import gsettings

    gsettings_key = ['org.gnome.libgnomekbd.keyboard', 'layouts']
    lang = lang.split('_')[0]
    variants = []

    # Map inspired from that of gfxboot-theme-ubuntu that's itself
    # based on console-setup's. This one has been restricted to
    # language => keyboard layout not locale => keyboard layout as
    # we don't actually know the exact locale
    default_keymap = {
        'ar': 'ara',
        'bs': 'ba',
        'de': 'de',
        'el': 'gr',
        'en': 'us',
        'eo': 'epo',
        'fr': 'fr_oss',
        'gu': 'in_guj',
        'hi': 'in',
        'hr': 'hr',
        'hy': 'am',
        'ka': 'ge',
        'kn': 'in_kan',
        'lo': 'la',
        'ml': 'in_mal',
        'pa': 'in_guru',
        'sr': 'rs',
        'sv': 'se',
        'ta': 'in_tam',
        'te': 'in_tel',
        'zh': 'cn',
    }

    def item_str(s):
        """ Convert a zero-terminated byte array to a proper str """
        i = s.find(b'\x00')
        return s[:i].decode()

    def process_variant(*args):
        if hasattr(args[2], 'name'):
            variants.append('{0}\t{1}'.format((item_str(args[1].name), item_str(args[2].name))))
        else:
            variants.append(item_str(args[1].name))

    def restrict_list(variants):
        new_variants = []

        # Start by looking by an explicit default layout in the keymap
        if lang in default_keymap:
            if default_keymap[lang] in variants:
                variants.remove(default_keymap[lang])
                new_variants.append(default_keymap[lang])
            else:
                tab_keymap = default_keymap[lang].replace('_', '\t')
                if tab_keymap in variants:
                    variants.remove(tab_keymap)
                    new_variants.append(tab_keymap)

        # Prioritize the layout matching the language (if any)
        if lang in variants:
            variants.remove(lang)
            new_variants.append(lang)

        # Uniquify our list (just in case)
        variants = list(set(variants))

        if len(variants) > 4:
            # We have a problem, X only supports 4

            # Add as many entry as we can that are layouts without variant
            country_variants = sorted(
                entry for entry in variants if '\t' not in entry)
            for entry in country_variants[:4 - len(new_variants)]:
                new_variants.append(entry)
                variants.remove(entry)

            if len(new_variants) < 4:
                # We can add some more
                simple_variants = sorted(
                    entry for entry in variants if '_' not in entry)
                for entry in simple_variants[:4 - len(new_variants)]:
                    new_variants.append(entry)
                    variants.remove(entry)

            if len(new_variants) < 4:
                # Now just add anything left
                for entry in variants[:4 - len(new_variants)]:
                    new_variants.append(entry)
                    variants.remove(entry)
        else:
            new_variants = new_variants + list(variants)

        # gsettings doesn't understand utf8
        new_variants = [str(variant) for variant in new_variants]

        return new_variants

    def call_setxkbmap(variants):
        kb_layouts = []
        kb_variants = []

        for entry in variants:
            fields = entry.split('\t')
            if len(fields) > 1:
                kb_layouts.append(fields[0])
                kb_variants.append(fields[1])
            else:
                kb_layouts.append(fields[0])
                kb_variants.append("")

        execute(
            "setxkbmap", "-layout", ",".join(kb_layouts),
            "-variant", ",".join(kb_variants))

    iso_639_3 = ElementTree.parse('/usr/share/xml/iso-codes/iso_639_3.xml')
    nodes = [element for element in iso_639_3.findall('iso_639_3_entry')
             if element.get('part1_code') == lang]
    display = GdkX11.x11_get_default_xdisplay()
    engine = Xkl.Engine.get_instance(display)
    if nodes:
        configreg = Xkl.ConfigRegistry.get_instance(engine)
        configreg.load(False)

        # Apparently part2_code doesn't always work (fails with French)
        for prop in ('part2_code', 'id', 'part1_code'):
            code = nodes[0].get(prop)
            if code is not None:
                configreg.foreach_language_variant(code, process_variant, None)
                if variants:
                    restricted_variants = restrict_list(variants)
                    call_setxkbmap(restricted_variants)
                    gsettings.set_list(
                        gsettings_key[0], gsettings_key[1],
                        restricted_variants)
                    break
        else:
            # Use the system default if no other keymaps can be determined.
            gsettings.set_list(gsettings_key[0], gsettings_key[1], [])

    engine.lock_group(0)


def get_prop(obj, iface, prop):
    try:
        return obj.Get(iface, prop, dbus_interface=dbus.PROPERTIES_IFACE)
    except (dbus.DBusException, dbus.exceptions.DBusException) as err:
        if err.get_dbus_name() == 'org.freedesktop.DBus.Error.UnknownMethod':
            return None
        else:
            raise


def is_wireless_enabled():
    bus = dbus.SystemBus()
    manager = bus.get_object(NM, '/org/freedesktop/NetworkManager')
    return get_prop(manager, NM, 'WirelessEnabled')


def get_nm_state():
    try:
        bus = dbus.SystemBus()
        manager = bus.get_object(NM, '/org/freedesktop/NetworkManager')
        state = get_prop(manager, NM, 'state')
    except (dbus.DBusException, dbus.exceptions.DBusException) as dbus_err:
        logging.warning(dbus_err)
        state = False
    finally:
        return state


def has_connection():
    # In a Virtualbox VM this returns true even when the host OS has no connection
    # karasu: But the ip idea is not good too. It fails under too many circumstances
    # (in this case is better a false positive than a false negative)
    if get_nm_state() == NM_STATE_CONNECTED_GLOBAL:
        return True

    try:
        url = 'http://74.125.228.100'
        urllib.request.urlopen(url, timeout=5)
        return True
    except (OSError, timeout, urllib.error.URLError) as err:
        logging.warning(err)
        return False


def add_connection_watch(func):
    def connection_cb(state):
        func(state == NM_STATE_CONNECTED_GLOBAL)

    bus = dbus.SystemBus()
    bus.add_signal_receiver(connection_cb, 'StateChanged', NM, NM)
    try:
        func(has_connection())
    except (dbus.DBusException, dbus.exceptions.DBusException) as err:
        logging.warning(err)
        # We can't talk to NM, so no idea.  Wild guess: we're connected
        # using ssh with X forwarding, and are therefore connected.  This
        # allows us to proceed with a minimum of complaint.
        func(True)


def install_size():
    if min_install_size:
        return min_install_size

    # Fallback size to 5 GB
    size = 5 * 1024 * 1024 * 1024

    # Maximal size to 8 GB
    max_size = 8 * 1024 * 1024 * 1024

    try:
        with open('/cdrom/casper/filesystem.size') as fp:
            size = int(fp.readline())
    except IOError:
        pass

    # TODO substitute into the template for the state box.
    min_disk_size = size * 2  # fudge factor

    # Set minimum size to 8GB if current minimum size is larger
    # than 8GB and we still have an extra 20% of free space
    if min_disk_size > max_size > 1.2 * size:
        min_disk_size = max_size

    return min_disk_size


min_install_size = None


def get_network():
    intip = False
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("antergos.com", 1234))
    except Exception:
        return ""
    myip = s.getsockname()[0]
    s.close()
    spip = myip.split(".")
    if spip[0] == '192':
        if spip[1] == '168':
            intip = True
    elif spip[0] == '10':
        intip = True
    elif spip[0] == '172':
        if 15 < int(spip[1]) < 32:
            intip = True
    if intip:
        ipran = '.'.join(spip[:-1]) + ".0/24"
    else:
        ipran = '.'.join(spip)
    return ipran


def sort_list(mylist, mylocale=""):
    try:
        import functools
    except ImportError as err:
        logging.warning(err)
        return mylist

    if mylocale != "":
        set_locale(mylocale)

    sorted_list = sorted(mylist, key=functools.cmp_to_key(locale.strcoll))

    return sorted_list


def set_locale(mylocale):
    try:
        locale.setlocale(locale.LC_ALL, mylocale)
        logging.info(_("locale changed to : %s"), mylocale)
    except locale.Error as err:
        logging.warning(_("Can't change to locale '%s' : %s"), mylocale, err)
        if mylocale.endswith(".UTF-8"):
            # Try without the .UTF-8 trailing
            mylocale = mylocale[:-len(".UTF-8")]
            try:
                locale.setlocale(locale.LC_ALL, mylocale)
                logging.info(_("locale changed to : %s"), mylocale)
            except locale.Error as err:
                logging.warning(_("Can't change to locale '%s'"), mylocale)
                logging.warning(err)
        else:
            logging.warning(_("Can't change to locale '%s'"), mylocale)


def gtk_refresh():
    """ Tell Gtk loop to run pending events """
    from gi.repository import Gtk

    while Gtk.events_pending():
        Gtk.main_iteration()


def remove_temp_files():
    """ Remove Cnchi temporary files """
    temp_files = [
        ".setup-running",
        ".km-running",
        "setup-pacman-running",
        "setup-mkinitcpio-running",
        ".tz-running",
        ".setup",
        "Cnchi.log"]
    for temp in temp_files:
        path = os.path.join("/tmp", temp)
        if os.path.exists(path):
            # FIXME: Some of these tmp files are created with sudo privileges
            with raised_privileges():
                os.remove(path)


def set_cursor(cursor_type):
    """ Set mouse cursor """
    from gi.repository import Gdk

    screen = Gdk.Screen.get_default()
    window = Gdk.Screen.get_root_window(screen)
    if window:
        display = Gdk.Display.get_default()
        cursor = Gdk.Cursor.new_for_display(display, cursor_type)
        window.set_cursor(cursor)
        gtk_refresh()

def partition_exists(partition):
    """ Check if a partition already exists """
    if "/dev/" in partition:
        partition = partition[len("/dev/"):]

    exists = False
    with open("/proc/partitions") as partitions:
        if partition in partitions.read():
            exists = True
    return exists


def is_partition_extended(partition):
    """ Check if a partition is of extended type """

    if "/dev/mapper" in partition:
        return False

    if "/dev/" in partition:
        partition = partition[len("/dev/"):]

    num = partition[len("sdX"):]
    if len(num) == 0:
        return False

    try:
        num = int(num)
    except ValueError as err:
        logging.warning(err)
        return False

    if num > 4:
        # logical partition
        return False

    with open("/proc/partitions") as partitions:
        lines = partitions.readlines()
    for line in lines:
        if "major" not in line:
            info = line.split()
            if len(info) > 0 and info[2] == '1' and info[3] == partition:
                return True
    return False


def get_partitions():
    partitions_list = []
    with open("/proc/partitions") as partitions:
        lines = partitions.readlines()
    for line in lines:
        if "major" not in line:
            info = line.split()
            if len(info) > 0:
                if len(info[3]) > len("sdX") and "loop" not in info[3]:
                    partitions_list.append("/dev/" + info[3])
    return partitions_list


class InstallError(Exception):
    """ Exception class called upon an installer error """

    def __init__(self, message):
        """ Initialize exception class """
        super().__init__(message)
        self.message = message

    def __str__(self):
        """ Returns exception message """
        return repr(self.message)
