/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 *
 * Attempt to make a cleaner distinction between high and low level drivers
 * by Davide Ciminaghi <ciminaghi@gnudd.com>
 */

/*
 * This header contains stuff related to the implemenation of usb slave drivers.
 *
 * Two kind of drivers shall exist:
 *    + Drivers of slave controllers.
 *    + Drivers implementing the slave side of specific usb protocols (such as
 *      cdc-acm).
 */

#ifndef __BATHOS_USB_DEVICE_H__
#define __BATHOS_USB_DEVICE_H__

#include <stdlib.h>
#include <bathos/usb.h>
#include <bathos/errno.h>
#include <bathos/types.h>
#include <bathos/bitops.h>

struct usb_device_proto;

/*
 * Platform data for high level protocol driver
 * 
 * @controller: controller to talk to
 * @eps: points to an array of pipe names (ep0 is always the control pipe and
 * shall be opened r/w, the other eps are protocol-specific)
 * @neps: number of endpoints
 * @proto_specific: protocol specific platform data
 */
struct usb_device_proto_platform_data {
	const char **eps;
	uint8_t neps;
	void *proto_specific;
};

/*
 * This represents an array of descriptors for the device
 */
struct usb_descriptor_list {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
};

/*
 * Declare a usb_descr_list variable somewhere, setting the last element
 * to addr NULL (this will actually be done by the code generator)
 */
extern const struct usb_descriptor_list usb_descr_list[];


#endif
