/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/sys_timer.h>

static inline void bathos_arm_sys_tick_timer_handler(void)
{
	jiffies++;
	trigger_event(&event_name(hw_timer_tick), NULL);
}
