/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <generated/autoconf.h>
#include <arch/hw.h>
#include <bathos/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

/* Data PB4, clock PB5 */

#define DATA_PORT PORTB
#define DATA_DIR DDRB
#define DATA_PIN 4


#define CLOCK_PORT PORTB
#define CLOCK_DIR DDRB
#define CLOCK_PIN 5

static void __bitbang_putc(char c);

static inline int __init_io(void)
{
	//DATA_DIR |= (1 << DATA_PIN);
	//CLOCK_DIR |= (1 << CLOCK_PIN);
	asm("sbi 4,5");
	asm("sbi 4,4");
	asm("cbi 5,5");
	asm("cbi 5,4");
	return 0;
}

static void __delay(void)
{
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
}

static inline void __set_gpio(int port, int pin)
{
	//port |= (1 << pin);
	
}

static inline void __clear_gpio(int port, int pin)
{
	//port &= ~(1 << pin);
}

static void __bitbang_putc(char c)
{
	int i, mask = 1;
	for (i = 0; i < 8; i++, mask<<=1) {
		if (c & mask)
			//__set_gpio(DATA_PORT, DATA_PIN);
			asm("sbi 5,5");
		else
			//__clear_gpio(DATA_PORT, DATA_PIN);
			asm("cbi 5,5");
		__delay();
		//__set_gpio(CLOCK_PORT, CLOCK_PIN);
		asm("sbi 5,4");
		__delay();
		//__clear_gpio(CLOCK_PORT, CLOCK_PIN);
		asm("cbi 5,4");
	}
}

/* A rising edge on data with stable high clock */
static void __send_start(void)
{
	/* Set clock */
	asm("sbi 5,4");
	__delay();
	/* Clear data */
	asm("cbi 5,5");
	__delay();
	/* Set data */
	asm("sbi 5,5");
	__delay();
	/* Clear data */
	asm("cbi 5,5");
	__delay();
	/* Clear clock */
	asm("cbi 5,4");
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

#if defined CONFIG_CONSOLE_DEBUG_BITBANG && defined CONFIG_EARLY_CONSOLE
int console_early_init(void)
{
	return __init_io();
}

void console_putc(int c)
{
	int flags;
	interrupt_disable(flags);
	__send_start();
	__bitbang_putc((char)c);
	interrupt_restore(flags);
}
#else
rom_initcall(__init_io);
#endif


static int bitbang_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	int i;
	__send_start();
	for (i = 0; i < len; i++)
		__bitbang_putc(buf[i]);
	return len;
}

static struct bathos_dev_ops PROGMEM bitbang_dev_ops = {
	/* Only write is implemented ! */
	.write = bitbang_write,
};

struct bathos_dev __bitbang_dev __attribute__((section(".bathos_devices"))) = {
	.name = "debug-console",
	.ops = &bitbang_dev_ops,	
};
