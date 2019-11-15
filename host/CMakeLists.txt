#top dir cmake project for libhackrf + tools

cmake_minimum_required(VERSION 3.1)
project(HackRF LANGUAGES C)

# standard policies for CMake 3.1
cmake_policy(VERSION 3.1)

list(APPEND CMAKE_MODULE_PATH 
	${CMAKE_CURRENT_SOURCE_DIR}/cmake
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/amber-cmake)

include(LibraryUtils)

# Compilation flags
# -----------------------------------------------------

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

	set(CMAKE_C_FLAGS_RELEASE "-O3")
	set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
		
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")

	set(CMAKE_C_FLAGS_RELEASE "/O2")
	set(CMAKE_C_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od")
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /W3")

	# disable verbose security warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)

elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")

	set(CMAKE_C_FLAGS_RELEASE "-O3")
	set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()

	message(WARNING "Unknown compiler ${CMAKE_C_COMPILER_ID}, don't know how to set CFLAGS")
endif()

# language standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS FALSE)

# endianness
include(TestBigEndian)
test_big_endian(BIGENDIAN)
if(BIGENDIAN)
	 add_definitions(-DHACKRF_BIG_ENDIAN)
endif()

# Find dependencies
# -----------------------------------------------------
message(STATUS "Checking dependencies...")

# fftw3
find_package(FFTW COMPONENTS SinglePrecision REQUIRED)

import_libraries(fftw3 LIBRARIES ${FFTW_LIBRARIES} INCLUDES ${FFTW_INCLUDES})
set_property(TARGET fftw3 PROPERTY INTERFACE_COMPILE_OPTIONS ${FFTW_COMPILE_OPTIONS})


# libusb1.0
find_package(USB1 REQUIRED)
import_library(libusb ${LIBUSB_LIBRARIES} ${LIBUSB_INCLUDE_DIR})

# pthread
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	# tell FindThreads to use MSVC pthread library
	set(THREADS_USE_PTHREADS_WIN32 TRUE)
endif()
find_package(Threads REQUIRED)
import_libraries(pthread LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

# Options
# -----------------------------------------------------
set(INSTALL_DEFAULT_BINDIR "bin" CACHE STRING "Appended to CMAKE_INSTALL_PREFIX")


# Subdirectories
# -----------------------------------------------------
add_subdirectory(libhackrf)
add_subdirectory(hackrf-tools)

########################################################################
# Create uninstall target
########################################################################

configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)


add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)
