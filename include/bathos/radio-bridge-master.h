/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __RADIO_BRIDGE_MASTER_H__
#define __RADIO_BRIDGE_MASTER_H__

#include <stdint.h>

typedef int (*rb_master_filter)(const char *buf, int len, void * priv);

struct rb_master_platform_data {
	const char *input_dev;
	const char *output_dev;
	rb_master_filter filter;
	void *filter_data;
	int packet_size;
};

extern const struct bathos_dev_ops master_radio_dev_ops;

/* Call this when beacon time has come, period should be >= 2 * packet time */
extern void rb_master_do_beacon(struct bathos_dev *);

#endif /* __RADIO_BRIDGE_MASTER__ */
