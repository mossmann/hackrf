# Copyright 2012 Jared Boone <jared@sharebrained.com>
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

cmake_minimum_required(VERSION 3.1.3)
set(CMAKE_TOOLCHAIN_FILE ../toolchain-arm-cortex-m.cmake)

project(hackrf_usb C)

include(../hackrf-common.cmake)

set(SRC_M4
	hackrf_usb.c
	"${PATH_HACKRF_FIRMWARE_COMMON}/tuning.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/streaming.c"
	sgpio_isr.c
	usb_bulk_buffer.c
	"${PATH_HACKRF_FIRMWARE_COMMON}/usb.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/usb_request.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/usb_standard_request.c"
	usb_descriptor.c
	usb_device.c
	usb_endpoint.c
	usb_api_board_info.c
	usb_api_cpld.c
	usb_api_register.c
	usb_api_spiflash.c
	usb_api_transceiver.c
	"${PATH_HACKRF_FIRMWARE_COMMON}/operacake.c"
	usb_api_operacake.c
	usb_api_sweep.c
	"${PATH_HACKRF_FIRMWARE_COMMON}/usb_queue.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/fault_handler.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/cpld_jtag.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/xapp058/lenval.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/xapp058/micro.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/xapp058/ports.c"
	"${PATH_HACKRF_FIRMWARE_COMMON}/rom_iap.c"
)

DeclareTargets()
