/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <bathos/init.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/sys_timer.h>
#include <bathos/irq.h>
#include <arch/hw.h>
#include <mach/hw.h>

void bathos_int_handler_name(NVIC_IRQ_PIT)(void)
{
	jiffies++;
	regs[REG_PIT_TFLG(0)] = PIT_TFLG_TIF_MASK;
	trigger_event(&event_name(hw_timer_tick), NULL);
}

void jiffies_init(void)
{
	/* Clock and enable PIT */
	regs[REG_SCGC6] |= SIM_SCGC6_PIT_MASK;
	regs[REG_PIT_MCR] = 0;

	/* Use channel 0 to generate irqs for jiffies update */
	regs[REG_PIT_LDVAL(0)] = BUS_FREQ / HZ - 1;
	regs[REG_PIT_TCTRL(0)] = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	bathos_enable_irq(NVIC_IRQ_PIT);
}
