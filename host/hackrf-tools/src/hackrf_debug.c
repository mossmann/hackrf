/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013 Benjamin Vernoux <titanmkd@gmail.com>
 * Copyright 2017 Dominic Spill <dominicgs@gmail.com>
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

#include <hackrf.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#define REGISTER_INVALID 32767

int parse_int(char* s, uint16_t* const value) {
	uint_fast8_t base = 10;
	char* s_end;
	long long_value;

	if( strlen(s) > 2 ) {
		if( s[0] == '0' ) {
			if( (s[1] == 'x') || (s[1] == 'X') ) {
				base = 16;
				s += 2;
			} else if( (s[1] == 'b') || (s[1] == 'B') ) {
				base = 2;
				s += 2;
			}
		}
	}

	s_end = s;
	long_value = strtol(s, &s_end, base);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = (uint16_t)long_value;
		return HACKRF_SUCCESS;
	} else {
		return HACKRF_ERROR_INVALID_PARAM;
	}
}

int max2837_read_register(hackrf_device* device, const uint16_t register_number) {
	uint16_t register_value;
	int result = hackrf_max2837_read(device, (uint8_t)register_number, &register_value);
	
	if( result == HACKRF_SUCCESS ) {
		printf("[%2d] -> 0x%03x\n", register_number, register_value);
	} else {
		printf("hackrf_max2837_read() failed: %s (%d)\n", hackrf_error_name(result), result);
	}
	return result;
}

int max2837_read_registers(hackrf_device* device) {
	uint16_t register_number;
	int result = HACKRF_SUCCESS;
	
	for(register_number=0; register_number<32; register_number++) {
		result = max2837_read_register(device, register_number);
		if( result != HACKRF_SUCCESS ) {
			break;
		}
	}
	return result;
}

int max2837_write_register(
	hackrf_device* device,
	const uint16_t register_number,
	const uint16_t register_value
) {
	int result = HACKRF_SUCCESS;
	result = hackrf_max2837_write(device, (uint8_t)register_number, register_value);
	
	if( result == HACKRF_SUCCESS ) {
		printf("0x%03x -> [%2d]\n", register_value, register_number);
	} else {
		printf("hackrf_max2837_write() failed: %s (%d)\n", hackrf_error_name(result), result);
	}
	return result;
}

int si5351c_read_register(hackrf_device* device, const uint16_t register_number) {
	uint16_t register_value;
	int result = hackrf_si5351c_read(device, register_number, &register_value);
	
	if( result == HACKRF_SUCCESS ) {
		printf("[%3d] -> 0x%02x\n", register_number, register_value);
	} else {
		printf("hackrf_si5351c_read() failed: %s (%d)\n", hackrf_error_name(result), result);
	}
	
	return result;
}

int si5351c_read_registers(hackrf_device* device) {
	uint16_t register_number;
	int result = HACKRF_SUCCESS;
	
	for(register_number=0; register_number<256; register_number++) {
		result = si5351c_read_register(device, register_number);
		if( result != HACKRF_SUCCESS ) {
			break;
		}
	}
	
	return result;
}

int si5351c_write_register(
	hackrf_device* device,
	const uint16_t register_number,
	const uint16_t register_value
) {
	int result = HACKRF_SUCCESS;
	result = hackrf_si5351c_write(device, register_number, register_value);
	
	if( result == HACKRF_SUCCESS ) {
		printf("0x%2x -> [%3d]\n", register_value, register_number);
	} else {
		printf("hackrf_max2837_write() failed: %s (%d)\n", hackrf_error_name(result), result);
	}
	
	return result;
}

#define SI5351C_CLK_POWERDOWN (1<<7)
#define SI5351C_CLK_INT_MODE  (1<<6)
#define SI5351C_CLK_PLL_SRC   (1<<5)
#define SI5351C_CLK_INV       (1<<4)
#define SI5351C_CLK_SRC_XTAL            0
#define SI5351C_CLK_SRC_CLKIN           1
#define SI5351C_CLK_SRC_MULTISYNTH_0_4  2
#define SI5351C_CLK_SRC_MULTISYNTH_SELF 3

