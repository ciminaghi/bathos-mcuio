/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* Instantiate devices for the nrf51822 machine */

#include <bathos/init.h>
#include <bathos/event.h>
#include <bathos/nrf5x-rtc.h>
#include <bathos/nrf5x-uart.h>
#include <bathos/dev_ops.h>
#include <mach/hw.h>


/* Just use rtc0 for jiffies, do not instantiate rtc1 */
static struct nrf5x_rtc_platform_data rtc0_plat = {
	.base = (void *)RTC0_BASE,
	.irq = RTC0_IRQ,
};

static int rtc_init(void)
{
	return nrf5x_rtc_init(&rtc0_plat);
}
core_initcall(rtc_init);

void bathos_ll_int_handler_name(RTC0_IRQ)(struct event_handler_data *data)
{
	nrf5x_irq_handler(&rtc0_plat);
}

static const struct nrf5x_uart_platform_data uart0_plat = {
	.tx_pin = 9,
	.rx_pin = 11,
	.irq = UART0_IRQ,
	.base = UART0_BASE,
};

static struct bathos_dev __udev0;

static struct bathos_dev __udev0
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart0",
	.ops = &nrf5x_uart_dev_ops,
	.platform_data = &uart0_plat,
};

void bathos_ll_int_handler_name(UART0_IRQ)(struct event_handler_data *data)
{
	nrf5x_uart_irq_handler(&__udev0);
}
