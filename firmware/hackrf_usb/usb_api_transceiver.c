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

#include "usb_api_transceiver.h"

#include "hackrf_ui.h"
#include <libopencm3/cm3/vector.h>
#include "usb_bulk_buffer.h"

#include "usb_api_cpld.h" // Remove when CPLD update is handled elsewhere

#include <max2837.h>
#include <rf_path.h>
#include <tuning.h>
#include <streaming.h>
#include <usb.h>
#include <usb_queue.h>

#include <stddef.h>

#include "usb_endpoint.h"

typedef struct {
	uint32_t freq_mhz;
	uint32_t freq_hz;
} set_freq_params_t;

set_freq_params_t set_freq_params;

struct set_freq_explicit_params {
	uint64_t if_freq_hz; /* intermediate frequency */
	uint64_t lo_freq_hz; /* front-end local oscillator frequency */
	uint8_t path;        /* image rejection filter path */
};

struct set_freq_explicit_params explicit_params;

typedef struct {
	uint32_t freq_hz;
	uint32_t divider;
} set_sample_r_params_t;

set_sample_r_params_t set_sample_r_params;

usb_request_status_t usb_vendor_request_set_baseband_filter_bandwidth(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage
) {
	if( stage == USB_TRANSFER_STAGE_SETUP ) {
		const uint32_t bandwidth = (endpoint->setup.index << 16) | endpoint->setup.value;
		if( baseband_filter_bandwidth_set(bandwidth) ) {
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		}
		return USB_REQUEST_STATUS_STALL;
	} else {
		return USB_REQUEST_STATUS_OK;
	}
}

usb_request_status_t usb_vendor_request_set_freq(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage) 
{
	if (stage == USB_TRANSFER_STAGE_SETUP) 
	{
		usb_transfer_schedule_block(endpoint->out, &set_freq_params, sizeof(set_freq_params_t),
					    NULL, NULL);
		return USB_REQUEST_STATUS_OK;
	} else if (stage == USB_TRANSFER_STAGE_DATA) 
	{
		const uint64_t freq = set_freq_params.freq_mhz * 1000000ULL + set_freq_params.freq_hz;
		if( set_freq(freq) ) 
		{
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		}
		return USB_REQUEST_STATUS_STALL;
	} else
	{
		return USB_REQUEST_STATUS_OK;
	}
}

usb_request_status_t usb_vendor_request_set_sample_rate_frac(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage) 
{
	if (stage == USB_TRANSFER_STAGE_SETUP) 
	{
                usb_transfer_schedule_block(endpoint->out, &set_sample_r_params, sizeof(set_sample_r_params_t),
					    NULL, NULL);
		return USB_REQUEST_STATUS_OK;
	} else if (stage == USB_TRANSFER_STAGE_DATA) 
	{
		if( sample_rate_frac_set(set_sample_r_params.freq_hz * 2, set_sample_r_params.divider ) )
		{
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		}
		return USB_REQUEST_STATUS_STALL;
	} else
	{
		return USB_REQUEST_STATUS_OK;
	}
}

usb_request_status_t usb_vendor_request_set_amp_enable(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		switch (endpoint->setup.value) {
		case 0:
			rf_path_set_lna(&rf_path, 0);
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		case 1:
			rf_path_set_lna(&rf_path, 1);
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		default:
			return USB_REQUEST_STATUS_STALL;
		}
	} else {
		return USB_REQUEST_STATUS_OK;
	}
}

