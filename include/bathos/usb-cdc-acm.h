/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 * Original code by Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#ifndef __BATHOS_USB_CDC_ACM_H__
#define __BATHOS_USB_CDC_ACM_H__

#include <stdint.h>

struct event;

enum cdc_ep {
	CONTROL = 0,
	ACM,
	RX,
	TX,
	NEPS,
};

/*
 * usb-cdc data structure related to a single instance
 * usb-cdc has no dynamic allocation, pass a pointer to a suitable buffer
 * along with platform data.
 */
struct usb_cdc_data {
	int open_mode;
	struct bathos_dev *dev;
	struct bathos_dev_data *dev_data;
	struct bathos_pipe *eps[NEPS];
	const struct event *control_evt;
	const struct event *data_evt;
};

extern const struct bathos_dev_ops usb_cdc_acm_dev_ops;

#endif /* __BATHOS_USB_CDC_ACM_H__ */
