# Copyright (C) 2008,2012 by Volker Lanz <vl@fidra.de>
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

############################################

file(GLOB partitionmanagerprivate_SRCS
	backend/*.cpp
	core/*.cpp
	util/*.cpp
	ops/*.cpp
	jobs/*.cpp
	fs/*.cpp
	gui/*.cpp
	config/*.cpp
)

file(GLOB partitionmanagerprivate_UIFILES config/*.ui gui/*.ui)

qt5_wrap_ui(partitionmanagerprivate_SRCS ${partitionmanagerprivate_UIFILES})

kconfig_add_kcfg_files(partitionmanagerprivate_SRCS config.kcfgc)
install(FILES config.kcfg DESTINATION ${KCFG_INSTALL_DIR})


add_library(partitionmanagerprivate SHARED ${partitionmanagerprivate_SRCS})
generate_export_header(partitionmanagerprivate EXPORT_FILE_NAME libpartitionmanager_export.h)
target_link_libraries( partitionmanagerprivate libfatlabel
    ${UUID_LIBRARIES} ${BLKID_LIBRARIES} ${LIBATASMART_LIBRARIES}
    KF5::I18n
    KF5::IconThemes
    KF5::JobWidgets
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Service
    KF5::Solid
    KF5::XmlGui
    KF5::WidgetsAddons
)

install(TARGETS partitionmanagerprivate ${INSTALL_TARGETS_DEFAULT_ARGS})

############################################

file(GLOB partitionmanager_SRCS	main.cpp)

add_executable(partitionmanager ${partitionmanager_SRCS})

target_link_libraries(partitionmanager partitionmanagerprivate)

install(TARGETS partitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/partitionmanager.appdata.xml DESTINATION share/appdata)
install(FILES gui/partitionmanagerui.rc DESTINATION ${DATA_INSTALL_DIR}/partitionmanager)
install(PROGRAMS partitionmanager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

############################################

add_subdirectory(plugins)

