/*
 * Copyright 2012 Michael Ossmann <mike@ossmann.com>
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
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

#include "i2c_lpc.h"

#include <libopencm3/lpc43xx/i2c.h>

/* FIXME return i2c0 status from each function */

void i2c_lpc_start(i2c_bus_t* const bus, const void* const _config) {
	const i2c_lpc_config_t* const config = _config;

	const uint32_t port = (uint32_t)bus->obj;
	i2c_init(port, config->duty_cycle_count);
}

void i2c_lpc_stop(i2c_bus_t* const bus) {
	const uint32_t port = (uint32_t)bus->obj;
	i2c_disable(port);
}

void i2c_lpc_transfer(i2c_bus_t* const bus,
	const uint_fast8_t slave_address,
	const uint8_t* const data_tx, const size_t count_tx,
	uint8_t* const data_rx, const size_t count_rx
) {
	const uint32_t port = (uint32_t)bus->obj;
	size_t i;
	bool ack = false;
	if (data_tx && (count_tx > 0)) {
		i2c_tx_start(port);
		i2c_tx_byte(port, (slave_address << 1) | I2C_WRITE);
		for(i=0; i<count_tx; i++) {
			i2c_tx_byte(port, data_tx[i]);
		}
	}

	if (data_rx && (count_rx > 0)) {
		i2c_tx_start(port);
		i2c_tx_byte(port, (slave_address << 1) | I2C_READ);
		for(i=0; i<count_rx; i++) {
			/* ACK each byte except the last */
			ack = (i!=count_rx-1);
			data_rx[i] = i2c_rx_byte(port, ack);
		}
	}

	i2c_stop(port);
}
