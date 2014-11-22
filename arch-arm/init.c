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
}
#endif

#ifdef CONFIG_DEBUG_BITBANG
struct bathos_dev __bitbang_dev __attribute__((section(".bathos_devices"))) = {
	.name = "debug-bitbang",
	.ops = &bitbang_dev_ops,
};
#endif

struct hw_stackframe {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	void *pc;
	uint32_t psr;
};

void __attribute__((naked)) hard_fault_handler(uint32_t lr, void *psp,
					       void *msp)
{
	struct hw_stackframe *frame;

	/* Find the active stack pointer (MSP or PSP) */
	if(lr & 0x4)
		frame = psp;
	else
		frame = msp;

	printf("** HARD FAULT **\r\n pc=%p\r\n  msp=%p\r\n  psp=%p\r\n",
	       frame->pc, msp, psp);

	while(1);
}
