# Copyright (C) 2010 by Volker Lanz <vl@fidra.de>
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -std=c99 -Wno-sign-compare -Wno-type-limits -fno-strict-aliasing -fvisibility=hidden")

file(GLOB libfatlabel_SRCS
	*.c
)

include_directories(.)
add_definitions(-D_LARGEFILE64_SOURCE -D_GNU_SOURCE)

############################################

add_library(libfatlabel SHARED ${libfatlabel_SRCS})
target_link_libraries(libfatlabel)

set_target_properties(libfatlabel PROPERTIES
	PREFIX ""
	OUTPUT_NAME "libpartitionmanagerfatlabel"
)

install(TARGETS libfatlabel ${INSTALL_TARGETS_DEFAULT_ARGS})

############################################

option(MLABEL_EXECUTABLE "Build a test executable for libfatlabel" OFF)
mark_as_advanced(MLABEL_EXECUTABLE)

if (MLABEL_EXECUTABLE)
	set(app_SRCS app/main.c)
	add_executable(fatlabel ${app_SRCS})
	target_link_libraries(fatlabel libfatlabel)
	install(TARGETS fatlabel ${INSTALL_TARGETS_DEFAULT_ARGS})
endif (MLABEL_EXECUTABLE)

