/*
 * Copyright (c) Davide Cimianghi
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#include <generated/autoconf.h>
#include <mach/hw.h>
#include <bathos/pl011-uart.h>
#include <bathos/gpio.h>
#include <bathos/init.h>

#define UART0_BASE 0x101f1000
#define UART1_BASE 0x101f2000
#define UART2_BASE 0x101f3000

#define VERSATILE_BUS_FREQ 1000000

#warning LA SCHEDA E' VERSATILE AB !!

static const struct pl011_uart_platform_data uart0_pdata = {
	.index = 0,
	.irq = IRQ_UART0,
	.clock = VERSATILE_BUS_FREQ,
	.base = UART0_BASE,
};

static const struct pl011_uart_platform_data uart1_pdata = {
	.index = 1,
	.irq = IRQ_UART1,
	.clock = VERSATILE_BUS_FREQ,
	.base = UART1_BASE,
};

static const struct pl011_uart_platform_data uart2_pdata = {
	.index = 2,
	.irq = IRQ_UART2,
	.clock = VERSATILE_BUS_FREQ,
	.base = UART2_BASE,
};

static struct bathos_dev __udev0, __udev1, __udev2;

static struct bathos_dev __udev0
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart0",
	.ops = &pl011_uart_dev_ops,
	.platform_data = &uart0_pdata,
};

static struct bathos_dev __udev1
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart1",
	.ops = &pl011_uart_dev_ops,
	.platform_data = &uart1_pdata,
};

static struct bathos_dev __udev2
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart2",
	.ops = &pl011_uart_dev_ops,
	.platform_data = &uart2_pdata,
};

void bathos_ll_int_handler_name(IRQ_UART0)(struct event_handler_data *data)
{
	pl011_uart_irq_handler(&__udev0);
}

void bathos_ll_int_handler_name(IRQ_UART1)(struct event_handler_data *data)
{
	pl011_uart_irq_handler(&__udev1);
}

void bathos_ll_int_handler_name(IRQ_UART2)(struct event_handler_data *data)
{
	pl011_uart_irq_handler(&__udev2);
}

#ifdef CONFIG_CONSOLE_UART0
int console_init(void)
{
	return pl011_uart_console_init(&uart0_pdata);
}

void console_putc(int c)
{
	pl011_uart_console_putc(UART0_BASE, c);
}
#endif

#ifdef CONFIG_CONSOLE_UART1
int console_init(void)
{
	return pl011_uart_console_init(&uart1_pdata);
}

void console_putc(int c)
{
	pl011_uart_console_putc(UART1_BASE, c);
}
#endif

#ifdef CONFIG_CONSOLE_UART2
int console_init(void)
{
	return pl011_uart_console_init(&uart2_pdata);
}

void console_putc(int c)
{
	pl011_uart_console_putc(UART2_BASE, c);
}
#endif

