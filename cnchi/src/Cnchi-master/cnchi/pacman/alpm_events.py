#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  alpm_events.py
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

""" ALPM Events from alpm.h """

# Dependencies will be computed for a package.
ALPM_EVENT_CHECKDEPS_START = 1
# Dependencies were computed for a package.
ALPM_EVENT_CHECKDEPS_DONE = 2
# File conflicts will be computed for a package.
ALPM_EVENT_FILECONFLICTS_START = 3
# File conflicts were computed for a package.
ALPM_EVENT_FILECONFLICTS_DONE = 4
# Dependencies will be resolved for target package.
ALPM_EVENT_RESOLVEDEPS_START = 5
# Dependencies were resolved for target package.
ALPM_EVENT_RESOLVEDEPS_DONE = 6
# Inter-conflicts will be checked for target package.
ALPM_EVENT_INTERCONFLICTS_START = 7
# Inter-conflicts were checked for target package.
ALPM_EVENT_INTERCONFLICTS_DONE = 8
# Package will be installed/upgraded/downgraded/re-installed/removed
ALPM_EVENT_PACKAGE_OPERATION_START = 9
# Package was installed/upgraded/downgraded/re-installed/removed
ALPM_EVENT_PACKAGE_OPERATION_DONE = 10
# Target package's integrity will be checked.
ALPM_EVENT_INTEGRITY_START = 11
# Target package's integrity was checked.
ALPM_EVENT_INTEGRITY_DONE = 12
# Target package will be loaded.
ALPM_EVENT_LOAD_START = 13
# Target package is finished loading.
ALPM_EVENT_LOAD_DONE = 14
# Target delta's integrity will be checked.
ALPM_EVENT_DELTA_INTEGRITY_START = 15
# Target delta's integrity was checked.
ALPM_EVENT_DELTA_INTEGRITY_DONE = 16
# Deltas will be applied to packages.
ALPM_EVENT_DELTA_PATCHES_START = 17
# Deltas were applied to packages.
ALPM_EVENT_DELTA_PATCHES_DONE = 18
# Delta patch will be applied to target package.
# The filename of the package and the filename of the patch is passed to the callback.
ALPM_EVENT_DELTA_PATCH_START = 19
# Delta patch was applied to target package.
ALPM_EVENT_DELTA_PATCH_DONE = 20
# Delta patch failed to apply to target package.
ALPM_EVENT_DELTA_PATCH_FAILED = 21
# Scriptlet has printed information.
# A line of text is passed to the callback.
ALPM_EVENT_SCRIPTLET_INFO = 22
# Files will be downloaded from a repository.
# The repository's tree name is passed to the callback.
ALPM_EVENT_RETRIEVE_START = 23
# Files will be downloaded from a repository.
# The repository's tree name is passed to the callback.
ALPM_EVENT_RETRIEVE_DONE = 24
# Not all files were successfully downloaded from a repository.
ALPM_EVENT_RETRIEVE_FAILED = 25
# A file will be downloaded from a repository
ALPM_EVENT_PKGDOWNLOAD_START = 26
# A file was downloaded from a repository
ALPM_EVENT_PKGDOWNLOAD_DONE = 27
# A file failed to be downloaded from a repository
ALPM_EVENT_PKGDOWNLOAD_FAILED = 28
# Disk space usage will be computed for a package
ALPM_EVENT_DISKSPACE_START = 29
# Disk space usage was computed for a package
ALPM_EVENT_DISKSPACE_DONE = 30
# An optdepend for another package is being removed
# The requiring package and its dependency are passed to the callback
ALPM_EVENT_OPTDEP_REMOVAL = 31
# A configured repository database is missing
ALPM_EVENT_DATABASE_MISSING = 32
# Checking keys used to create signatures are in keyring.
ALPM_EVENT_KEYRING_START = 33
# Keyring checking is finished.
ALPM_EVENT_KEYRING_DONE = 34
# Downloading missing keys into keyring.
ALPM_EVENT_KEY_DOWNLOAD_START = 35
# Key downloading is finished.
ALPM_EVENT_KEY_DOWNLOAD_DONE = 36
# A .pacnew file was created
ALPM_EVENT_PACNEW_CREATED = 37
# A .pacsave file was created
ALPM_EVENT_PACSAVE_CREATED = 38
# A .pacorig file was created
ALPM_EVENT_PACORIG_CREATED = 39

# Package (to be) installed. (No oldpkg)
ALPM_PACKAGE_INSTALL = 1,
# Package (to be) upgraded
ALPM_PACKAGE_UPGRADE = 2
# Package (to be) re-installed.
ALPM_PACKAGE_REINSTALL = 3
# Package (to be) downgraded.
ALPM_PACKAGE_DOWNGRADE = 4
# Package (to be) removed. (No newpkg)
ALPM_PACKAGE_REMOVE = 5

# Progress
ALPM_PROGRESS_ADD_START = 0
ALPM_PROGRESS_UPGRADE_START = 1
ALPM_PROGRESS_DOWNGRADE_START = 2
ALPM_PROGRESS_REINSTALL_START = 3
ALPM_PROGRESS_REMOVE_START = 4
ALPM_PROGRESS_CONFLICTS_START = 5
ALPM_PROGRESS_DISKSPACE_START = 6
ALPM_PROGRESS_INTEGRITY_START = 7
ALPM_PROGRESS_LOAD_START = 8
ALPM_PROGRESS_KEYRING_START = 9
