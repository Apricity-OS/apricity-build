project(calapm)

cmake_minimum_required(VERSION 2.8.12)

# Dirs
set(pm_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(pm_SOURCE_DIR ${pm_BASE_DIR}/src)

## ECM
find_package(ECM 0.0.13 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(KDEInstallDirs)
include(GenerateExportHeader)

# Dependencies
find_package(Qt5 CONFIG REQUIRED Core Gui Widgets)

find_package(KF5 REQUIRED COMPONENTS
    Config
    I18n
    CoreAddons
    Solid)

find_package(PkgConfig REQUIRED)
pkg_check_modules(BLKID REQUIRED blkid)
pkg_check_modules(LIBATASMART REQUIRED libatasmart)

## Parted
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${pm_BASE_DIR}/cmake/modules)
find_package(LIBPARTED REQUIRED)
if (LIBPARTED_FILESYSTEM_SUPPORT)
    add_definitions(-DLIBPARTED_FILESYSTEM_SUPPORT)
endif (LIBPARTED_FILESYSTEM_SUPPORT)

if (LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT)
    add_definitions(-DLIBPARTED_FS_RESIZE_LIBRARY_SUPPORT)
endif (LIBPARTED_FS_RESIZE_LIBRARY_SUPPORT)

# Build flags
add_definitions(-D'VERSION="0.0.0"' -DENABLE_UDISKS2 -DCALAMARES)
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-undefined -Wall -Woverloaded-virtual -std=c++11")
endif()

add_subdirectory(fatlabel)

# sources
set(core_DIR ${pm_SOURCE_DIR}/core)
set(core_SRCS
    ${core_DIR}/device.cpp
    ${core_DIR}/copysourcedevice.cpp
    ${core_DIR}/copytargetdevice.cpp
    ${core_DIR}/partition.cpp
    ${core_DIR}/partitionalignment.cpp
    ${core_DIR}/partitionnode.cpp
    ${core_DIR}/partitionrole.cpp
    ${core_DIR}/partitiontable.cpp
    ${core_DIR}/smartattribute.cpp
    ${core_DIR}/smartstatus.cpp
    )
kconfig_add_kcfg_files(core_SRCS ${pm_SOURCE_DIR}/config.kcfgc)

set(fs_DIR ${pm_SOURCE_DIR}/fs)
file(GLOB fs_SRCS ${fs_DIR}/*.cpp)

set(backend_DIR ${pm_SOURCE_DIR}/backend)
set(backend_SRCS
    ${backend_DIR}/corebackend.cpp
    ${backend_DIR}/corebackenddevice.cpp
    ${backend_DIR}/corebackendpartition.cpp
    ${backend_DIR}/corebackendpartitiontable.cpp
    )

set(plugins_DIR ${pm_SOURCE_DIR}/plugins)
file(GLOB dummy_SRCS ${plugins_DIR}/dummy/*.cpp)
file(GLOB libparted_SRCS ${plugins_DIR}/libparted/*.cpp)

set(util_DIR ${pm_SOURCE_DIR}/util)
set(util_SRCS
    ${util_DIR}/capacity.cpp
    ${util_DIR}/externalcommand.cpp
    ${util_DIR}/globallog.cpp
    ${util_DIR}/report.cpp
    )

set(gui_DIR ${pm_SOURCE_DIR}/gui)
set(gui_SRCS
    ${gui_DIR}/partresizerwidget.cpp
    ${gui_DIR}/partwidget.cpp
    ${gui_DIR}/partwidgetbase.cpp
    )

include(kio/KMountPointChecks.cmake)
configure_file(kio/config-kmountpoint.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-kmountpoint.h)
set(kio_SRCS
    kio/kdiskfreespaceinfo.cpp
    kio/kmountpoint.cpp
    )

set(calapm_SRCS
    CalaPM.cpp
    helpers.cpp
    corebackendmanager.cpp
    ${core_SRCS}
    ${backend_SRCS}
    ${fs_SRCS}
    ${util_SRCS}
    ${gui_SRCS}
    ${dummy_SRCS}
    ${libparted_SRCS}
    ${kio_SRCS}
    )

# Building
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kio
    ${pm_SOURCE_DIR}
    ${pm_BASE_DIR}/lib # For fatlabel
    )
add_library(calapm SHARED ${calapm_SRCS})
generate_export_header(calapm EXPORT_FILE_NAME libpartitionmanager_export.h)

target_link_libraries(calapm
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        KF5::ConfigCore
        KF5::ConfigGui
        KF5::CoreAddons
        KF5::I18n
        KF5::Solid
        libfatlabel
        ${BLKID_LIBRARIES}
        ${LIBATASMART_LIBRARIES}
        ${LIBPARTED_LIBS}
    )
set_target_properties(calapm PROPERTIES AUTOMOC TRUE)

target_include_directories(calapm
    PUBLIC
        ${pm_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR} # For CalaPM.h
        ${pm_BASE_DIR}/lib # For fatlabel
        ${CMAKE_CURRENT_BINARY_DIR} # For libpartitionmanager_export.h
    )

# Install
install(TARGETS calapm
    ${INSTALL_TARGETS_DEFAULT_ARGS}
    )
