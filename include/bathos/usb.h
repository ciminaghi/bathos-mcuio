/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 *
 * Attempt to make a cleaner distinction between high and low level drivers
 * and general code reorganization by Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __BATHOS_USB_H__
#define __BATHOS_USB_H__

#include <stdint.h>

/*
 * Generic usb related definitions
 */

/* These defines are for the 'type' argument of token_cb */
#define USB_TOKEN_IN	0x9
#define USB_TOKEN_OUT	0x1
#define USB_TOKEN_SETUP	0xd

/* standard control endpoint request types */
#define USB_GET_STATUS		0
#define USB_CLEAR_FEATURE	1
#define USB_SET_FEATURE		3
#define USB_SET_ADDRESS		5
#define USB_GET_DESCRIPTOR	6
#define USB_GET_CONFIGURATION	8
#define USB_SET_CONFIGURATION	9
#define USB_GET_INTERFACE	10
#define USB_SET_INTERFACE	11

/*
 * This represents a setup message
 */
struct usb_setup_token {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint8_t wValueL;
	uint8_t wValueH;
	uint8_t wIndexL;
	uint8_t wIndexH;
	uint8_t wLengthL;
	uint8_t wLengthH;
};

#endif /* __BATHOS_USB_H__ */
