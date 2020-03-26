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

#include <stddef.h>

#include <libopencm3/lpc43xx/ipc.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/rgu.h>

#include <streaming.h>

#include "tuning.h"

#include "usb.h"
#include "usb_standard_request.h"

#include <rom_iap.h>
#include "usb_descriptor.h"

#include "usb_device.h"
#include "usb_endpoint.h"
#include "usb_api_board_info.h"
#include "usb_api_cpld.h"
#include "usb_api_register.h"
#include "usb_api_spiflash.h"
#include "usb_api_operacake.h"
#include "operacake.h"
#include "usb_api_sweep.h"
#include "usb_api_transceiver.h"
#include "usb_api_ui.h"
#include "usb_bulk_buffer.h"
#include "cpld_xc2c.h"
#include "portapack.h"
 
#include "hackrf_ui.h"

extern uint32_t __m0_start__;
extern uint32_t __m0_end__;
extern uint32_t __ram_m0_start__;

static usb_request_handler_fn vendor_request_handler[] = {
	NULL,
	usb_vendor_request_set_transceiver_mode,
	usb_vendor_request_write_max2837,
	usb_vendor_request_read_max2837,
	usb_vendor_request_write_si5351c,
	usb_vendor_request_read_si5351c,
	usb_vendor_request_set_sample_rate_frac,
	usb_vendor_request_set_baseband_filter_bandwidth,
#ifdef RAD1O
	NULL, // write_rffc5071 not used
	NULL, // read_rffc5071 not used
#else
	usb_vendor_request_write_rffc5071,
	usb_vendor_request_read_rffc5071,
#endif
	usb_vendor_request_erase_spiflash,
	usb_vendor_request_write_spiflash,
	usb_vendor_request_read_spiflash,
	NULL, // used to be write_cpld
	usb_vendor_request_read_board_id,
	usb_vendor_request_read_version_string,
	usb_vendor_request_set_freq,
	usb_vendor_request_set_amp_enable,
	usb_vendor_request_read_partid_serialno,
	usb_vendor_request_set_lna_gain,
	usb_vendor_request_set_vga_gain,
	usb_vendor_request_set_txvga_gain,
	NULL, // was set_if_freq
#ifdef HACKRF_ONE
	usb_vendor_request_set_antenna_enable,
#else
	NULL,
#endif
	usb_vendor_request_set_freq_explicit,
	usb_vendor_request_read_wcid,  // USB_WCID_VENDOR_REQ
	usb_vendor_request_init_sweep,
	usb_vendor_request_operacake_get_boards,
	usb_vendor_request_operacake_set_ports,
	usb_vendor_request_set_hw_sync_mode,
	usb_vendor_request_reset,
	usb_vendor_request_operacake_set_ranges,
	usb_vendor_request_set_clkout_enable,
	usb_vendor_request_spiflash_status,
	usb_vendor_request_spiflash_clear_status,
	usb_vendor_request_operacake_gpio_test,
#ifdef HACKRF_ONE
	usb_vendor_request_cpld_checksum,
#else
	NULL,
#endif
	usb_vendor_request_set_ui_enable,
};

static const uint32_t vendor_request_handler_count =
	sizeof(vendor_request_handler) / sizeof(vendor_request_handler[0]);

usb_request_status_t usb_vendor_request(
	usb_endpoint_t* const endpoint,
	const usb_transfer_stage_t stage
) {
	usb_request_status_t status = USB_REQUEST_STATUS_STALL;
	
	if( endpoint->setup.request < vendor_request_handler_count ) {
		usb_request_handler_fn handler = vendor_request_handler[endpoint->setup.request];
		if( handler ) {
			status = handler(endpoint, stage);
		}
	}
	
	return status;
}

const usb_request_handlers_t usb_request_handlers = {
	.standard = usb_standard_request,
	.class = 0,
	.vendor = usb_vendor_request,
	.reserved = 0,
};

void usb_configuration_changed(
	usb_device_t* const device
) {
	/* Reset transceiver to idle state until other commands are received */
	set_transceiver_mode(TRANSCEIVER_MODE_OFF);
	if( device->configuration->number == 1 ) {
		// transceiver configuration
		led_on(LED1);
	} else {
		/* Configuration number equal 0 means usb bus reset. */
		led_off(LED1);
	}
	usb_endpoint_init(&usb_endpoint_bulk_in);
	usb_endpoint_init(&usb_endpoint_bulk_out);
}

