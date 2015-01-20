/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __GPIO_MACH_KLZ25_H__
#define __GPIO_MACH_KLZ25_H__

#include <bathos/io.h>
#include "hw.h"

#define HAVE_GPIO_TO_AF 1

/* Alternate function for gpios is 1 */
static inline int gpio_to_af(int gpio)
{
	return 1;
}

static inline void gpio_set(int gpio, int value)
{
	int port = GPIO_PORT(gpio);
	int pin = GPIO_BIT(gpio);

	regs[REG_SIM_SCGC5] |= PORT_CG(port);
	if (value)
		regs[REG_GPIO_PSOR(port)] = (1 << pin);
	else
		regs[REG_GPIO_PCOR(port)] = (1 << pin);
}

static inline void gpio_dir(int gpio, int output, int value)
{
	int port = GPIO_PORT(gpio);
	int pin = GPIO_BIT(gpio);

	gpio_set(gpio, value);

	if (output)
		regs[REG_GPIO_PDDR(port)] |= (1 << pin);
	else
		regs[REG_GPIO_PDDR(port)] &= ~(1 << pin);
}

static inline int gpio_get(int gpio)
{
	int port = GPIO_PORT(gpio);
	int pin = GPIO_BIT(gpio);

	regs[REG_SIM_SCGC5] |= PORT_CG(port);
	return regs[REG_GPIO_PDIR(port)] & (1 << pin);
}

#define AFNUM_MASK 7
#define AFNUM_SHIFT 8

static inline int gpio_dir_af(int gpio, int output, int value, int afnum)
{
	int port = GPIO_PORT(gpio);
	int pin = GPIO_BIT(gpio);
	u32 tmp;

	gpio_dir(gpio, output, value);
	tmp = regs[REG_PORT_PCR(port, pin)];
	tmp &= ~(AFNUM_MASK << AFNUM_SHIFT);
	tmp |= (afnum & AFNUM_MASK) << AFNUM_SHIFT;
	regs[REG_PORT_PCR(port, pin)] = tmp;
	gpio_dir(gpio, output, value);
	return 0;
}


static inline int gpio_get_dir_af(int gpio, int *output, int *value, int *afnum)
{
	int port = GPIO_PORT(gpio);
	int pin = GPIO_BIT(gpio);

	if (output)
		*output = regs[REG_GPIO_PDIR(port)] & (1 << pin);
	if (afnum)
		*afnum = (regs[REG_PORT_PCR(port, pin)] >> AFNUM_SHIFT) &
		    AFNUM_MASK;
	if (value)
		*value = gpio_get(gpio);
	return 0;
}

extern void gpio_init(void);

#endif /* __GPIO_MACH_KLZ25_H__ */
