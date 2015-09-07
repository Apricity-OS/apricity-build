#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  pacman_conf.py
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

""" Configuration module for Cnchi """

from multiprocessing import Queue


class Settings(object):
    """ Store all Cnchi setup options here """

    def __init__(self):
        """ Initialize default configuration """

        # Creates a one element size queue
        self.settings = Queue(1)

        self.settings.put({
            'auto_device': '/dev/sda',
            'bootloader': 'grub2',
            'bootloader_device': '/dev/sda',
            'bootloader_install': True,
            'bootloader_installation_successful': False,
            'btrfs': False,
            'cache': '',
            'cnchi': '/usr/share/cnchi/',
            'installer_thread_call': {},
            'data': '/usr/share/cnchi/data/',
            'desktop': 'gnome',
            'desktops': [],
            'download_library': 'urllib',
            'enable_alongside': True,
            'encrypt_home': False,
            'f2fs': False,
            'failed_download': False,
            'feature_aur': False,
            'feature_bluetooth': False,
            'feature_cups': False,
            'feature_firefox': False,
            'feature_firewall': False,
            'feature_fonts': False,
            'feature_lts': False,
            'feature_office': False,
            'feature_smb': False,
            'feature_visual': False,
            'fullname': '',
            'GRUB_CMDLINE_LINUX': '',
            'hostname': 'apricity',
            'keyboard_layout': '',
            'keyboard_variant': '',
            'language_name': '',
            'language_code': '',
            'location': '',
            'laptop': 'False',
            'locale': '',
            'log_file': '/tmp/cnchi.log',
            'luks_root_password': "",
            'luks_root_volume': "",
            'luks_root_device': "",
            'partition_mode': 'easy',
            'password': '',
            'rankmirrors_done': False,
            'require_password': True,
            'ruuid': '',
            'stop_all_threads': False,
            'third_party_software': False,
            'timezone_human_zone': '',
            'timezone_country': '',
            'timezone_zone': '',
            'timezone_human_country': '',
            'timezone_comment': '',
            'timezone_latitude': 0,
            'timezone_longitude': 0,
            'timezone_done': False,
            'timezone_start': False,
            'tmp': '/tmp',
            'ui': '/usr/share/cnchi/ui/',
            'use_home': False,
            'use_luks': False,
            'use_luks_in_root': False,
            'use_lvm': False,
            'use_ntp': True,
            'user_info_done': False,
            'username': '',
            'z_hidden': False})

    def _get_settings(self):
        """ Get a copy of our settings """
        settings = self.settings.get()
        copy = settings.copy()
        self.settings.put(settings)
        return copy

    def _update_settings(self, new_settings):
        """ Updates global settings """
        settings = self.settings.get()
        try:
            settings.update(new_settings)
        finally:
            self.settings.put(settings)

    def get(self, key):
        """ Get one setting value """
        settings = self._get_settings()
        return settings.get(key, None)

    def set(self, key, value):
        """ Set one setting's value """
        settings = self._get_settings()
        settings[key] = value
        self._update_settings(settings)
