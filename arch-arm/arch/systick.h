/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __ARM_SYSTICK_H__
#define __ARM_SYSTICK_H__

#include <bathos/jiffies.h>
#include <bathos/event.h>

#ifdef ARCH_NEEDS_INTERRUPT_FOR_JIFFIES
static inline void inc_jiffies(void)
{
	jiffies++;
}
#else
static inline void inc_jiffies(void)
{
}
#endif

static inline void bathos_arm_sys_tick_timer_handler(void)
{
	inc_jiffies();
	trigger_event(&event_name(hw_timer_tick), NULL);
}

#endif /* __ARM_SYSTICK_H__ */
