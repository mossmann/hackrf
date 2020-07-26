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

#ifndef __OPERACAKE_H
#define __OPERACAKE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "i2c_bus.h"

#define OPERACAKE_PA1 0
#define OPERACAKE_PA2 1
#define OPERACAKE_PA3 2
#define OPERACAKE_PA4 3

#define OPERACAKE_PB1 4
#define OPERACAKE_PB2 5
#define OPERACAKE_PB3 6
#define OPERACAKE_PB4 7

#define MAX_OPERACAKE_RANGES 8

/* Up to 8 Operacake boards can be used with one HackRF */
extern uint8_t operacake_boards[8];

uint8_t operacake_init(void);
uint8_t operacake_set_ports(uint8_t address, uint8_t PA, uint8_t PB);
uint8_t operacake_add_range(uint16_t freq_min, uint16_t freq_max, uint8_t port);
uint8_t operacake_set_range(uint32_t freq_mhz);
uint16_t gpio_test(uint8_t address);

#ifdef __cplusplus
}
#endif

#endif /* __OPERACAKE_H */
