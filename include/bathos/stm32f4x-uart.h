/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#ifndef __STM32F4X_UART_H__
#define __STM32F4X_UART_H__

#include <stdint.h>
#include <bathos/dev_ops.h>

struct stm32f4x_uart_priv;

struct stm32f4x_uart_platform_data {
	uint8_t tx_pin;
	uint8_t tx_af;
	uint8_t rx_pin;
	uint8_t rx_af;
	int irq;
	uint32_t base;
	unsigned long clock;
	int bus;
	int periph_id;
};

extern const struct bathos_dev_ops PROGMEM stm32f4x_uart_dev_ops;
extern void stm32f4x_uart_irq_handler(struct bathos_dev *);
extern int stm32f4x_console_putc(const struct stm32f4x_uart_platform_data *plat,
				 const char c);
extern int
stm32f4x_uart_console_init(const struct stm32f4x_uart_platform_data *plat);

#endif /* __STM32F4X_UART_H__ */
