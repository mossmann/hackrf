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

#include <hackrf.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#define FREQ_MIN_MHZ (0)    /*    0 MHz */
#define FREQ_MAX_MHZ (7250) /* 7250 MHz */
#define MAX_FREQ_RANGES 8

#define INVALID_ADDRESS 0xFF
#define INVALID_PORT 0xFF

static void usage() {
	printf("\nUsage:\n");
	printf("\t-h, --help: this help\n");
	printf("\t-d, --device <n>: specify a particular device by serial number\n");
	printf("\t-o, --address <n>: specify a particular operacake by address [default: 0x00]\n");
	printf("\t-a <n>: set port A connection\n");
	printf("\t-b <n>: set port B connection\n");
	printf("\t-f <min:max:port>: automatically assign <port> for range <min:max> in MHz\n");
	printf("\t-l, --list: list available operacake boards\n");
	printf("\t-g, --gpio_test: test GPIO functionality of an opera cake\n");
}

static struct option long_options[] = {
	{ "device", no_argument, 0, 'd' },
	{ "address", no_argument, 0, 'o' },
	{ "list", no_argument, 0, 'l' },
	{ "gpio_test", no_argument, 0, 'g' },
	{ "help", no_argument, 0, 'h' },
	{ 0, 0, 0, 0 },
};

typedef struct {
	uint16_t freq_min;
	uint16_t freq_max;
	uint8_t port;
} hackrf_oc_range;

int parse_uint16(char* const s, uint16_t* const value) {
	char* s_end = s;
	const long long_value = strtol(s, &s_end, 10);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = (uint16_t)long_value;
		return HACKRF_SUCCESS;
	} else {
		return HACKRF_ERROR_INVALID_PARAM;
	}
}

int parse_port(char* str, uint8_t* port) {
	uint16_t tmp_port;
	int result;

	if(str[0] == 'A' || str[0] == 'B') {
		// The port was specified as a side and number eg. A1 or B3
		result = parse_uint16(str+1, &tmp_port);
		if (result != HACKRF_SUCCESS)
			return result;

		if(tmp_port >= 5 || tmp_port <= 0) {
			fprintf(stderr, "invalid port: %s\n", str);
			return HACKRF_ERROR_INVALID_PARAM;
		}

		// Value was a valid port between 0-4
		if(str[0] == 'A') {
			// A1=0, A2=1, A3=2, A4=3
			tmp_port -= 1;
		} else {
			// If B was specfied just add 4-1 ports
			// B1=4, B2=5, B3=6, B4=7
			tmp_port += 3;
		}
	} else {
		result = parse_uint16(str, &tmp_port);
		if (result != HACKRF_SUCCESS)
			return result;
	}
	*port = tmp_port & 0xFF;
	// printf("Port: %d\n", *port);
	return HACKRF_SUCCESS;
}

int parse_range(char* s, hackrf_oc_range* range) {
	int result;
	char *sep = strchr(s, ':');
	if (!sep)
		return HACKRF_ERROR_INVALID_PARAM;
	// Replace : separator to null terminate string for strtol()
	*sep = 0;
	sep++; // Skip past the separator

	char *sep2 = strchr(sep, ':');
	if (!sep2)
		return HACKRF_ERROR_INVALID_PARAM;
	// Replace : separator to null terminate string for strtol()
	*sep2 = 0;
	sep2++; // Skip past the separator

	result = parse_uint16(s, &range->freq_min);
	if (result != HACKRF_SUCCESS)
		return result;
	result = parse_uint16(sep, &range->freq_max);
	if (result != HACKRF_SUCCESS)
		return result;
	result = parse_port(sep2, &(range->port));
	return result;
}

