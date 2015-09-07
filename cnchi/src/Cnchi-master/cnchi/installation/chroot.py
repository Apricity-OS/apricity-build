#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  chroot.py
#
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

""" Chroot related functions. Used in the installation process """

import logging
import os
import subprocess

# When testing, no _() is available
try:
    _("")
except NameError as err:
    def _(message):
        return message

_special_dirs_mounted = False


def get_special_dirs():
    """ Get special dirs to be mounted or unmounted """
    special_dirs = ["/dev", "/dev/pts", "/proc", "/sys"]    
    efi = "/sys/firmware/efi/efivars"
    if os.path.exists(efi):
        special_dirs.append(efi)
    return special_dirs

def mount_special_dirs(dest_dir):
    """ Mount special directories for our chroot (bind them)"""

    """
    There was an error creating the child process for this terminal
    grantpt failed: Operation not permitted
    """

    global _special_dirs_mounted

    # Don't try to remount them
    if _special_dirs_mounted:
        msg = _("Special dirs are already mounted. Skipping.")
        logging.debug(msg)
        return
    
    special_dirs = []
    special_dirs = get_special_dirs()

    for special_dir in special_dirs:
        mountpoint = os.path.join(dest_dir, special_dir[1:])
        if not os.path.exists(mountpoint):
            logging.debug("Making directory '{0}'".format(mountpoint))
            os.makedirs(mountpoint)
        os.chmod(mountpoint, 0o755)
        cmd = ["mount", "--bind", special_dir, mountpoint]
        logging.debug("Mounting special dir '{0}' to {1}".format(special_dir, mountpoint))
        try:
            subprocess.check_call(cmd)
        except subprocess.CalledProcessError as process_error:
            logging.warning(_("Unable to mount {0}".format(mountpoint)))
            logging.warning(_("Command {0} has failed.".format(process_error.cmd)))
            logging.warning(_("Output : {0}".format(process_error.output)))

    _special_dirs_mounted = True


def umount_special_dirs(dest_dir):
    """ Umount special directories for our chroot """

    global _special_dirs_mounted

    # Do not umount if they're not mounted
    if not _special_dirs_mounted:
        msg = _("Special dirs are not mounted. Skipping.")
        logging.debug(msg)
        return

    special_dirs = []
    special_dirs = get_special_dirs()

    for special_dir in reversed(special_dirs):
        mountpoint = os.path.join(dest_dir, special_dir[1:])
        logging.debug("Unmounting special dir '{0}'".format(mountpoint))
        try:            
            subprocess.check_call(["umount", mountpoint])
        except subprocess.CalledProcessError:
            logging.debug("Can't unmount. Try -l to force it.")
            try:
                subprocess.check_call(["umount", "-l", mountpoint])
            except subprocess.CalledProcessError as process_error:
                logging.warning(_("Unable to umount {0}".format(mountpoint)))
                logging.warning(_("Command {0} has failed.".format(process_error.cmd)))
                logging.warning(_("Output : {0}".format(process_error.output)))

    _special_dirs_mounted = False


def run(cmd, dest_dir, timeout=None, stdin=None):
    """ Runs command inside the chroot """
    full_cmd = ['chroot', dest_dir]

    for element in cmd:
        full_cmd.append(element)

    proc = None
    try:
        proc = subprocess.Popen(full_cmd,
                                stdin=stdin,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
        outs, errs = proc.communicate(timeout=timeout)
        txt = outs.decode().strip()
        if len(txt) > 0:
            logging.debug(txt)
    except subprocess.TimeoutExpired as timeout_error:
        if proc:
            proc.kill()
            proc.communicate()
        logging.error(_("Timeout running the command %s"), timeout_error.cmd)
        logging.error(_("Cnchi will try to continue anyways"))
    except OSError as os_error:
        logging.error(_("Error running command: %s"), os_error.strerror)
        logging.error(_("Cnchi will try to continue anyways"))
