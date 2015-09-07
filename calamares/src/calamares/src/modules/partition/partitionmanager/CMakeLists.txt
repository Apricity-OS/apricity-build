# Copyright (C) 2008 by Volker Lanz <vl@fidra.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

project(partitionmanager)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(QT_MIN_VERSION "5.3.0")
set(VERSION_MAJOR "1")
set(VERSION_MINOR "1")
set(VERSION_RELEASE "60")
set(VERSION_SUFFIX "")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}${VERSION_SUFFIX})
add_definitions(-D'VERSION="${VERSION}"')

find_package(ECM 0.0.13 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
include(ECMInstallIcons)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)
include(GenerateExportHeader)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
  Core
  DBus
  Widgets
  Sql
  Test
)

# Load the frameworks we need
find_package(KF5 CONFIG REQUIRED COMPONENTS
  Config
  DocTools
  FrameworkIntegration
  GuiAddons
  I18n
  IconThemes
  Init
  JobWidgets
  KIO
  Service
  Parts
  TextEditor
  WindowSystem
  XmlGui
  WidgetsAddons
)

# use sane compile flags
add_definitions(
  -fexceptions
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
)

find_package(MSGFMT REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(BLKID REQUIRED blkid)
pkg_check_modules(LIBATASMART REQUIRED libatasmart)

option(ENABLE_UDISKS2 "Use UDisks2 backend." OFF)
if (ENABLE_UDISKS2)
	add_definitions(-DENABLE_UDISKS2)
endif (ENABLE_UDISKS2)

include_directories(${Qt5Core_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS} lib/ src/)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(icons)

message(STATUS "KDE Partition Manager ${VERSION} will be built for install into ${CMAKE_INSTALL_PREFIX}")

add_custom_target(docs
	COMMAND doxygen Doxyfile
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
