/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* Instantiate devices for the nrf51822 machine */

#include <bathos/init.h>
#include <bathos/event.h>
#include <bathos/nrf51-rtc.h>
#include <bathos/nrf51-uart.h>
#include <bathos/dev_ops.h>
#include <mach/hw.h>


/* Just use rtc0 for jiffies, do not instantiate rtc1 */
static struct nrf51_rtc_platform_data rtc0_plat = {
	.base = (void *)RTC0_BASE,
	.irq = RTC0_IRQ,
};

static int rtc_init(void)
{
	return nrf51_rtc_init(&rtc0_plat);
}
core_initcall(rtc_init);

void bathos_ll_int_handler_name(RTC0_IRQ)(struct event_handler_data *data)
{
	nrf51_irq_handler(&rtc0_plat);
}

static const struct nrf51_uart_platform_data uart0_plat = {
	.tx_pin = 10,
	.rx_pin = 11,
	.irq = UART0_IRQ,
	.base = UART0_BASE,
};

static struct bathos_dev __udev0;

static struct nrf51_uart_priv uart0_priv = {
	.dev = &__udev0,
	.platform_data = &uart0_plat,
};

static struct bathos_dev __udev0
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart0",
	.ops = &nrf51_uart_dev_ops,
	.priv = &uart0_priv,
};

void bathos_ll_int_handler_name(IRQ_UART0)(struct event_handler_data *data)
{
	nrf51_uart_irq_handler(&uart0_priv);
}
