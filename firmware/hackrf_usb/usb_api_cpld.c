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

#include "usb_api_cpld.h"

#include <hackrf_core.h>
#include <cpld_jtag.h>
#include <usb_queue.h>

#include "usb_endpoint.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

volatile bool start_cpld_update = false;
uint8_t cpld_xsvf_buffer[512];
volatile bool cpld_wait = false;

static void cpld_buffer_refilled(void* user_data, unsigned int length)
{
	(void)user_data;
	(void)length;
	cpld_wait = false;
}

static void refill_cpld_buffer(void)
{
	cpld_wait = true;
	usb_transfer_schedule(
		&usb_endpoint_bulk_out,
		cpld_xsvf_buffer,
		sizeof(cpld_xsvf_buffer),
		cpld_buffer_refilled,
		NULL
		);

	// Wait until transfer finishes
	while (cpld_wait);
}

void cpld_update(void)
{
	#define WAIT_LOOP_DELAY (6000000)
	int i;
	int error;

	usb_queue_flush_endpoint(&usb_endpoint_bulk_in);
	usb_queue_flush_endpoint(&usb_endpoint_bulk_out);

	refill_cpld_buffer();

	error = cpld_jtag_program(&jtag_cpld, sizeof(cpld_xsvf_buffer),
				  cpld_xsvf_buffer,
				  refill_cpld_buffer);
	if(error == 0)
	{
		/* blink LED1, LED2, and LED3 on success */
		while (1)
		{
			led_on(LED1);
			led_on(LED2);
			led_on(LED3);
			for (i = 0; i < WAIT_LOOP_DELAY; i++)  /* Wait a bit. */
				__asm__("nop");
			led_off(LED1);
			led_off(LED2);
			led_off(LED3);
			for (i = 0; i < WAIT_LOOP_DELAY; i++)  /* Wait a bit. */
				__asm__("nop");
		}
	}else
	{
		/* LED3 (Red) steady on error */
		led_on(LED3);
		while (1);
	}
}