void print_clk_control(uint16_t clk_ctrl) {
	uint8_t clk_src, clk_pwr;
	printf("\tclock control = \n");
	if(clk_ctrl & SI5351C_CLK_POWERDOWN)
		printf("\t\tPower Down\n");
	else
		printf("\t\tPower Up\n");
	if(clk_ctrl & SI5351C_CLK_INT_MODE)
		printf("\t\tInt Mode\n");
	else
		printf("\t\tFrac Mode\n");
	if(clk_ctrl & SI5351C_CLK_PLL_SRC)
		printf("\t\tPLL src B\n");
	else
		printf("\t\tPLL src A\n");
	if(clk_ctrl & SI5351C_CLK_INV)
		printf("\t\tInverted\n");
	clk_src = (clk_ctrl >> 2) & 0x3;
	switch (clk_src) {
		case 0:
			printf("\t\tXTAL\n");
			break;
		case 1:
			printf("\t\tCLKIN\n");
			break;
		case 2:
			printf("\t\tMULTISYNTH 0 4\n");
			break;
		case 3:
			printf("\t\tMULTISYNTH SELF\n");
			break;
	}
	clk_pwr = clk_ctrl & 0x3;
	switch (clk_pwr) {
		case 0:
			printf("\t\t2 mA\n");
			break;
		case 1:
			printf("\t\t4 mA\n");
			break;
		case 2:
			printf("\t\t6 mA\n");
			break;
		case 3:
			printf("\t\t8 mA\n");
			break;
	}
}

int si5351c_read_multisynth_config(hackrf_device* device, const uint_fast8_t ms_number) {
	uint_fast8_t i, reg_base, reg_number;
	uint16_t parameters[8], clk_control;
	uint32_t p1,p2,p3,r_div;
	uint_fast8_t div_lut[] = {1,2,4,8,16,32,64,128};
	int result;

	printf("MS%d:", ms_number);
	result = hackrf_si5351c_read(device, 16+ms_number, &clk_control);
	if( result != HACKRF_SUCCESS ) {
		return result;
	}
	print_clk_control(clk_control);
	if(ms_number <6){
		reg_base = 42 + (ms_number * 8);
		for(i=0; i<8; i++) {
			reg_number = reg_base + i;
			result = hackrf_si5351c_read(device, reg_number, &parameters[i]);
			if( result != HACKRF_SUCCESS ) {
				return result;
			}
		}

		p1 = ((parameters[2] & 0x03) << 16)
			| (parameters[3] << 8)
			| parameters[4];
		p2 = ((parameters[5] & 0x0F) << 16)
			| (parameters[6] << 8)
			| parameters[7];
		p3 = ((parameters[5] & 0xF0) << 12)
			| (parameters[0] << 8)
			|  parameters[1];
		r_div = (parameters[2] >> 4) & 0x7;

		printf("\tp1 = %u\n", p1);
		printf("\tp2 = %u\n", p2);
		printf("\tp3 = %u\n", p3);
		if(p3)
			printf("\tOutput (800Mhz PLL): %#.10f Mhz\n", ((double)800 / (double)(((double)p1*p3 + p2 + 512*p3)/(double)(128*p3))) / div_lut[r_div] );
	} else {
		// MS6 and 7 are integer only
		unsigned int parms;
		reg_base = 90;

		for(i=0; i<3; i++) {
			uint_fast8_t reg_number = reg_base + i;
			int result = hackrf_si5351c_read(device, reg_number, &parameters[i]);
			if( result != HACKRF_SUCCESS ) {
				return result;
			}
		}
		r_div = (ms_number == 6) ? parameters[2] & 0x7 : (parameters[2] & 0x70) >> 4 ;
		parms = (ms_number == 6) ? parameters[0] : parameters[1];
		printf("\tp1_int = %u\n", parms);
		if(parms)
			printf("\tOutput (800Mhz PLL): %#.10f Mhz\n", (800.0f / parms) / div_lut[r_div] );
	}
	printf("\toutput divider = %u\n", div_lut[r_div]);
	return HACKRF_SUCCESS;
}

int si5351c_read_configuration(hackrf_device* device) {
	uint_fast8_t ms_number;
	int result;
			
	for(ms_number=0; ms_number<8; ms_number++) {
		result = si5351c_read_multisynth_config(device, ms_number);
		if( result != HACKRF_SUCCESS ) {
			return result;
		}
	}
	return HACKRF_SUCCESS;
}

