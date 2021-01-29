#
# Copyright (c) 2012, Jared Boone <jared@sharebrained.com>
# Copyright (c) 2013, Benjamin Vernoux <titanmkd@gmail.com>
# Copyright (c) 2013, Michael Ossmann <mike@ossmann.com>
# 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 
#     Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#     Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the 
# 	documentation and/or other materials provided with the distribution.
#     Neither the name of Great Scott Gadgets nor the names of its contributors may be used to endorse or promote products derived from this software
# 	without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Based heavily upon the libftdi cmake setup.

# Targets
set(LIBHACKRF_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/hackrf.c)
set(LIBHACKRF_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/hackrf.h)

# Dynamic library
add_library(hackrf SHARED ${LIBHACKRF_SOURCES})
set_target_properties(hackrf PROPERTIES VERSION ${libhackrf_VERSION} SOVERSION 0)

# Static library
add_library(hackrf-static STATIC ${LIBHACKRF_SOURCES})
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  # avoid .lib files with identical names
	set_target_properties(hackrf-static PROPERTIES OUTPUT_NAME "hackrf_static")
else()
	set_target_properties(hackrf-static PROPERTIES OUTPUT_NAME "hackrf")
endif()

# prevent cleaning one target from cleaning the other
set_target_properties(hackrf PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(hackrf-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# Dependencies
target_link_libraries(hackrf libusb pthread)
target_link_libraries(hackrf-static libusb pthread)

# add DLL flag to dynamic version
target_compile_definitions(hackrf-static PUBLIC hackrf_STATIC)

# add interface include directories
target_include_directories(hackrf PUBLIC .)
target_include_directories(hackrf-static PUBLIC .)

# install DLL to bin and link library to lib
install(TARGETS hackrf
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         COMPONENT sharedlibs
         )
install(TARGETS hackrf-static
       DESTINATION bin
       COMPONENT staticlibs
       )
install(FILES ${LIBHACKRF_HEADERS}
       DESTINATION include/${PROJECT_NAME}
       COMPONENT headers
       )


