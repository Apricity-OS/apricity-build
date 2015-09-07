#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  rank_mirrors.py
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

""" Creates mirrorlist sorted by both latest updates and fastest connection """

import threading
import subprocess
import logging
import time
import os
import shutil

import misc.misc as misc

import requests


class AutoRankmirrorsThread(threading.Thread):
    """ Thread class that downloads and sorts the mirrorlist """

    def __init__(self):
        """ Initialize thread class """
        super(AutoRankmirrorsThread, self).__init__()
        self.rankmirrors_pid = None
        self.reflector_script = "/usr/share/cnchi/scripts/update-mirrors.sh"
        self.antergos_mirrorlist = "/etc/pacman.d/antergos-mirrorlist"
        self.arch_mirrorlist = "/etc/pacman.d/mirrorlist"
        self.arch_mirror_status = "http://www.archlinux.org/mirrors/status/json/"

    @staticmethod
    def check_mirror_status(mirrors, url):
        for mirror in mirrors:
            if mirror['url'] in url and mirror['completion_pct'] == 1 and mirror['score'] <= 4:
                return True
        return False

    def run(self):
        """ Run thread """

        # Wait until there is an Internet connection available
        while not misc.has_connection():
            time.sleep(1)  # Delay

        if not os.path.exists(self.reflector_script):
            logging.warning(_("Can't find update mirrors script"))
            return

        # Uncomment Apricity mirrors and comment out auto selection so rankmirrors can find the best mirror.

        autoselect = "http://mirrors.antergos.com/$repo/$arch"

        # Make sure we have the latest antergos-mirrorlist
        with misc.raised_privileges:
            try:
                subprocess.check_call(['pacman', '-Syy', 'antergos-mirrorlist'])
            except subprocess.CalledProcessError as err:
                logging.debug(_('Update of antergos-mirrorlist package failed with error: %s', err))

        if os.path.exists(self.antergos_mirrorlist):
            with open(self.antergos_mirrorlist) as mirrors:
                lines = [x.strip() for x in mirrors.readlines()]

            for i in range(len(lines)):
                if lines[i].startswith("Server") and autoselect in lines[i]:
                    # Comment out auto selection
                    lines[i] = "#" + lines[i]
                elif lines[i].startswith("#Server") and autoselect not in lines[i]:
                    # Uncomment Apricity mirror
                    lines[i] = lines[i].lstrip("#")

            with misc.raised_privileges():
                # Backup original file
                shutil.copy(self.antergos_mirrorlist, self.antergos_mirrorlist + ".cnchi_backup")
                # Write new one
                with open(self.antergos_mirrorlist, 'w') as mirrors:
                    mirrors.write("\n".join(lines) + "\n")

        # Run rankmirrors command
        try:
            with misc.raised_privileges():
                self.rankmirrors_pid = subprocess.Popen([self.reflector_script]).pid

        except subprocess.CalledProcessError as process_error:
            logging.error(_("Couldn't execute auto mirror selection"))
            logging.error(process_error)

        # Check arch mirrorlist against mirror status data, remove any bad mirrors.
        if os.path.exists(self.arch_mirrorlist):
            # Use session to avoid silly warning
            # See https://github.com/kennethreitz/requests/issues/1882
            with requests.Session() as session:
                status = session.get(self.arch_mirror_status).json()
                mirrors = status['urls']

            with open(self.arch_mirrorlist) as arch_mirrors:
                lines = [x.strip() for x in arch_mirrors.readlines()]

            for i in range(len(lines)):
                server_uncommented = lines[i].startswith("Server")
                server_commented = lines[i].startswith("#Server")
                if server_commented or server_uncommented:
                    url = lines[i].split('=')[1].strip()
                    check = self.check_mirror_status(mirrors, url)
                    if not check and server_uncommented:
                        # Bad mirror, comment it
                        logging.debug('Removing bad mirror: %s', lines[i])
                        lines[i] = "#" + lines[i]
                    if check and server_commented:
                        # It's a good mirror, uncomment it
                        lines[i] = lines[i].lstrip("#")

            with misc.raised_privileges():
                # Backup original file
                shutil.copy(self.arch_mirrorlist, self.arch_mirrorlist + ".cnchi_backup")
                # Write new one
                with open(self.arch_mirrorlist, 'w') as arch_mirrors:
                    arch_mirrors.write("\n".join(lines) + "\n")

        logging.debug(_("Auto mirror selection has been run successfully"))
