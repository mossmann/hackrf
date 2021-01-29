# - Try to find the freetype library
# Once done this defines
#
#  LIBUSB_FOUND - system has libusb
#  LIBUSB_INCLUDE_DIR - the libusb include directory
#  LIBUSB_LIBRARIES - Link these to use libusb

# Copyright (c) 2006, 2008  Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (LIBUSB_INCLUDE_DIR AND LIBUSB_LIBRARIES)

  # in cache already
  set(USB1_FIND_QUIETLY TRUE)
endif()


IF (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig)
  pkg_check_modules(PC_LIBUSB libusb-1.0)
ENDIF()

set(LIBUSB_LIBRARY_NAME usb-1.0)
IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  set(LIBUSB_LIBRARY_NAME usb)
ENDIF()

FIND_PATH(LIBUSB_INCLUDE_DIR libusb.h
  PATHS ${PC_LIBUSB_INCLUDEDIR} ${PC_LIBUSB_INCLUDE_DIRS}
  PATH_SUFFIXES libusb-1.0)

FIND_LIBRARY(LIBUSB_LIBRARIES NAMES ${LIBUSB_LIBRARY_NAME}
  PATHS ${PC_LIBUSB_LIBDIR} ${PC_LIBUSB_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(USB1 DEFAULT_MSG LIBUSB_LIBRARIES LIBUSB_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBUSB_INCLUDE_DIR LIBUSB_LIBRARIES)

