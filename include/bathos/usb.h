/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

/* Generic driver for usb device */

#ifndef __BATHOS_USB_H__
#define __BATHOS_USB_H__

#include <bathos/types.h>

/* Generic usb device callbacks. The actual implementation shall provide
 * the needed functions */

struct usb_ops_t {
	void (*reset)(void);
	void (*sleep)(void);
	void (*token)(int type, int ep, uint8_t *buf, int len);
	void (*sof)(void);
};

#define declare_usb_ops(r, s, t, S) struct usb_ops_t __usb_ops = { \
		.reset = r, .sleep = s, .token = t, .sof = S}

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

struct usb_tok_setup {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint8_t wValueL;
	uint8_t wValueH;
	uint8_t wIndexL;
	uint8_t wIndexH;
	uint8_t wLengthL;
	uint8_t wLengthH;
};

struct usb_descriptor_list {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
};

/* Endpoint 0 size: 8 bytes is enough*/
#define EP0_BUFSIZE 8

int usb_enable(void);

int usb_disable(void);

int usb_set_address(uint8_t addr);

int usb_request_tx_ep(uint8_t ep, void *buf, int bufsize);

int usb_release_tx_ep(uint8_t ep, int bufsize);

int usb_request_rx_ep(uint8_t ep, void *buf, int bufsize);

int usb_release_rx_ep(uint8_t ep, int bufsize);

int usb_write(uint8_t ep, const uint8_t *buf, int len);

#endif
