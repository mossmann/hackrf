# - Try to find the libhackrf library
# Once done this defines
#
#  LIBHACKRF_FOUND - system has libhackrf
#  LIBHACKRF_INCLUDE_DIR - the libhackrf include directory
#  LIBHACKRF_LIBRARIES - Link these to use libhackrf

# Copyright (c) 2013  Benjamin Vernoux
#


if (LIBHACKRF_INCLUDE_DIR AND LIBHACKRF_LIBRARIES)

  # in cache already
  set(LIBHACKRF_FIND_QUIETLY TRUE)
endif()

IF (NOT WIN32)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig)
  pkg_check_modules(PC_LIBHACKRF QUIET libhackrf)
ENDIF(NOT WIN32)

FIND_PATH(LIBHACKRF_INCLUDE_DIR
  NAMES hackrf.h
  HINTS $ENV{LIBHACKRF_DIR}/include ${PC_LIBHACKRF_INCLUDEDIR}
  PATHS /usr/local/include/libhackrf /usr/include/libhackrf /usr/local/include
  /usr/include 
  /opt/local/include/libhackrf
)

set(libhackrf_library_names hackrf)

FIND_LIBRARY(LIBHACKRF_LIBRARIES
  NAMES ${libhackrf_library_names}
  HINTS $ENV{LIBHACKRF_DIR}/lib ${PC_LIBHACKRF_LIBDIR}
  PATHS /usr/local/lib /usr/lib /opt/local/lib ${PC_LIBHACKRF_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBHACKRF DEFAULT_MSG LIBHACKRF_LIBRARIES LIBHACKRF_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBHACKRF_INCLUDE_DIR LIBHACKRF_LIBRARIES)

