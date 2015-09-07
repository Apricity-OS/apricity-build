#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  installation_process.py
#
#  Copyright 2014 Markus M. May
#  Copyright 2014 Antergos
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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

import os, sys
import logging
from mock import Mock

logging.basicConfig(level=10)
logger = logging.getLogger(__name__)

parentdir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parentdir)
sys.path.append(os.path.join(parentdir, 'src'))

logger.debug("parentdir: %s" % parentdir)

from installation_process import InstallationProcess

# set needed config options
settings = dict()
settings['data'] = os.path.join(parentdir, 'data')

# create mock object to test just one method and not the __init__
mobject = Mock(InstallationProcess)

mobject.settings = settings
mobject.dest_dir = '/tmp'
mobject.arch = 'x86_64'

mobject.write_file = InstallationProcess.write_file.__get__(mobject)

InstallationProcess.create_pacman_conf_file(mobject)

logger.debug('Done')

assert os.path.isfile('/tmp/pacman.conf')
