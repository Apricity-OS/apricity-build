#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "calamares" for configuration "Release"
set_property(TARGET calamares APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(calamares PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcalamares.so.1.1.2"
  IMPORTED_SONAME_RELEASE "libcalamares.so.1.1.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS calamares )
list(APPEND _IMPORT_CHECK_FILES_FOR_calamares "${_IMPORT_PREFIX}/lib/libcalamares.so.1.1.2" )

# Import target "calamaresui" for configuration "Release"
set_property(TARGET calamaresui APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(calamaresui PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcalamaresui.so.1.1.2"
  IMPORTED_SONAME_RELEASE "libcalamaresui.so.1.1.2"
  )

list(APPEND _IMPORT_CHECK_TARGETS calamaresui )
list(APPEND _IMPORT_CHECK_FILES_FOR_calamaresui "${_IMPORT_PREFIX}/lib/libcalamaresui.so.1.1.2" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
