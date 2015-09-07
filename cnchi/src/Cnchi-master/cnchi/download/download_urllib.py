#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  download_urllib.py
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

""" Module to download packages using urllib """

import os
import logging
import queue
import shutil
import urllib.request
import urllib.error
import http.client


def url_open(url):
    """ Helper function to open a remote file """

    if url is None:
        logging.warning(_("Wrong url, will try another one if available."))
        return None

    # Remove trailing spaces or new lines at the end of the string
    url = url.rstrip()

    try:
        urlp = urllib.request.urlopen(url)
    except urllib.error.HTTPError as err:
        urlp = None
        msg = _("Can't open {0} - Reason: {1}").format(url, err.reason)
        logging.warning(msg)
    except urllib.error.URLError as err:
        urlp = None
        msg = _("Can't open {0} - Reason: {1}").format(url, err.reason)
        logging.warning(msg)
    except http.client.BadStatusLine as err:
        urlp = None
        msg = _("Can't open {0} - Reason: {1}").format(url, err)
        logging.warning(msg)
    except AttributeError as err:
        urlp = None
        msg = _("Can't open {0} - Reason: {1}").format(url, err)
        logging.warning(msg)

    return urlp


def url_open_read(urlp, chunk_size=8192):
    """ Helper function to download and read a fragment of a remote file """

    download_error = True
    data = None

    try:
        data = urlp.read(chunk_size)
        download_error = False
    except urllib.error.HTTPError as err:
        msg = _('HTTP Error : {0}').format(err.reason)
        logging.error(msg)
    except urllib.error.URLError as err:
        msg = _('URL Error : {0}').format(err.reason)
        logging.error(msg)

    return data, download_error


class Download(object):
    """ Class to download packages using urllib
        This class tries to previously download all necessary packages for
        Apricity OS installation using urllib """

    def __init__(self, pacman_cache_dir, cache_dir, callback_queue):
        """ Initialize Download class. Gets default configuration """
        self.pacman_cache_dir = pacman_cache_dir
        self.cache_dir = cache_dir
        self.callback_queue = callback_queue

        # Stores last issued event (to prevent repeating events)
        self.last_event = {}

    def start(self, downloads):
        """ Downloads using urllib """

        downloaded = 0
        total_downloads = len(downloads)
        all_successful = True

        self.queue_event('downloads_progress_bar', 'show')
        self.queue_event('downloads_percent', '0')

        while len(downloads) > 0:
            identity, element = downloads.popitem()

            self.queue_event('percent', '0')

            txt = _("Downloading {0} {1} ({2}/{3})...")
            txt = txt.format(element['identity'], element['version'], downloaded + 1, total_downloads)
            self.queue_event('info', txt)

            try:
                total_length = int(element['size'])
            except TypeError:
                logging.warning(_("Metalink for package %s has no size info"), element['identity'])
                total_length = 0

            # If the user doesn't give us a cache dir to copy xz files from, self.cache_dir will be None
            if self.cache_dir:
                dst_cache_path = os.path.join(self.cache_dir, element['filename'])
            else:
                dst_cache_path = ""

            dst_path = os.path.join(self.pacman_cache_dir, element['filename'])

            needs_to_download = True

            if os.path.exists(dst_path):
                # File already exists (previous install?) do not download
                logging.warning(_("File %s already exists, Cnchi will not overwrite it"), element['filename'])
                needs_to_download = False
                downloaded += 1
            elif self.cache_dir and os.path.exists(dst_cache_path):
                # We're lucky, the package is already downloaded in the cache the user has given us
                # let's copy it to our destination
                logging.debug(_('%s found in iso pkg cache. Copying...'), element['filename'])
                try:
                    shutil.copy(dst_cache_path, dst_path)
                    needs_to_download = False
                    downloaded += 1
                except OSError as os_error:
                    logging.warning(_("Error copying %s to %s. Cnchi will try to download it"), dst_cache_path, dst_path)
                    logging.error(os_error)
                    needs_to_download = True

            if needs_to_download:
                # Let's download our filename using url
                download_error = True
                for url in element['urls']:
                    # msg = _("Downloading file from url {0}").format(url)
                    # logging.debug(msg)
                    download_error = True
                    percent = 0
                    completed_length = 0
                    urlp = url_open(url)
                    if urlp is not None:
                        with open(dst_path, 'wb') as xzfile:
                            (data, download_error) = url_open_read(urlp)

                            while not download_error and len(data) > 0:
                                xzfile.write(data)
                                completed_length += len(data)
                                old_percent = percent
                                if total_length > 0:
                                    percent = round(float(completed_length / total_length), 2)
                                else:
                                    percent += 0.1
                                if old_percent != percent:
                                    self.queue_event('percent', percent)
                                (data, download_error) = url_open_read(urlp)

                            if not download_error:
                                downloaded += 1
                                break
                                # else:
                                # try next mirror url
                                # completed_length = 0
                                # msg = _("Can't download {0}, will try another mirror if available").format(url)
                                # logging.warning(msg)
                                # else:
                                #    # try next mirror url
                                #    msg = _("Can't open {0}, will try another mirror if available").format(url)
                                #    logging.warning(msg)

                if download_error:
                    # None of the mirror urls works.
                    # This is not a total disaster, maybe alpm will be able
                    # to download it for us later in pac.py
                    msg = _("Can't download {0}, even after trying all available mirrors")
                    msg = msg.format(element['filename'])
                    all_successful = False
                    logging.warning(msg)

            downloads_percent = round(float(downloaded / total_downloads), 2)
            self.queue_event('downloads_percent', str(downloads_percent))

        self.queue_event('downloads_progress_bar', 'hide')
        return all_successful

    def queue_event(self, event_type, event_text=""):
        """ Adds an event to Cnchi event queue """

        if self.callback_queue is None:
            if event_type != "percent":
                logging.debug("{0}: {1}".format(event_type, event_text))
            return

        if event_type in self.last_event:
            if self.last_event[event_type] == event_text:
                # do not repeat same event
                return

        self.last_event[event_type] = event_text

        try:
            # Add the event
            self.callback_queue.put_nowait((event_type, event_text))
        except queue.Full:
            pass