usb_request_status_t usb_vendor_request_set_lna_gain(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage)
{
	if( stage == USB_TRANSFER_STAGE_SETUP ) {
			const uint8_t value = max2837_set_lna_gain(&max2837, endpoint->setup.index);
			endpoint->buffer[0] = value;
			if(value) hackrf_ui()->set_bb_lna_gain(endpoint->setup.index);
			usb_transfer_schedule_block(endpoint->in, &endpoint->buffer, 1,
						    NULL, NULL);
			usb_transfer_schedule_ack(endpoint->out);
			return USB_REQUEST_STATUS_OK;
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_set_vga_gain(
	usb_endpoint_t* const endpoint,	const usb_transfer_stage_t stage)
{
	if( stage == USB_TRANSFER_STAGE_SETUP ) {
			const uint8_t value = max2837_set_vga_gain(&max2837, endpoint->setup.index);
			endpoint->buffer[0] = value;
			if(value) hackrf_ui()->set_bb_vga_gain(endpoint->setup.index);
			usb_transfer_schedule_block(endpoint->in, &endpoint->buffer, 1,
						    NULL, NULL);
			usb_transfer_schedule_ack(endpoint->out);
			return USB_REQUEST_STATUS_OK;
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_set_txvga_gain(
	usb_endpoint_t* const endpoint,	const usb_transfer_stage_t stage)
{
	if( stage == USB_TRANSFER_STAGE_SETUP ) {
			const uint8_t value = max2837_set_txvga_gain(&max2837, endpoint->setup.index);
			endpoint->buffer[0] = value;
			if(value) hackrf_ui()->set_bb_tx_vga_gain(endpoint->setup.index);
			usb_transfer_schedule_block(endpoint->in, &endpoint->buffer, 1,
						    NULL, NULL);
			usb_transfer_schedule_ack(endpoint->out);
			return USB_REQUEST_STATUS_OK;
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_set_antenna_enable(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		switch (endpoint->setup.value) {
		case 0:
			rf_path_set_antenna(&rf_path, 0);
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		case 1:
			rf_path_set_antenna(&rf_path, 1);
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		default:
			return USB_REQUEST_STATUS_STALL;
		}
	} else {
		return USB_REQUEST_STATUS_OK;
	}
}

usb_request_status_t usb_vendor_request_set_freq_explicit(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->out, &explicit_params,
				sizeof(struct set_freq_explicit_params), NULL, NULL);
		return USB_REQUEST_STATUS_OK;
	} else if (stage == USB_TRANSFER_STAGE_DATA) {
		if (set_freq_explicit(explicit_params.if_freq_hz,
				explicit_params.lo_freq_hz, explicit_params.path)) {
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		}
		return USB_REQUEST_STATUS_STALL;
	} else {
		return USB_REQUEST_STATUS_OK;
	}
}

static volatile transceiver_mode_t _transceiver_mode = TRANSCEIVER_MODE_OFF;
static volatile hw_sync_mode_t _hw_sync_mode = HW_SYNC_MODE_OFF;
static volatile bool _should_prepare_buffer_for_tx_or_rx = false;

void set_hw_sync_mode(const hw_sync_mode_t new_hw_sync_mode) {
	_hw_sync_mode = new_hw_sync_mode;
}

transceiver_mode_t transceiver_mode(void) {
	return _transceiver_mode;
}

void set_transceiver_mode(const transceiver_mode_t new_transceiver_mode) {
	baseband_streaming_disable(&sgpio_config);

	usb_endpoint_flush(&usb_endpoint_bulk_in);
	usb_endpoint_flush(&usb_endpoint_bulk_out);

	_transceiver_mode = new_transceiver_mode;
	
	if( _transceiver_mode == TRANSCEIVER_MODE_RX ) {
		led_off(LED3);
		led_on(LED2);
		rf_path_set_direction(&rf_path, RF_PATH_DIRECTION_RX);
		usb_bulk_buffer_tx = false;
	} else if (_transceiver_mode == TRANSCEIVER_MODE_TX) {
		led_off(LED2);
		led_on(LED3);
		rf_path_set_direction(&rf_path, RF_PATH_DIRECTION_TX);
		usb_bulk_buffer_tx = true;
	} else {
		led_off(LED2);
		led_off(LED3);
		rf_path_set_direction(&rf_path, RF_PATH_DIRECTION_OFF);
		usb_bulk_buffer_tx = false;
	}


	if( _transceiver_mode != TRANSCEIVER_MODE_OFF ) {
		activate_best_clock_source();

        hw_sync_enable(_hw_sync_mode);
	}

	_should_prepare_buffer_for_tx_or_rx = true;
}

bool should_prepare_buffer_for_tx_or_rx(void) {
	bool ret = _should_prepare_buffer_for_tx_or_rx;
	_should_prepare_buffer_for_tx_or_rx = false;
	return ret;
}

usb_request_status_t usb_vendor_request_set_transceiver_mode(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage)
{
	if( stage == USB_TRANSFER_STAGE_SETUP ) {
		switch( endpoint->setup.value ) {
		case TRANSCEIVER_MODE_OFF:
		case TRANSCEIVER_MODE_RX:
		case TRANSCEIVER_MODE_TX:
			set_transceiver_mode(endpoint->setup.value);
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		case TRANSCEIVER_MODE_CPLD_UPDATE:
			start_cpld_update = true;
			usb_transfer_schedule_ack(endpoint->in);
			return USB_REQUEST_STATUS_OK;
		default:
			return USB_REQUEST_STATUS_STALL;
		}
	} else {
		return USB_REQUEST_STATUS_OK;
	}
}

usb_request_status_t usb_vendor_request_set_hw_sync_mode(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage)
{
	if( stage == USB_TRANSFER_STAGE_SETUP ) {
		set_hw_sync_mode(endpoint->setup.value);
		usb_transfer_schedule_ack(endpoint->in);
		return USB_REQUEST_STATUS_OK;
	} else {
		return USB_REQUEST_STATUS_OK;
	}
}
