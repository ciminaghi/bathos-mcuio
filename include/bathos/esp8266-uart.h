/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#ifndef __ESP8266_UART_H__
#define __ESP8266_UART_H__

#include <stdint.h>
#include <bathos/dev_ops.h>

struct esp8266_uart_priv;

struct esp8266_uart_platform_data {
	int index;
	uint32_t base;
	void (*pin_setup)(const struct esp8266_uart_platform_data *);
	int rx_fifo_threshold;
};

extern const struct bathos_dev_ops PROGMEM esp8266_uart_dev_ops;
extern void esp8266_uart_irq_handler(struct bathos_dev *);
extern
void esp8266_uart_console_putc(const struct esp8266_uart_platform_data *plat,
			       const char c);
extern int
esp8266_uart_console_init(const struct esp8266_uart_platform_data *plat);

/*
 * FIXME: this is provided by the sdk, get rid of it
 */
extern void uart_div_modify(int no, unsigned int freq);


#endif /* __ESP8266_UART_H__ */
