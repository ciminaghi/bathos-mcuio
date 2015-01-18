/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>, original code by
 * Adriano Costanzo <>
 */

#ifndef __NRF5X_UART_H__
#define __NRF5X_UART_H__

#include <stdint.h>
#include <bathos/dev_ops.h>

struct nrf5x_uart_priv;

struct nrf5x_uart_platform_data {
	uint8_t tx_pin;
	uint8_t rx_pin;
	int irq;
	uint32_t base;
};

extern const struct bathos_dev_ops PROGMEM nrf5x_uart_dev_ops;
extern void nrf5x_uart_irq_handler(struct bathos_dev *);
extern int nrf5x_console_putc(const struct nrf5x_uart_platform_data *plat,
			      const char c);
extern int nrf5x_uart_console_init(const struct nrf5x_uart_platform_data *plat);

#endif /* __NRF5X_UART_H__ */