int main(int argc, char** argv) {
	int opt;
	const char* serial_number = NULL;
	uint8_t operacake_address = INVALID_ADDRESS;
	uint8_t port_a = INVALID_PORT;
	uint8_t port_b = INVALID_PORT;
	bool set_ports = false;
	bool list = false;
	bool gpio_test = false;
	uint8_t operacakes[8];
	uint8_t operacake_count = 0;
	int i = 0;
	hackrf_device* device = NULL;
	int option_index = 0;
	hackrf_oc_range ranges[MAX_FREQ_RANGES];
	uint8_t range_idx = 0;

	int result = hackrf_init();
	if( result ) {
		printf("hackrf_init() failed: %s (%d)\n", hackrf_error_name(result), result);
		return -1;
	}

	while( (opt = getopt_long(argc, argv, "d:o:a:b:lf:hg?", long_options, &option_index)) != EOF ) {
		switch( opt ) {
		case 'd':
			serial_number = optarg;
			break;

		case 'o':
			operacake_address = atoi(optarg);
			break;

		case 'f':
			if(MAX_FREQ_RANGES == range_idx) {
				fprintf(stderr,
						"argument error: specify a maximum of %u frequency ranges.\n",
						MAX_FREQ_RANGES);
				usage();
				return EXIT_FAILURE;
			}
			result = parse_range(optarg, &ranges[range_idx]);
			if (result != HACKRF_SUCCESS) {
				fprintf(stderr, "failed to parse range\n");
				return EXIT_FAILURE;
			}
			if(ranges[range_idx].freq_min >= ranges[range_idx].freq_max) {
				fprintf(stderr,
						"argument error: freq_max must be greater than freq_min.\n");
				usage();
				return EXIT_FAILURE;
			}
			if(FREQ_MAX_MHZ < ranges[range_idx].freq_max) {
				fprintf(stderr,
						"argument error: freq_max may not be higher than %u.\n",
						FREQ_MAX_MHZ);
				usage();
				return EXIT_FAILURE;
			}
			range_idx++;
			break;

		case 'a':
			result = parse_port(optarg, &port_a);
			if (result != HACKRF_SUCCESS) {
				fprintf(stderr, "failed to parse port\n");
				return EXIT_FAILURE;
			}
			set_ports = true;
			break;

		case 'b':
			result = parse_port(optarg, &port_b);
			if (result != HACKRF_SUCCESS) {
				fprintf(stderr, "failed to parse port\n");
				return EXIT_FAILURE;
			}
			set_ports = true;
			break;

		case 'l':
			list = true;
			break;

		case 'g':
			gpio_test = true;
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
	}

	if(!(list || set_ports || range_idx || gpio_test)) {
		fprintf(stderr, "Specify either list, address, or GPIO test option.\n");
		usage();
		return EXIT_FAILURE;
	}

	if((set_ports || gpio_test) && (operacake_address == INVALID_ADDRESS)) {
		fprintf(stderr, "An address is required.\n");
		usage();
		return EXIT_FAILURE;
	}

	result = hackrf_open_by_serial(serial_number, &device);
	if (result != HACKRF_SUCCESS) {
		fprintf(stderr, "hackrf_open() failed: %s (%d)\n",
				hackrf_error_name(result), result);
		return EXIT_FAILURE;
	}

	if(list) {
		result = hackrf_get_operacake_boards(device, operacakes);
		if (result != HACKRF_SUCCESS) {
			fprintf(stderr, "hackrf_get_operacake_boards() failed: %s (%d)\n",
					hackrf_error_name(result), result);
			return EXIT_FAILURE;
		}
		printf("Operacakes found: ");
		for(i=0; i<8; i++) {
			if(operacakes[i] !=0) {
				printf("\n%d", operacakes[i]);
				operacake_count++;
			}
		}
		if(!operacake_count)
			printf("None");
		printf("\n");
	}

	if(gpio_test) {
		uint16_t test_result;
		uint8_t reg, mask = 0x7;
		result = hackrf_operacake_gpio_test(device, operacake_address, &test_result);
		if (result != HACKRF_SUCCESS) {
			fprintf(stderr, "hackrf_operacake_gpio_test() failed: %s (%d)\n",
					hackrf_error_name(result), result);
			return EXIT_FAILURE;
		}
		
		if(test_result) {
			fprintf(stderr, "GPIO test failed\n");
			fprintf(stderr, "Pin\tHigh\tShorts\tLow\n");
			reg = test_result & mask;
			fprintf(stderr, "u2ctrl1\t%d\t%d\t%d\n",
			        (reg>>2) & 1,
					(reg>>1) & 1,
					reg & 1);
			test_result >>= 3;
			reg = test_result & mask;
			fprintf(stderr, "u2ctrl0\t%d\t%d\t%d\n",
			        (reg>>2) & 1,
					(reg>>1) & 1,
					reg & 1);
			test_result >>= 3;
			reg = test_result & mask;
			fprintf(stderr, "u3ctrl1\t%d\t%d\t%d\n",
			        (reg>>2) & 1,
					(reg>>1) & 1,
					reg & 1);
			test_result >>= 3;
			reg = test_result & mask;
			fprintf(stderr, "u3ctrl0\t%d\t%d\t%d\n",
			        (reg>>2) & 1,
					(reg>>1) & 1,
					reg & 1);
			test_result >>= 3;
			reg = test_result & mask;
			fprintf(stderr, "u1ctrl \t%d\t%d\t%d\n",
			        (reg>>2) & 1,
					(reg>>1) & 1,
					reg & 1);
		} else {
			fprintf(stderr, "GPIO test passed\n");
		}
	}

	if(set_ports) {
		// Set other port to "don't care" if not set
		if(port_a == INVALID_PORT) {
			if(port_b >= 4) {
				port_a = 0;
			} else {
				port_a = 4;
			}
		}
		if(port_b == INVALID_PORT) {
			if(port_a >= 4) {
				port_b = 0;
			} else {
				port_b = 4;
			}
		}
		if(((port_a<=3) && (port_b<=3)) || ((port_a>=4) && (port_b>=4))) {
			fprintf(stderr, "Port A and B cannot be connected to the same side\n");
				return EXIT_FAILURE;
		}
		result = hackrf_set_operacake_ports(device, operacake_address, port_a, port_b);
		if( result ) {
			printf("hackrf_set_operacake_ports() failed: %s (%d)\n", hackrf_error_name(result), result);
			return EXIT_FAILURE;
		}
	}

	if(range_idx) {
		uint8_t range_bytes[MAX_FREQ_RANGES * sizeof(hackrf_oc_range)];
		uint8_t ptr;
		for(i=0; i<range_idx; i++) {
			ptr = 5*i;
			range_bytes[ptr] = ranges[i].freq_min >> 8;
			range_bytes[ptr+1] = ranges[i].freq_min & 0xFF;
			range_bytes[ptr+2] = ranges[i].freq_max >> 8;
			range_bytes[ptr+3] = ranges[i].freq_max & 0xFF;
			range_bytes[ptr+4] = ranges[i].port;
		}

		result = hackrf_set_operacake_ranges(device, range_bytes, range_idx*5);
		if( result ) {
			printf("hackrf_set_operacake_ranges() failed: %s (%d)\n", hackrf_error_name(result), result);
			return -1;
		}
	}

	result = hackrf_close(device);
	if( result ) {
		printf("hackrf_close() failed: %s (%d)\n", hackrf_error_name(result), result);
		return -1;
	}
	hackrf_exit();
	return 0;
}
