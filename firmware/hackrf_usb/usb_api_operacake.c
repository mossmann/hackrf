/*
 * Copyright 2016 Dominic Spill <dominicgs@gmail.com>
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

#include "usb_api_operacake.h"
#include "usb_queue.h"

#include <operacake.h>

usb_request_status_t usb_vendor_request_operacake_get_boards(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		usb_transfer_schedule_block(endpoint->in, operacake_boards, 8, NULL, NULL);
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_operacake_set_ports(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint8_t address, port_a, port_b;
	address = endpoint->setup.value & 0xFF;
	port_a = endpoint->setup.index & 0xFF;
	port_b = (endpoint->setup.index >> 8) & 0xFF;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		operacake_set_ports(address, port_a, port_b);
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

static unsigned char data[MAX_OPERACAKE_RANGES * 5];
usb_request_status_t usb_vendor_request_operacake_set_ranges(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint16_t i, freq_min, freq_max, num_ranges = 0;
	uint8_t port;
	
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		num_ranges = endpoint->setup.length  / 5;
		if((num_ranges == 0) || (num_ranges > MAX_OPERACAKE_RANGES)) {
			return USB_REQUEST_STATUS_STALL;
		}
		usb_transfer_schedule_block(endpoint->out, &data,
				endpoint->setup.length, NULL, NULL);
	} else if (stage == USB_TRANSFER_STAGE_DATA) {

		for(i=0; i<endpoint->setup.length; i+=5) {
			freq_min = data[i] << 8 | data[i+1];
			freq_max = data[i+2] << 8 | data[i+3];
			port = data[i+4];
			operacake_add_range(freq_min, freq_max, port);
		}
		usb_transfer_schedule_ack(endpoint->in);
	}
	return USB_REQUEST_STATUS_OK;
}

usb_request_status_t usb_vendor_request_operacake_gpio_test(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	uint16_t test_result;
	uint8_t address = endpoint->setup.value & 0xFF;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		test_result = gpio_test(address);
		endpoint->buffer[0] = test_result & 0xff;
		endpoint->buffer[1] = test_result >> 8;
		usb_transfer_schedule_block(endpoint->in, &endpoint->buffer, 2, NULL, NULL);
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}