/*
 * RFFC5071 and RFFC5072 are similar components with a compatible control
 * interface.  RFFC5071 was used on some early prototypes, so the libhackrf API
 * calls are named that way.  Because we use RFFC5072 on production hardware,
 * we use that name here and present it to the user.
 */

int rffc5072_read_register(hackrf_device* device, const uint16_t register_number) {
	uint16_t register_value;
	int result = hackrf_rffc5071_read(device, (uint8_t)register_number, &register_value);
	
	if( result == HACKRF_SUCCESS ) {
		printf("[%2d] -> 0x%03x\n", register_number, register_value);
	} else {
		printf("hackrf_rffc5071_read() failed: %s (%d)\n", hackrf_error_name(result), result);
	}
	
	return result;
}

int rffc5072_read_registers(hackrf_device* device) {
	uint16_t register_number;
	int result = HACKRF_SUCCESS;
	
	for(register_number=0; register_number<31; register_number++) {
		result = rffc5072_read_register(device, register_number);
		if( result != HACKRF_SUCCESS ) {
			break;
		}
	}
	
	return result;
}

int rffc5072_write_register(
	hackrf_device* device,
	const uint16_t register_number,
	const uint16_t register_value
) {
	int result = HACKRF_SUCCESS;
	result = hackrf_rffc5071_write(device, (uint8_t)register_number, register_value);
	
	if( result == HACKRF_SUCCESS ) {
		printf("0x%03x -> [%2d]\n", register_value, register_number);
	} else {
		printf("hackrf_rffc5071_write() failed: %s (%d)\n", hackrf_error_name(result), result);
	}
	
	return result;
}

enum parts {
	PART_NONE = 0,
	PART_MAX2837 = 1,
	PART_SI5351C = 2,
	PART_RFFC5072 = 3,
};

int read_register(hackrf_device* device, uint8_t part,
                   const uint16_t register_number) {
	switch (part) {
		case PART_MAX2837:
			return max2837_read_register(device, register_number);
		case PART_SI5351C:
			return si5351c_read_register(device, register_number);
		case PART_RFFC5072:
			return rffc5072_read_register(device, register_number);
	}
	return HACKRF_ERROR_INVALID_PARAM;
}

int read_registers(hackrf_device* device, uint8_t part) {
	switch (part) {
		case PART_MAX2837:
			return max2837_read_registers(device);
		case PART_SI5351C:
			return si5351c_read_registers(device);
		case PART_RFFC5072:
			return rffc5072_read_registers(device);
	}
	return HACKRF_ERROR_INVALID_PARAM;
}

int write_register(hackrf_device* device, uint8_t part,
                   const uint16_t register_number,
	               const uint16_t register_value) {
	switch (part) {
		case PART_MAX2837:
			return max2837_write_register(device, register_number, register_value);
		case PART_SI5351C:
			return si5351c_write_register(device, register_number, register_value);
		case PART_RFFC5072:
			return rffc5072_write_register(device, register_number, register_value);
	}
	return HACKRF_ERROR_INVALID_PARAM;
}

static void usage() {
	printf("\nUsage:\n");
	printf("\t-h, --help: this help\n");
	printf("\t-n, --register <n>: set register number for read/write operations\n");
	printf("\t-r, --read: read register specified by last -n argument, or all registers\n");
	printf("\t-w, --write <v>: write register specified by last -n argument with value <v>\n");
	printf("\t-c, --config: print SI5351C multisynth configuration information\n");
	printf("\t-d, --device <s>: specify a particular device by serial number\n");
	printf("\t-m, --max2837: target MAX2837\n");
	printf("\t-s, --si5351c: target SI5351C\n");
	printf("\t-f, --rffc5072: target RFFC5072\n");
	printf("\nExamples:\n");
	printf("\thackrf_debug --si5351c -n 0 -r     # reads from si5351c register 0\n");
	printf("\thackrf_debug --si5351c -c          # displays si5351c multisynth configuration\n");
	printf("\thackrf_debug --rffc5072 -r         # reads all rffc5072 registers\n");
	printf("\thackrf_debug --max2837 -n 10 -w 22 # writes max2837 register 10 with 22 decimal\n");
}

