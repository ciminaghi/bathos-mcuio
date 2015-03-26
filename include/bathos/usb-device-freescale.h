/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * Low level usb device controller driver for freescale microcontrollers
 * Original code by Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */
#ifndef __USB_DEVICE_FREESCALE_H__
#define __USB_DEVICE_FREESCALE_H__

#include <stdint.h>

struct __attribute__((packed)) bdt {
/* Flags from CPU to HW */
#define OWN_MASK			BIT(7)
#define DATA01_MASK			BIT(6)
#define KEEP_MASK			BIT(5)
#define NINC_MASK			BIT(4)
#define DTS_MASK			BIT(3)
#define STALL_MASK			BIT(2)
/* Flags from HW to CPU */
#define TOK_PID_SHIFT			2
#define TOK_PID_MASK			0x3c
	uint8_t flags;
	uint8_t rsvd1;
	uint16_t bc;
	union {
		uint32_t l;
		const void *wp;
		void *rp;
	} address;
};

struct usb_device_freescale_buf {
	uint8_t *buf1;
	uint8_t *buf2;
	/*
	 * The ODD flag is used for rx (to do ping pong), while the
	 * BUSY flag is used for tx (to clone non-constant tx bufs).
	 */
#define ODD BIT(0)
/* buf1 is busy */
#define BUSY1 BIT(1)
/* buf2 is busy */
#define BUSY2 BIT(2)
	uint8_t flags;
};

struct usb_device_freescale_platform_data {
	struct bdt *bdt;
	struct usb_device_freescale_buf *bufs;
};

struct usb_device_freescale_ep_ll_data {
	uint8_t tx_data0;
	uint8_t odd;
};

extern struct usb_device_controller_ops PROGMEM \
freescale_usb_device_controller_ops;

extern void usb_device_freescale_irq_handler(struct usb_device_controller *c);

#endif /* __USB_DEVICE_FREESCALE_H__ */
