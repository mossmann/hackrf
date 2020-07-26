/*
 * Copyright 2012 Jared Boone
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

#include <stdint.h>

#include "usb_type.h"
#include "usb_descriptor.h"

#define USB_VENDOR_ID			(0x1D50)

#ifdef HACKRF_ONE
#define USB_PRODUCT_ID			(0x6089)
#elif JAWBREAKER
#define USB_PRODUCT_ID			(0x604B)
#elif RAD1O
#define USB_PRODUCT_ID			(0xCC15)
#else
#define USB_PRODUCT_ID			(0xFFFF)
#endif

#define USB_API_VERSION			(0x0103)

#define USB_WORD(x)	(x & 0xFF), ((x >> 8) & 0xFF)

#define USB_MAX_PACKET0     	(64)
#define USB_MAX_PACKET_BULK_FS	(64)
#define USB_MAX_PACKET_BULK_HS	(512)

#define USB_BULK_IN_EP_ADDR 	(0x81)
#define USB_BULK_OUT_EP_ADDR 	(0x02)

#define USB_STRING_LANGID		(0x0409)

uint8_t usb_descriptor_device[] = {
	18,				   // bLength
	USB_DESCRIPTOR_TYPE_DEVICE,	   // bDescriptorType
	USB_WORD(0x0200),		   // bcdUSB
	0x00,				   // bDeviceClass
	0x00,				   // bDeviceSubClass
	0x00,				   // bDeviceProtocol
	USB_MAX_PACKET0,		   // bMaxPacketSize0
	USB_WORD(USB_VENDOR_ID),	   // idVendor
	USB_WORD(USB_PRODUCT_ID),	   // idProduct
	USB_WORD(USB_API_VERSION),	// bcdDevice
	0x01,				   // iManufacturer
	0x02,				   // iProduct
	0x04,				   // iSerialNumber
	0x01				   // bNumConfigurations
};

uint8_t usb_descriptor_device_qualifier[] = {
	10,					// bLength
	USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,	// bDescriptorType
	USB_WORD(0x0200),			// bcdUSB
	0x00,					// bDeviceClass
	0x00,					// bDeviceSubClass
	0x00,					// bDeviceProtocol
	64,					// bMaxPacketSize0
	0x01,					// bNumOtherSpeedConfigurations
	0x00					// bReserved
};

uint8_t usb_descriptor_configuration_full_speed[] = {
	9,					// bLength
	USB_DESCRIPTOR_TYPE_CONFIGURATION,	// bDescriptorType
	USB_WORD(32),				// wTotalLength
	0x01,					// bNumInterfaces
	0x01,					// bConfigurationValue
	0x03,					// iConfiguration
	0x80,					// bmAttributes: USB-powered
	250,					// bMaxPower: 500mA

	9,							// bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,		// bDescriptorType
	0x00,							// bInterfaceNumber
	0x00,							// bAlternateSetting
	0x02,							// bNumEndpoints
	0xFF,							// bInterfaceClass: vendor-specific
	0xFF,							// bInterfaceSubClass
	0xFF,							// bInterfaceProtocol: vendor-specific
	0x00,							// iInterface

	7,							// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	USB_BULK_IN_EP_ADDR,				// bEndpointAddress
	0x02,							// bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS),	// wMaxPacketSize
	0x00,							// bInterval: no NAK

	7,							// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	USB_BULK_OUT_EP_ADDR,			// bEndpointAddress
	0x02,							// bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_FS),	// wMaxPacketSize
	0x00,							// bInterval: no NAK

	0,									// TERMINATOR
};

uint8_t usb_descriptor_configuration_high_speed[] = {
	9,							// bLength
	USB_DESCRIPTOR_TYPE_CONFIGURATION,	// bDescriptorType
	USB_WORD(32),						// wTotalLength
	0x01,							// bNumInterfaces
	0x01,							// bConfigurationValue
	0x03,							// iConfiguration
	0x80,							// bmAttributes: USB-powered
	250,							// bMaxPower: 500mA

	9,							// bLength
	USB_DESCRIPTOR_TYPE_INTERFACE,		// bDescriptorType
	0x00,							// bInterfaceNumber
	0x00,							// bAlternateSetting
	0x02,							// bNumEndpoints
	0xFF,							// bInterfaceClass: vendor-specific
	0xFF,							// bInterfaceSubClass
	0xFF,							// bInterfaceProtocol: vendor-specific
	0x00,							// iInterface

	7,							// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	USB_BULK_IN_EP_ADDR,				// bEndpointAddress
	0x02,							// bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_HS),	// wMaxPacketSize
	0x00,							// bInterval: no NAK

	7,								// bLength
	USB_DESCRIPTOR_TYPE_ENDPOINT,		// bDescriptorType
	USB_BULK_OUT_EP_ADDR,			// bEndpointAddress
	0x02,							// bmAttributes: BULK
	USB_WORD(USB_MAX_PACKET_BULK_HS),	// wMaxPacketSize
	0x00,							// bInterval: no NAK

	0,									// TERMINATOR
};


uint8_t usb_descriptor_string_languages[] = {
	0x04,			    // bLength
	USB_DESCRIPTOR_TYPE_STRING,	    // bDescriptorType
	USB_WORD(USB_STRING_LANGID),	// wLANGID
};

uint8_t usb_descriptor_string_manufacturer[] = {
	40,					// bLength
	USB_DESCRIPTOR_TYPE_STRING,	    // bDescriptorType
	'G', 0x00,
	'r', 0x00,
	'e', 0x00,
	'a', 0x00,
	't', 0x00,
	' ', 0x00,
	'S', 0x00,
	'c', 0x00,
	'o', 0x00,
	't', 0x00,
	't', 0x00,
	' ', 0x00,
	'G', 0x00,
	'a', 0x00,
	'd', 0x00,
	'g', 0x00,
	'e', 0x00,
	't', 0x00,
	's', 0x00,
};

uint8_t usb_descriptor_string_product[] = {
#ifdef HACKRF_ONE
	22,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'H', 0x00,
	'a', 0x00,
	'c', 0x00,
	'k', 0x00,
	'R', 0x00,
	'F', 0x00,
	' ', 0x00,
	'O', 0x00,
	'n', 0x00,
	'e', 0x00,
#elif JAWBREAKER
	36,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'H', 0x00,
	'a', 0x00,
	'c', 0x00,
	'k', 0x00,
	'R', 0x00,
	'F', 0x00,
	' ', 0x00,
	'J', 0x00,
	'a', 0x00,
	'w', 0x00,
	'b', 0x00,
	'r', 0x00,
	'e', 0x00,
	'a', 0x00,
	'k', 0x00,
	'e', 0x00,
	'r', 0x00,
#elif RAD1O
	12,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'r', 0x00,
	'a', 0x00,
	'd', 0x00,
	'1', 0x00,
	'o', 0x00,
#else
	14,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'H', 0x00,
	'a', 0x00,
	'c', 0x00,
	'k', 0x00,
	'R', 0x00,
	'F', 0x00,
#endif
};

uint8_t usb_descriptor_string_config_description[] = {
	24,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'T', 0x00,
	'r', 0x00,
	'a', 0x00,
	'n', 0x00,
	's', 0x00,
	'c', 0x00,
	'e', 0x00,
	'i', 0x00,
	'v', 0x00,
	'e', 0x00,
	'r', 0x00,
};

#ifdef DFU_MODE
uint8_t usb_descriptor_string_serial_number[] = {
	30,						// bLength
	USB_DESCRIPTOR_TYPE_STRING,		// bDescriptorType
	'R', 0x00,
	'u', 0x00,
	'n', 0x00,
	'n', 0x00,
	'i', 0x00,
	'n', 0x00,
	'g', 0x00,
	'F', 0x00,
	'r', 0x00,
	'o', 0x00,
	'm', 0x00,
	'R', 0x00,
	'A', 0x00,
	'M', 0x00,
};
#else
uint8_t usb_descriptor_string_serial_number[USB_DESCRIPTOR_STRING_SERIAL_BUF_LEN];
#endif

uint8_t* usb_descriptor_strings[] = {
	usb_descriptor_string_languages,
	usb_descriptor_string_manufacturer,
	usb_descriptor_string_product,
	usb_descriptor_string_config_description,
	usb_descriptor_string_serial_number,
	0,		// TERMINATOR
};

uint8_t wcid_string_descriptor[] = {
	18,                          // bLength
	USB_DESCRIPTOR_TYPE_STRING,  // bDescriptorType
	'M', 0x00,
	'S', 0x00,
	'F', 0x00,
	'T', 0x00,
	'1', 0x00,
	'0', 0x00,
	'0', 0x00,
	USB_WCID_VENDOR_REQ, // vendor request code for further descriptor
	0x00
};

uint8_t wcid_feature_descriptor[] = {
	0x28, 0x00, 0x00, 0x00,  // bLength
	USB_WORD(0x0100),        // WCID version
	USB_WORD(0x0004),        // WICD descriptor index
	0x01,                    //bNumSections
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Reserved
	0x00,        //bInterfaceNumber
	0x01,        //Reserved
	'W', 'I', 'N', 'U', 'S', 'B', 0x00,0x00, //Compatible ID, padded with zeros
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Sub-compatible ID
	0x00,0x00,0x00,0x00,0x00,0x00            //Reserved
};
