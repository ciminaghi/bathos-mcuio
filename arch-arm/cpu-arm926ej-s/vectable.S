/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <generated/autoconf.h>


.section .vectors, "x"
.extern _bathos_start

	b _bathos_start
	b . /* Undefined */
	b . /* SWI */
	b . /* Prefetch Abort */
	b . /* Data Abort */
	b . /* reserved */
	b versatile_irq_handler /* IRQ */
	b . /* FIQ, unsupported at the moment */
