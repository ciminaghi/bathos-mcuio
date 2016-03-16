/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <generated/autoconf.h>
#include <arch/scb-cortex-m.h>

#define NVIC_CORE_NUM		16

#define NVIC_IRQ_NMI 		( 2 - NVIC_CORE_NUM)
#define NVIC_IRQ_HARD_FAULT 	( 3 - NVIC_CORE_NUM)
#define NVIC_IRQ_SVCALL 	(11 - NVIC_CORE_NUM),
#define NVIC_IRQ_PENDSV		(14 - NVIC_CORE_NUM),
#define NVIC_IRQ_SYSTICK	(15 - NVIC_CORE_NUM),
