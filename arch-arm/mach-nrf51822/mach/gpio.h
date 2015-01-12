/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __GPIO_MACH_NRF51822_H__
#define __GPIO_MACH_NRF51822_H__

#include <bathos/io.h>
#include "hw.h"

static inline void gpio_set(int gpio, int value)
{
	int pin = GPIO_BIT(gpio);

	if (value)
		regs[REG_GPIO_OUTSET] = (1 << pin);
	else
		regs[REG_GPIO_OUTCLR] = (1 << pin);
}

static inline void gpio_dir(int gpio, int output, int value)
{
	int pin = GPIO_BIT(gpio);

	gpio_set(gpio, value);

	if (output)
		regs[REG_GPIO_DIRSET] = (1 << pin);
	else {
		uint32_t v = PIN_CNF_DIR_IN |
			PIN_CNF_IN_CON |
			PIN_CNF_S0S1 |
			PIN_CNF_SNS_DIS;
		if (value)
			v |= PIN_CNF_PULLDN;
		regs[REG_PIN_CNF(pin)] = v;
	}
}

static inline int gpio_get(int gpio)
{
	int pin = GPIO_BIT(gpio);

	return regs[REG_GPIO_IN] & (1 << pin);
}

static inline int gpio_dir_af(int gpio, int output, int value, int afnum)
{
	if (afnum)
		return -EINVAL;
	gpio_dir(gpio, output, value);
	return 0;
}


static inline int gpio_get_dir_af(int gpio, int *output, int *value, int *afnum)
{
	int pin = GPIO_BIT(gpio);

	if (output)
		*output = regs[REG_GPIO_DIR] & (1 << pin);
	if (afnum)
		*afnum = 0;
	if (value)
		*value = gpio_get(gpio);
	return 0;
}

extern void gpio_init(void);


#endif /* __GPIO_MACH_NRF51822_H__ */

