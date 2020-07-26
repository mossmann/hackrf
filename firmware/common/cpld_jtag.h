/*
 * Copyright 2013 Michael Ossmann <mike@ossmann.com>
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

#ifndef __CPLD_JTAG_H__
#define __CPLD_JTAG_H__

#include <stdint.h>

#include "gpio.h"

typedef struct jtag_gpio_t {
	gpio_t gpio_tms;
	gpio_t gpio_tck;
	gpio_t gpio_tdi;
	gpio_t gpio_tdo;
} jtag_gpio_t;

typedef struct jtag_t {
	jtag_gpio_t* const gpio;
} jtag_t;

typedef void (*refill_buffer_cb)(void);

void cpld_jtag_release(jtag_t* const jtag);

/* Return 0 if success else return error code see xsvfExecute() see micro.h.
 *
 * We expect the buffer to be initially full of data. After the entire
 * contents of the buffer has been streamed to the CPLD the given
 * refill_buffer callback will be called. */
int cpld_jtag_program(
		jtag_t* const jtag,
        const uint32_t buffer_length,
        unsigned char* const buffer,
        refill_buffer_cb refill
);
unsigned char cpld_jtag_get_next_byte(void);

#endif//__CPLD_JTAG_H__
