# Copyright 2012 Jared Boone
# Copyright 2013 Benjamin Vernoux
#
# This file is part of HackRF.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

# Based heavily upon the libftdi cmake setup.



SET(TOOLS
	hackrf_transfer
	hackrf_spiflash
	hackrf_cpldjtag
	hackrf_info
	hackrf_debug
	hackrf_clock
	hackrf_sweep
	hackrf_operacake
)

set(TOOLS_LINK_LIBS ${Math_LIBRARY} fftw3 hackrf)

if(MSVC)
	# use getopt support library
	add_library(libgetopt_static STATIC
	    ../getopt/getopt.c
	)

	LIST(APPEND TOOLS_LINK_LIBS libgetopt_static)
endif()

foreach(tool ${TOOLS})
	add_executable(${tool} ${tool}.c)
	target_link_libraries(${tool} ${TOOLS_LINK_LIBS})
	install(TARGETS ${tool} RUNTIME DESTINATION ${INSTALL_DEFAULT_BINDIR})
endforeach(tool)
