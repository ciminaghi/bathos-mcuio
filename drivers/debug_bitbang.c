/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * From arch-atmega/debug-bitbang.c
 */
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <bathos/gpio.h>
#include <bathos/delay.h>
#include <bathos/debug_bitbang.h>
#include <generated/autoconf.h>
#include <arch/hw.h>

#define CLOCK_GPIO CONFIG_BB_CLOCK_GPIO
#define DATA_GPIO CONFIG_BB_DATA_GPIO

static void __bitbang_putc(char c);

static inline int __init_io(void)
{
	gpio_dir(CLOCK_GPIO, 1, 1);
	gpio_dir(DATA_GPIO, 1, 1);
	return 0;
}

static void __delay(void)
{
	udelay(10);
}

static void __bitbang_putc(char c)
{
	int i, mask = 1;
	for (i = 0; i < 8; i++, mask<<=1) {
		gpio_set(DATA_GPIO, c & mask);
		__delay();
		gpio_set(CLOCK_GPIO, 1);
		__delay();
		gpio_set(CLOCK_GPIO, 0);
	}
}

/* A rising edge on data with stable high clock */
static void __send_start(void)
{
	/* Set clock */
	gpio_set(CLOCK_GPIO, 1);
	__delay();
	/* Clear data */
	gpio_set(DATA_GPIO, 0);
	__delay();
	/* Set data */
	gpio_set(DATA_GPIO, 1);
	__delay();
	/* Clear data */
	gpio_set(DATA_GPIO, 0);
	__delay();
	/* Clear clock */
	gpio_set(CLOCK_GPIO, 0);
}

/*
 * Early puts
 */
void bitbang_puts(char *buf)
{
	__send_start();
	while (*buf)
		__bitbang_putc(*buf++);

}

#if defined CONFIG_CONSOLE_DEBUG_BITBANG

#if defined CONFIG_EARLY_CONSOLE
int console_early_init(void)
{
	return __init_io();
}
#else /* !CONFIG_EARLY_CONSOLE */
core_initcall(__init_io);
#endif

void console_putc(int c)
{
	__send_start();
	__bitbang_putc((char)c);
}
#endif

static int bitbang_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	int i;
	__send_start();
	for (i = 0; i < len; i++)
		__bitbang_putc(buf[i]);
	return len;
}

struct bathos_dev_ops PROGMEM bitbang_dev_ops = {
	/* Only write is implemented ! */
	.write = bitbang_write,
};

