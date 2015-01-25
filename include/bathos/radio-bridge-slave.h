/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __RADIO_BRIDGE_SLAVE_H__
#define __RADIO_BRIDGE_SLAVE_H__

#include <stdint.h>

struct rb_slave_platform_data {
	const char *input_dev;
	int packet_size;
};

extern const struct bathos_dev_ops slave_radio_dev_ops;

#endif /* __RADIO_BRIDGE_MASTER__ */
