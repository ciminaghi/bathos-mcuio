/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/idle.h>

/*
 * Just wait for interrupts
 */
void idle(void)
{
	asm("wfi");
}
