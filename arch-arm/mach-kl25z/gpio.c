/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <arch/hw.h>
#include <mach/hw.h>
#include <arch/bathos-arch.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/gpio.h>
#include <bathos/bitops.h>
#include <bathos/errno.h>
#include <bathos/init.h>
#include <bathos/irq.h>

#define IRQC_MASK 0xf
#define IRQC_SHIFT 16

static uint32_t enable[5];
static uint32_t events[5];

/* Each irqc is 4 bits, save them as nibbles */
static uint8_t porta_irqc[16];
static uint8_t portd_irqc[16];

declare_event(gpio_evt);

static void _gpio_irq_handler(int port)
{
	int i, j;
	uint32_t s;
	uint32_t m;
	int do_trigger = 0;
	s = regs[REG_PORT_ISFR(port)];
	for (j = 0; j < 32; j++) {
		m = 1 << j;
		if ((s & m) && (enable[port] & m)) {
			do_trigger = 1;
			set_bit(port * 8 + j, events);
		}
		regs[REG_PORT_ISFR(port)] |= (1 << j);
	}
	if (do_trigger)
		trigger_event(&event_name(gpio_evt), events);
}

void bathos_int_handler_name(IRQ_PORTA)(struct event_handler_data *d)
{
	_gpio_irq_handler(PORTA);
}

void bathos_int_handler_name(IRQ_PORTD)(struct event_handler_data *d)
{
	_gpio_irq_handler(PORTD);
}

static int gpio_irq_init(void)
{
	bathos_enable_irq(IRQ_PORTD);
	bathos_enable_irq(IRQ_PORTA);
	return 0;
}
core_initcall(gpio_irq_init);

int gpio_request_events(int gpio, int flags)
{
	unsigned int mask, port;
	unsigned int pin = GPIO_BIT(gpio);
	unsigned int _shft;
	unsigned int addr;
	static uint8_t *irqc;

	port = GPIO_PORT(gpio);

	/* Interrupt is supported by hw on ports A and D only */
	if (port == PORTA)
		irqc = &porta_irqc[pin / 2];
	else if (port == PORTD)
		irqc = &portd_irqc[pin / 2];
	else
		return -EINVAL;

	mask = 1 << pin;
	_shft = pin % 2 ? 4 : 0;
	enable[port] &= ~mask;
	*irqc &= ~(0xf << _shft);

	if (flags & GPIO_EVT_ENABLE)
		enable[port] |= mask;
	if (flags & GPIO_EVT_RISING)
		*irqc |= 0x9 << _shft;
	if (flags & GPIO_EVT_FALLING)
		*irqc |= 0xa << _shft;
	if (flags & GPIO_EVT_LOW)
		*irqc |= 0x8 << _shft;
	if (flags & GPIO_EVT_HIGH)
		*irqc |= 0xc << _shft;

	addr = REG_PORT_PCR(port, pin);

	if ((enable[port] & mask) == 0)
		regs[addr] &= ~(IRQC_MASK << IRQC_SHIFT);
	else {
		int d, v;

		/* Set gpio alternative function */
		/* FIXME: review this when irq ctrl will be implemented */
		gpio_get_dir_af(gpio, &d, &v, NULL);
		gpio_dir_af(gpio, d, v, 1);
		regs[addr] |=
			((uint32_t)((*irqc) >> _shft)) << IRQC_SHIFT;
		regs[addr] |= (1 << 24);
	}
	return 0;
}
