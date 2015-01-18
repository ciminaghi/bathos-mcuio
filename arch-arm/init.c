/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/init.h>
#include <bathos/pipe.h>
#include <bathos/shell.h>
#include <bathos/debug_bitbang.h>
#include <bathos/delay.h>
#include <generated/autoconf.h>
#include <arch/hw.h>
#include <bathos/gpio.h>

#ifndef CONFIG_STDOUT
#define CONFIG_STDOUT "null"
#endif
#ifndef CONFIG_STDIN
#define CONFIG_STDIN "null"
#endif

int stdio_init(void)
{
	bathos_stdout = pipe_open(CONFIG_STDOUT, BATHOS_MODE_OUTPUT, NULL);
	bathos_stdin = pipe_open(CONFIG_STDIN, BATHOS_MODE_INPUT, NULL);
	return 0;
}
core_initcall(stdio_init);

#if defined CONFIG_EARLY_CONSOLE
int bathos_setup(void)
{
	console_early_init();
	console_putc('.');
	do_initcalls();
	return 0;
}
#endif

/*
 * We get here after bathos setup, but with interrupts enabled
 * This is the place were initializations needing interrupts enabled
 * can be done
 */
int arm_bathos_main(void)
{
	if (udelay_init() < 0)
		printf("Warning: error in udelay init\n");
	return bathos_main();
}

#ifdef CONFIG_DEBUG_BITBANG
struct bathos_dev __bitbang_dev __attribute__((section(".bathos_devices"))) = {
	.name = "debug-bitbang",
	.ops = &bitbang_dev_ops,
};
#endif
