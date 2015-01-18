/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>, original code by
 * Adriano Costanzo <>
 */

#ifndef __NRF51_UART_H__
#define __NRF51_UART_H__

#include <stdint.h>

struct nrf51_uart_priv;

struct nrf51_uart_platform_data {
	uint8_t tx_pin;
	uint8_t rx_pin;
	int irq;
	uint32_t base;
};

struct nrf51_uart_priv {
	struct bathos_dev *dev;
	struct bathos_dev_data *dev_data;
	const struct nrf51_uart_platform_data *platform_data;
};

extern const struct bathos_dev_ops PROGMEM nrf51_uart_dev_ops;
extern void nrf51_uart_irq_handler(void *_priv);

#endif /* __NRF51_UART_H__ */
