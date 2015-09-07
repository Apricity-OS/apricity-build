#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  aria2.py
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

""" Module to comunicate with Aria2 """

import os
import subprocess
import xmlrpc.client
import logging

ARIA2_URL = 'http://localhost:6800/rpc'


class Aria2(object):
    def __init__(self, pacman_cache_dir, max_concurrent_downloads):
        self.pacman_cache_dir = pacman_cache_dir
        self.max_concurrent_downloads = max_concurrent_downloads
        self.rpc_uid = None

    def get_global_stat(self):
        """ This method returns global statistics such as
            the overall download and upload speeds. """
        stat = None
        try:
            s = xmlrpc.client.ServerProxy(ARIA2_URL)
            stat = s.aria2.getGlobalStat(self.rpc_uid)
        except (xmlrpc.client.Fault, ConnectionRefusedError, BrokenPipeError) as err:
            logging.debug(_("Can't call Aria2. Error Output: %s"), err)
        finally:
            return stat

    def tell_active(self, keys):
        """ This method returns  a  list  of  active  downloads. """
        active = None
        try:
            s = xmlrpc.client.ServerProxy(ARIA2_URL)
            active = s.aria2.tellActive(self.rpc_uid, keys)
        except (xmlrpc.client.Fault, ConnectionRefusedError, BrokenPipeError) as err:
            logging.debug(_("Can't call Aria2. Error Output: %s"), err)
        finally:
            return active

    def add_metalink(self, metalink):
        """ This method adds a Metalink download by uploading a ".metalink" file. """
        gids = []
        if metalink is not None:
            try:
                s = xmlrpc.client.ServerProxy(ARIA2_URL)
                binary_metalink = xmlrpc.client.Binary(str(metalink).encode())
                gids = s.aria2.addMetalink(self.rpc_uid, binary_metalink)
            except (xmlrpc.client.Fault, ConnectionRefusedError, BrokenPipeError, OverflowError) as err:
                logging.error(_("Can't add metalink to Aria2. Error Output: %s"), err)
            finally:
                return gids

        return gids

    def add_uris(self, uris):
        """ This method adds an uri list (of the same file) to the aria2's downloads list """
        gid = None
        try:
            s = xmlrpc.client.ServerProxy(ARIA2_URL)
            gid = s.aria2.addUri(self.rpc_uid, uris)
        except (xmlrpc.client.Fault, ConnectionRefusedError, BrokenPipeError, OverflowError) as err:
            logging.error(_("Can't add uris to Aria2. Error Output: %s"), err)
        finally:
            return gid

    def purge_download_result(self):
        """ This method purges completed/error/removed downloads to free memory. """
        try:
            s = xmlrpc.client.ServerProxy(ARIA2_URL)
            s.aria2.purgeDownloadResult(self.rpc_uid)
        except (xmlrpc.client.Fault, ConnectionRefusedError, BrokenPipeError) as err:
            logging.debug(_("Can't call Aria2. Error Output: %s"), err)

    def shutdown(self):
        """ This method shuts down aria2 """
        try:
            s = xmlrpc.client.ServerProxy(ARIA2_URL)
            s.aria2.shutdown(self.rpc_uid)
        except (xmlrpc.client.Fault, ConnectionRefusedError, BrokenPipeError) as err:
            logging.debug(_("Can't call Aria2. Error Output: %s"), err)

    def run(self):
        """ Run aria2 in daemon mode """

        import uuid
        uid = uuid.uuid4().hex[:16]

        pid = os.getpid()

        aria2_options = [
            "--allow-overwrite=false",      # If file is already downloaded overwrite it
            "--always-resume=true",         # Always resume download.
            "--auto-file-renaming=false",   # Rename file name if the same file already exists.
            "--auto-save-interval=0",       # Save a control file(*.aria2) every SEC seconds.
            "--dir={0}".format(self.pacman_cache_dir),  # The directory to store the downloaded file(s).
            "--enable-rpc=true",            # Enable XML-RPC server.
            "--file-allocation=prealloc",   # Specify file allocation method (default 'prealloc')
            "--log=/tmp/cnchi-aria2.log",   # The file name of the log file
            "--log-level=warn",             # Set log level to output to console. LEVEL is either debug, info, notice,
                                            # warn or error (default notice)
            "--min-split-size=20M",         # Do not split less than 2*SIZE byte range (default 20M)
                                            # Set maximum number of parallel downloads for each metalink (default 5)
            "--max-concurrent-downloads={0}".format(self.max_concurrent_downloads),
            "--max-connection-per-server=5",  # The maximum number of connections to one server for each download
            "--max-tries=5",                # Set number of tries (default 5)
            "--no-conf=true",               # Disable loading aria2.conf file.
            "--quiet=true",                 # Make aria2 quiet (no console output).
            "--remote-time=false",          # Retrieve timestamp of the remote file from the remote HTTP/FTP server
                                            # and if it is available, apply it to the local file.
            "--remove-control-file=true",   # Remove control file before download.
            "--retry-wait=0",               # Set the seconds to wait between retries (default 0)
            "--rpc-secret={0}".format(uid),        # Set RPC secret authorization token
            "--rpc-secure=false",           # RPC transport will be encrypted by SSL/TLS
            "--rpc-listen-port=6800",
            "--rpc-save-upload-metadata=false",  # Save the uploaded torrent or metalink metadata in the directory
                                                 # specified by --dir option.
            "--rpc-max-request-size=16M",   # Set max size of XML-RPC request. If aria2 detects the request is more
                                            # than SIZE bytes, it drops connection (default 2M)
            "--show-console-readout=false",  # Show console readout (default true)
            "--split=5",                    # Download a file using N connections (default 5)
            "--stop-with-process={0}".format(pid),  # Stop aria2 if Cnchi ends unexpectedly
            "--summary-interval=0",         # Set interval in seconds to output download progress summary. Setting 0
                                            # suppresses the output (default 60)
            "--timeout=60"]                 # Set timeout in seconds (default 60)

        try:
            aria2_cmd = ['/usr/bin/aria2c'] + aria2_options + ['--daemon=true']
            aria2_process = subprocess.Popen(aria2_cmd)
            aria2_process.wait()
            self.rpc_uid = "token:" + uid
        except FileNotFoundError as err:
            # aria2 is not installed
            logging.warning(_("Can't run aria2: %s"), err)
            self.rpc_uid = None