static struct option long_options[] = {
	{ "config", no_argument, 0, 'c' },
	{ "register", required_argument, 0, 'n' },
	{ "write", required_argument, 0, 'w' },
	{ "read", no_argument, 0, 'r' },
	{ "device", no_argument, 0, 'd' },
	{ "help", no_argument, 0, 'h' },
	{ "max2837", no_argument, 0, 'm' },
	{ "si5351c", no_argument, 0, 's' },
	{ "rffc5072", no_argument, 0, 'f' },
	{ 0, 0, 0, 0 },
};

int main(int argc, char** argv) {
	int opt;
	uint16_t register_number = REGISTER_INVALID;
	uint16_t register_value;
	hackrf_device* device = NULL;
	int option_index = 0;
	bool read = false;
	bool write = false;
	bool dump_config = false;
	uint8_t part = PART_NONE;
	const char* serial_number = NULL;

	int result = hackrf_init();
	if(result) {
		printf("hackrf_init() failed: %s (%d)\n", hackrf_error_name(result), result);
		return EXIT_FAILURE;
	}

	while( (opt = getopt_long(argc, argv, "n:rw:d:cmsfh?", long_options, &option_index)) != EOF ) {
		switch( opt ) {
		case 'n':
			result = parse_int(optarg, &register_number);
			break;
		
		case 'w':
			write = true;
			result = parse_int(optarg, &register_value);
			break;

		case 'r':
			read = true;
			break;
		
		case 'c':
			dump_config = true;
			break;

		case 'd':
			serial_number = optarg;
			break;
		
		case 'm':
			if(part != PART_NONE) {
				fprintf(stderr, "Only one part can be specified.'\n");
				return EXIT_FAILURE;
			}
			part = PART_MAX2837;
			break;
		
		case 's':
			if(part != PART_NONE) {
				fprintf(stderr, "Only one part can be specified.'\n");
				return EXIT_FAILURE;
			}
			part = PART_SI5351C;
			break;
		
		case 'f':
			if(part != PART_NONE) {
				fprintf(stderr, "Only one part can be specified.'\n");
				return EXIT_FAILURE;
			}
			part = PART_RFFC5072;
			break;

		case 'h':
		case '?':
			usage();
			return EXIT_SUCCESS;
		default:
			fprintf(stderr, "unknown argument '-%c %s'\n", opt, optarg);
			usage();
			return EXIT_FAILURE;
		}

		if(result != HACKRF_SUCCESS) {
			printf("argument error: %s (%d)\n", hackrf_error_name(result), result);
			usage();
			return EXIT_FAILURE;
		}
	}

	if(write && read) {
		fprintf(stderr, "Read and write options are mutually exclusive.\n");
		usage();
		return EXIT_FAILURE;
	}

	if(write && dump_config) {
		fprintf(stderr, "Config and write options are mutually exclusive.\n");
		usage();
		return EXIT_FAILURE;
	}

	if(dump_config && part != PART_SI5351C) {
		fprintf(stderr, "Config option is only valid for SI5351C.\n");
		usage();
		return EXIT_FAILURE;
	}

	if(!(write || read || dump_config)) {
		fprintf(stderr, "Specify read, write, or config option.\n");
		usage();
		return EXIT_FAILURE;
	}

	if(part == PART_NONE) {
		fprintf(stderr, "Specify a part to read, write, or print config from.\n");
		usage();
		return EXIT_FAILURE;
	}

	result = hackrf_open_by_serial(serial_number, &device);
	if(result) {
		printf("hackrf_open() failed: %s (%d)\n", hackrf_error_name(result), result);
		return EXIT_FAILURE;
	}

	if(write) {
		result = write_register(device, part, register_number, register_value);
	}

	if(read) {
		if(register_number == REGISTER_INVALID) {
			result = read_registers(device, part);
		} else {
			result = read_register(device, part, register_number);
		}
	}

	if(dump_config) {
		si5351c_read_configuration(device);
	}

	result = hackrf_close(device);
	if(result) {
		printf("hackrf_close() failed: %s (%d)\n", hackrf_error_name(result), result);
		return EXIT_FAILURE;
	}

	hackrf_exit();
    return EXIT_SUCCESS;
}
