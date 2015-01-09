/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>, original code by
 * Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */
#include <generated/autoconf.h>
#include <mach/hw.h>
#include <bathos/freescale-uart.h>
#include <bathos/gpio.h>

static const struct freescale_uart_platform_data uart0_pdata = {
	.index = 0,
	.tx_pin = GPIO_NR(PORTE, 20),
	.rx_pin = GPIO_NR(PORTE, 21),
	.irq = NVIC_IRQ_UART0,
	.afnum = 4,
	.clock = CONFIG_KL25Z_BUS_FREQ,
	.base = REG_UART0_BASE, 0x4006a000,
};

static const struct freescale_uart_platform_data uart1_pdata = {
	.index = 1,
	.tx_pin = GPIO_NR(PORTE, 0),
	.rx_pin = GPIO_NR(PORTE, 1),
	.irq = NVIC_IRQ_UART1,
	.afnum = 3,
	.clock = CONFIG_KL25Z_BUS_FREQ,
	.base = REG_UART1_BASE, 0x4006b000,
};

static const struct freescale_uart_platform_data uart2_pdata = {
	.index = 2,
	.tx_pin = GPIO_NR(PORTE, 22),
	.rx_pin = GPIO_NR(PORTE, 23),
	.irq = NVIC_IRQ_UART2,
	.afnum = 3,
	.clock = CONFIG_KL25Z_BUS_FREQ,
	.base = REG_UART2_BASE, 0x4006c000,
};

static struct bathos_dev __udev0, __udev1, __udev2;

static struct bathos_dev __udev0
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart0",
	.ops = &freescale_uart_dev_ops,
	.platform_data = &uart0_pdata,
};

static struct bathos_dev __udev1
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart1",
	.ops = &freescale_uart_dev_ops,
	.platform_data = &uart1_pdata,
};

static struct bathos_dev __udev2
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart2",
	.ops = &freescale_uart_dev_ops,
	.platform_data = &uart2_pdata,
};

void bathos_ll_int_handler_name(NVIC_IRQ_UART0)(struct event_handler_data *data)
{
	freescale_uart_irq_handler(&__udev0);
}

void bathos_ll_int_handler_name(NVIC_IRQ_UART1)(struct event_handler_data *data)
{
	freescale_uart_irq_handler(&__udev1);
}

void bathos_ll_int_handler_name(NVIC_IRQ_UART2)(struct event_handler_data *data)
{
	freescale_uart_irq_handler(&__udev2);
}
