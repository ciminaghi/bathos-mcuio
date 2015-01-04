/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <avr/io.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/gpio.h>
#include <bathos/bitops.h>
#include <bathos/errno.h>

static uint8_t ports[5];
static uint8_t rising[5];
static uint8_t falling[5];
static uint8_t enable[5];

static uint32_t events[2];

declare_event(gpio_evt);

static void gpio_sample(struct event_handler_data *__d)
{
	int i, j;
	uint8_t s, d, m;
	int addr, do_trigger = 0;
	static uint8_t initialized = 0;
	for (i = 0; i < sizeof(ports); i++) {
		addr = PORTS_BASE + i * 3;
		s = _SFR_IO8(addr);
		if (!(initialized & (1 << i))) {
			initialized |= (1 << i);
			goto next;
		}
		d = s ^ ports[i];
		for (m = 1, j = 0; j < 8; m <<= 1, j++) {
			if (!(enable[i] & m) || !(d & m))
				continue;
			if ((rising[i] & m) && !(ports[i] & m)) {
				/* Rising edge */
				do_trigger = 1;
				set_bit(i * 8 + j, events);
			}
			if ((falling[i] & m) && (ports[i] & m)) {
				/* Falling edge */
				do_trigger = 1;
				set_bit(i * 8 + j, events);
			}
		}
	next:
		/* Store previous status */
		ports[i] = s;
	}
	if (do_trigger)
		trigger_event(&event_name(gpio_evt), events);
}

declare_event_handler(hw_timer_tick, NULL, gpio_sample, NULL);

int gpio_request_events(int gpio, int flags)
{
	uint8_t mask, port;
	if (gpio < 0 || gpio > 39)
		return -EINVAL;
	mask = 1 << GPIO_BIT(gpio);
	port = GPIO_PORT(gpio);
	rising[port] &= ~mask;
	falling[port] &= ~mask;
	enable[port] &= ~mask;
	if (flags & GPIO_EVT_RISING)
		rising[port] |= mask;
	if (flags & GPIO_EVT_FALLING)
		falling[port] |= mask;
	if (flags & GPIO_EVT_ENABLE)
		enable[port] |= mask;
	if ((flags & GPIO_EVT_LOW) | (flags & GPIO_EVT_HIGH)) {
		rising[port] |= mask;
		falling[port] |= mask;
	}
	return 0;
}
