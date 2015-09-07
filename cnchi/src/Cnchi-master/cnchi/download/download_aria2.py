#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  download_aria2.py
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

""" Module to download packages using Aria2 """

import os
import logging
import queue
import shutil

import download.aria2 as aria2

try:
    _("")
except NameError as err:
    def _(message):
        return message

MAX_CONCURRENT_DOWNLOADS = 5


class Download(object):
    """ Class to download packages using Aria2
        This class tries to previously download all necessary packages for
        Apricity OS installation using aria2 """

    def __init__(self, pacman_cache_dir, cache_dir, callback_queue):
        """ Initialize DownloadAria2 class. Gets default configuration """
        self.pacman_cache_dir = pacman_cache_dir
        self.cache_dir = cache_dir
        self.callback_queue = callback_queue

        # Stores last issued event (to prevent repeating events)
        self.last_event = {}

        self.aria2 = aria2.Aria2(pacman_cache_dir, MAX_CONCURRENT_DOWNLOADS)

    def get_num_active(self):
        """ Get num of active downloads """
        num_active = 0
        try:
            global_stat = self.aria2.get_global_stat()
            if global_stat is not None and "numActive" in global_stat:
                num_active = int(global_stat["numActive"])
        except TypeError as type_error:
            logging.error(type_error)
        finally:
            return num_active

    def start(self, downloads):
        """ Downloads using aria2 """

        # Start Aria2
        self.aria2.run()

        if self.aria2.rpc_uid is None:
            logging.warning(_("Aria2 is not running."))
            return

        downloaded = 0
        total_downloads = len(downloads)
        percent = 0
        self.queue_event('percent', str(percent))
        txt = _("Downloading packages... ({0}/{1})...")
        txt = txt.format(downloaded, total_downloads)
        self.queue_event('info', txt)

        while len(downloads) > 0:
            num_active = self.get_num_active()

            while num_active < MAX_CONCURRENT_DOWNLOADS and len(downloads) > 0:
                identity, element = downloads.popitem()

                # If the user doesn't give us a cache dir to copy xz files from, self.cache_dir will be None
                if self.cache_dir:
                    src_cache_path = os.path.join(self.cache_dir, element['filename'])
                else:
                    src_cache_path = ""

                dst_path = os.path.join(self.pacman_cache_dir, element['filename'])

                self.show_download_info(downloaded + 1, total_downloads)

                if os.path.exists(dst_path):
                    # File already exists (previous install?) do not download
                    logging.warning(_("File %s already exists, Cnchi will not overwrite it"), element['filename'])
                    downloaded += 1
                elif self.cache_dir and os.path.exists(src_cache_path):
                    # We're lucky, the package is already downloaded in the cache the user has given us
                    # let's copy it to our destination
                    shutil.copy(src_cache_path, dst_path)
                    downloaded += 1
                else:
                    # Add file to aria2 downloads queue
                    # gid = self.aria2.add_uris(element['urls'])
                    num_active = self.get_num_active()

            num_active = self.get_num_active()
            old_num_active = num_active

            while num_active > 0:
                num_active = self.get_num_active()

                if old_num_active > num_active:
                    downloaded += old_num_active - num_active
                    old_num_active = num_active

                self.show_download_info(downloaded, total_downloads)

            # This method purges completed/error/removed downloads, in order to free memory
            self.aria2.purge_download_result()

        # Finished, close aria2
        self.aria2.shutdown()

    def show_download_info(self, downloaded, total_downloads):
        percent = round(float(downloaded / total_downloads), 2)
        self.queue_event('percent', str(percent))
        txt = _("Downloading packages... ({0}/{1})...")
        txt = txt.format(downloaded, total_downloads)
        self.queue_event('info', txt)

    def queue_event(self, event_type, event_text=""):
        """ Adds an event to Cnchi event queue """
        if self.callback_queue is None:
            logging.debug(event_text)
            return

        if event_type in self.last_event:
            if self.last_event[event_type] == event_text:
                # do not repeat same event
                return

        self.last_event[event_type] = event_text

        try:
            self.callback_queue.put_nowait((event_type, event_text))
        except queue.Full:
            pass