void usb_set_descriptor_by_serial_number(void)
{
	iap_cmd_res_t iap_cmd_res;
	
	/* Read IAP Serial Number Identification */
	iap_cmd_res.cmd_param.command_code = IAP_CMD_READ_SERIAL_NO;
	iap_cmd_call(&iap_cmd_res);
	
	if (iap_cmd_res.status_res.status_ret == CMD_SUCCESS) {
		usb_descriptor_string_serial_number[0] = USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN;
		usb_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
		
		/* 32 characters of serial number, convert to UTF-16LE */
		for (size_t i=0; i<USB_DESCRIPTOR_STRING_SERIAL_LEN; i++) {
			const uint_fast8_t nibble = (iap_cmd_res.status_res.iap_result[i >> 3] >> (28 - (i & 7) * 4)) & 0xf;
			const char c = (nibble > 9) ? ('a' + nibble - 10) : ('0' + nibble);
			usb_descriptor_string_serial_number[2 + i * 2] = c;
			usb_descriptor_string_serial_number[3 + i * 2] = 0x00;
		}
	} else {
		usb_descriptor_string_serial_number[0] = 2;
		usb_descriptor_string_serial_number[1] = USB_DESCRIPTOR_TYPE_STRING;
	}
}

static bool cpld_jtag_sram_load(jtag_t* const jtag) {
	cpld_jtag_take(jtag);
	cpld_xc2c64a_jtag_sram_write(jtag, &cpld_hackrf_program_sram);
	const bool success = cpld_xc2c64a_jtag_sram_verify(jtag, &cpld_hackrf_program_sram, &cpld_hackrf_verify);
	cpld_jtag_release(jtag);
	return success;
}

static void m0_rom_to_ram() {
	uint32_t *dest = &__ram_m0_start__;
	uint32_t *src = &__m0_start__;
	while (src < &__m0_end__) {
		*dest = *src;
		dest++;
		src++;
	}
}

void initial_tx_buffer_fill_completed(void* user_data, unsigned int transferred) {
	(void)user_data;
	(void)transferred;

	// Now we have filled the first half of the transfer buffer we reset the buffer offset to zero
	// so that we will transmit from the start of the buffer and to trigger the transfer of the
	// second half of the buffer.
	usb_bulk_buffer_offset = 0;

	// The SGPIO output is double buffered. Therefore we must write two samples to the SGPIO
	// registers so that the value at the SGPIO pins when streaming starts is our first sample.
	for (int i = 0; i < 2; i++) {
		SGPIO_CLR_STATUS_1 = (1 << SGPIO_SLICE_A);

		uint32_t* const p = (uint32_t*)&usb_bulk_buffer[usb_bulk_buffer_offset];
		__asm__(
			"ldr r0, [%[p], #0]\n\t"
			"str r0, [%[SGPIO_REG_SS], #44]\n\t"
			"ldr r0, [%[p], #4]\n\t"
			"str r0, [%[SGPIO_REG_SS], #20]\n\t"
			"ldr r0, [%[p], #8]\n\t"
			"str r0, [%[SGPIO_REG_SS], #40]\n\t"
			"ldr r0, [%[p], #12]\n\t"
			"str r0, [%[SGPIO_REG_SS], #8]\n\t"
			"ldr r0, [%[p], #16]\n\t"
			"str r0, [%[SGPIO_REG_SS], #36]\n\t"
			"ldr r0, [%[p], #20]\n\t"
			"str r0, [%[SGPIO_REG_SS], #16]\n\t"
			"ldr r0, [%[p], #24]\n\t"
			"str r0, [%[SGPIO_REG_SS], #32]\n\t"
			"ldr r0, [%[p], #28]\n\t"
			"str r0, [%[SGPIO_REG_SS], #0]\n\t"
			:
			: [SGPIO_REG_SS] "l" (SGPIO_PORT_BASE + 0x100),
			[p] "l" (p)
			: "r0"
		);
		usb_bulk_buffer_offset += 32;
	}

	baseband_streaming_enable(&sgpio_config);
}

