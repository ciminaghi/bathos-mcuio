/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#ifndef __ESP8266_WLAN_H__
#define __ESP8266_WLAN_H__

#include <stdint.h>
#include <bathos/dev_ops.h>

struct esp8266_wlan_priv;

struct esp8266_wlan_platform_data {
	int nbufs;
	int bufsize;
};

extern const struct bathos_dev_ops PROGMEM esp8266_wlan_dev_ops;

#endif /* __ESP8266_UART_H__ */
