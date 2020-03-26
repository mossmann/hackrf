/*
 * Copyright 2012 Jared Boone
 * Copyright 2013 Benjamin Vernoux
 *
 * This file is part of HackRF.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __USB_API_TRANSCEIVER_H__
#define __USB_API_TRANSCEIVER_H__

#include <hackrf_core.h>
#include <usb_type.h>
#include <usb_request.h>

void set_hw_sync_mode(const hw_sync_mode_t new_hw_sync_mode);
usb_request_status_t usb_vendor_request_set_transceiver_mode(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_baseband_filter_bandwidth(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_freq(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_sample_rate_frac(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_amp_enable(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_lna_gain(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_vga_gain(
	usb_endpoint_t* const endpoint,	const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_txvga_gain(
	usb_endpoint_t* const endpoint,	const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_antenna_enable(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_freq_explicit(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage);
usb_request_status_t usb_vendor_request_set_hw_sync_mode(
	usb_endpoint_t* const endpoint,	const usb_transfer_stage_t stage);

transceiver_mode_t transceiver_mode(void);
void set_transceiver_mode(const transceiver_mode_t new_transceiver_mode);
bool should_prepare_buffer_for_tx_or_rx(void);

#endif/*__USB_API_TRANSCEIVER_H__*/