int main(void) {
	bool operacake_allow_gpio;
	pin_setup();
	enable_1v8_power();
#if (defined HACKRF_ONE || defined RAD1O)
	enable_rf_power();

	/* Let the voltage stabilize */
	delay(1000000);
#endif
	cpu_clock_init();

	/* Wake the M0 */
	m0_rom_to_ram();
	ipc_start_m0((uint32_t)&__ram_m0_start__);

	if( !cpld_jtag_sram_load(&jtag_cpld) ) {
		halt_and_flash(6000000);
	}

#ifdef HACKRF_ONE
	portapack_init();
#endif

#ifndef DFU_MODE
	usb_set_descriptor_by_serial_number();
#endif

	usb_set_configuration_changed_cb(usb_configuration_changed);
	usb_peripheral_reset();
	
	usb_device_init(0, &usb_device);
	
	usb_queue_init(&usb_endpoint_control_out_queue);
	usb_queue_init(&usb_endpoint_control_in_queue);
	usb_queue_init(&usb_endpoint_bulk_out_queue);
	usb_queue_init(&usb_endpoint_bulk_in_queue);

	usb_endpoint_init(&usb_endpoint_control_out);
	usb_endpoint_init(&usb_endpoint_control_in);
	
	nvic_set_priority(NVIC_USB0_IRQ, 255);

	hackrf_ui()->init();

	usb_run(&usb_device);
	
	rf_path_init(&rf_path);

	if( hackrf_ui()->operacake_gpio_compatible() ) {
		operacake_allow_gpio = true;
	} else {
		operacake_allow_gpio = false;
	}
	operacake_init(operacake_allow_gpio);

	unsigned int phase = 0;
	usb_bulk_buffer_offset = 0;

	while(true) {
		// Check whether we need to initiate a CPLD update
		if (start_cpld_update)
			cpld_update();

		// Check whether we need to initiate sweep mode
		if (start_sweep_mode) {
			start_sweep_mode = false;
			sweep_mode();
		}

		if (should_prepare_buffer_for_tx_or_rx()) {
			if (transceiver_mode() == TRANSCEIVER_MODE_RX) {
				// In receive mode we reset buffer positions so the first samples we receive are
				// captured samples and not stale ones already in the buffer.
				// As the SGPIO is double buffered it will produce two buffers worth of junk before
				// the first captured sample.
				usb_bulk_buffer_offset = 0x4000 - 64;
				phase = 1;
				baseband_streaming_enable(&sgpio_config);
			} else if (transceiver_mode() == TRANSCEIVER_MODE_TX) {
				// In transmit mode we fill the buffers over USB before we start streaming so that
				// we only transmit user provided samples. This is especially useful in hardware
				// triggered mode so that we transmit valid samples as soon as the trigger occurs.
				usb_bulk_buffer_offset = 0x4000;
				phase = 0;
				usb_transfer_schedule_block(
					&usb_endpoint_bulk_out,
					&usb_bulk_buffer[0x0000],
					0x4000,
					initial_tx_buffer_fill_completed, NULL
					);
			}
		}

		// Set up IN transfer of buffer 0.
		if ( usb_bulk_buffer_offset >= 16384
		     && phase == 1
		     && transceiver_mode() != TRANSCEIVER_MODE_OFF) {
			usb_transfer_schedule_block(
				(transceiver_mode() == TRANSCEIVER_MODE_RX)
				? &usb_endpoint_bulk_in : &usb_endpoint_bulk_out,
				&usb_bulk_buffer[0x0000],
				0x4000,
				NULL, NULL
				);
			phase = 0;
		}

		// Set up IN transfer of buffer 1.
		if ( usb_bulk_buffer_offset < 16384
		     && phase == 0
		     && transceiver_mode() != TRANSCEIVER_MODE_OFF) {
			usb_transfer_schedule_block(
				(transceiver_mode() == TRANSCEIVER_MODE_RX)
				? &usb_endpoint_bulk_in : &usb_endpoint_bulk_out,
				&usb_bulk_buffer[0x4000],
				0x4000,
				NULL, NULL
			);
			phase = 1;
		}
	}

	return 0;
}
