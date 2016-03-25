/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __STM32F4X_GPIO_H__
#define __STM32F4X_GPIO_H__

#include <bathos/io.h>
#include <mach/hw.h>
#include <family/clocks.h>

#define GPIO_PORT_RANGE 0x400

#define GPIO_MODER	0x00
#define GPIO_OTYPER	0x04
#define GPIO_OSPEEDR	0x08
#define GPIO_PUPDR	0x0c
#define GPIO_IDR	0x10
#define GPIO_ODR	0x14
#define GPIO_BSRR	0x18
#define GPIO_LCKR	0x1c
#define GPIO_AFRL	0x20
#define GPIO_AFRH	0x24

enum pin_af {
	GPIO = 0,
	AF00 = 1,
	AF01 = 2,
	AF02 = 3,
	AF03 = 4,
	AF04 = 5,
	AF05 = 6,
	AF06 = 7,
	AF07 = 8,
	AF08 = 9,
	AF09 = 10,
	AF10 = 11,
	AF11 = 12,
	AF12 = 13,
	AF13 = 14,
	AF14 = 15,
	AF15 = 16,
	ANALOG = 17,
};

static inline void gpio_set(int gpio, int value)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio) + value ? 0 : 16;

	writel(1 << bit, GPIO_BASE + GPIO_BSRR + port * GPIO_PORT_RANGE);
}

static inline void gpio_dir(int gpio, int output, int value)
{
	int port = GPIO_PORT(gpio);
	int shift = GPIO_BIT(gpio) << 1;
	int mask = 0x3 << shift;
	uint32_t tmp;

	gpio_set(gpio, value);
	tmp = readl(GPIO_BASE + GPIO_MODER + port * GPIO_PORT_RANGE);
	tmp &= ~mask;
	if (output)
		tmp |= 1 << shift;
	writel(tmp, GPIO_BASE + GPIO_MODER + port * GPIO_PORT_RANGE);
}

static inline int gpio_get(int gpio)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);

	return !!(readl(GPIO_BASE + GPIO_IDR + port * GPIO_PORT_RANGE) &
		  (1 << bit));
}

static void _enable_gpio_port(int port)
{
	/* Port ids are numerically equal to peripheral ids */
	stm32f4x_enable_peripheral_clock(AHB1, port);
}

static inline int gpio_dir_af(int gpio, int output, int value, int afnum)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);
	int offs = GPIO_AFRL + ((bit >= 8) ? 4 : 0);
	int shift, mask;
	uint32_t tmp, addr;

	if (afnum < GPIO || afnum > ANALOG)
		return -1;
	_enable_gpio_port(port);
	switch (afnum) {
	case GPIO:
		gpio_dir(gpio, output, value);
		break;
	case AF00:
	case AF01:
	case AF02:
	case AF03:
	case AF04:
	case AF05:
	case AF06:
	case AF07:
	case AF08:
	case AF09:
	case AF10:
	case AF11:
	case AF12:
	case AF13:
	case AF14:
	case AF15:
		shift = 4 * (bit & 0x7);
		mask = 0xf << shift;
		tmp = readl(GPIO_BASE + offs + port * GPIO_PORT_RANGE);
		tmp &= ~mask;
		tmp |= (afnum - AF00) << shift;
		writel(tmp, GPIO_BASE + offs + port * GPIO_PORT_RANGE);
		/* FALL THROUGH */
	case ANALOG:
		shift = bit << 1;
		mask = 3 << shift;
		addr = GPIO_BASE + GPIO_MODER + port * GPIO_PORT_RANGE;
		tmp = readl(addr);
		tmp &= ~mask;
		tmp |= (afnum == ANALOG ? 3 : 2) << shift;
		writel(tmp, addr);
		break;
	default:
		return -1;
	}
	return 0;
}


static inline int gpio_get_dir_af(int gpio, int *output, int *value, int *afnum)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);

	if (*output) {
		int shift = GPIO_BIT(gpio) << 1;
		int mask = 0x3 << shift;

		*output = !!(readl(GPIO_BASE + GPIO_MODER +
				   port * GPIO_PORT_RANGE) & mask);
	}
	if (*value)
		*value = gpio_get(gpio);
	if (*afnum) {
		int shift = 4 * (bit & 0x7);
		int mask = 0xf << shift;
		int offs = GPIO_AFRL + ((bit >= 8) ? 4 : 0);

		*afnum = (readl(GPIO_BASE + offs +
				port * GPIO_PORT_RANGE) & mask) >> shift;
	}
	return 0;
}

#endif /* __STM32F4X_GPIO_H__ */
