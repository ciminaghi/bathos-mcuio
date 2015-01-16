/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* nrf51 family clock management */

#include <stdint.h>
#include <bathos/io.h>
#include <bathos/init.h>
#include <arch/hw.h>
#include <mach/hw.h>
#include <generated/autoconf.h>

#define CLK_BASE 0x40000000

#define CLK_TASK_HFCLKSTART 0x000
#define CLK_TASK_LFCLKSTART 0x008

#define CLK_EVT_HFCLKSTARTED 0x100
#define CLK_EVT_LFCLKSTARTED 0x104


void __clocks_init(uint32_t b)
{
	writel(1, b + CLK_TASK_HFCLKSTART);
	while(!readl(b + CLK_EVT_HFCLKSTARTED))
		;
	writel(1, b + CLK_TASK_LFCLKSTART);
	while(!readl(b + CLK_EVT_LFCLKSTARTED))
		;
}

/*
 * Preliminary support. For now just start low and high frequency clocks
 * Works on redbearlab nrf51822
 */
void clocks_init(void)
{
	return __clocks_init(CLK_BASE);
}

