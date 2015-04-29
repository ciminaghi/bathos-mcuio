/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * Platform file for kl25 usb controller in slave mode
 * We just instantiate a device controller here
 */
#include <bathos/usb.h>
#include <bathos/usb-device.h>
#include <bathos/usb-device-controller.h>
#include <bathos/usb-device-freescale.h>
#include <mach/hw.h>

/* These get defined in drivers/usb-data.c, automatically generated */
extern struct bdt freescale_bdt[];
extern struct usb_device_freescale_buf freescale_bufs[];

static const struct usb_device_freescale_platform_data freescale_platdata = {
	.bdt = freescale_bdt,
	.bufs = freescale_bufs,
};

static const struct usb_device_controller_platform_data controller_platdata = {
	.ops = &freescale_usb_device_controller_ops,
	.device_specific = &freescale_platdata,
};

struct usb_device_controller USB_DEVICE_CONTROLLER_NAME(0) = {
	.plat = &controller_platdata,
};

/*
 * Seems to work with handler in non-interrupt context
 */
void bathos_int_handler_name(IRQ_USBOTG)(struct event_handler_data *data)
{
	usb_device_freescale_irq_handler(&USB_DEVICE_CONTROLLER_NAME(0));
}
