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
#include <bathos/nrf5x-radio.h>
#include <bathos/dev_ops.h>
#include <bathos/pipe.h>
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

#ifndef CONFIG_NRF51822_MY_RADIO_ADDR
#define CONFIG_NRF51822_MY_RADIO_ADDR 1
#endif
#ifndef CONFIG_NRF51822_DST_RADIO_ADDR
#define CONFIG_NRF51822_DST_RADIO_ADDR 2
#endif

static union {
	struct nrf5x_radio_packet radio_packet;
	/* Get space for 16bytes payload */
	uint8_t dummy[sizeof(struct nrf5x_radio_packet) + 16];
} radio_packet_area;

static const uint8_t my_radio_addr[] = {
	0,
	CONFIG_NRF51822_MY_RADIO_ADDR,
};

static const uint8_t dst_radio_addr[] = {
	0,
	CONFIG_NRF51822_DST_RADIO_ADDR,
};

static const struct nrf5x_radio_platform_data radio_plat = {
	.irq = RADIO_IRQ,
	.base = RADIO_BASE,
	.addr_length = ARRAY_SIZE(my_radio_addr),
	.my_addr = my_radio_addr,
	.dst_addr = dst_radio_addr,
	.packet_area = &radio_packet_area.radio_packet,
	.packet_size = 16,
};

static struct bathos_dev __rdev0;

static struct bathos_dev __rdev0
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "radio",
	.ops = &nrf5x_radio_dev_ops,
	.platform_data = &radio_plat,
};

void bathos_ll_int_handler_name(RADIO_IRQ)(struct event_handler_data *data)
{
	nrf5x_radio_irq_handler(&__rdev0);
}
