/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __BATHOS_ARCH_CORTEX_M__
#define __BATHOS_ARCH_CORTEX_M__


#define interrupt_disable(flags)					\
	do {								\
		asm("mrs %[out], primask": [out] "=r"(flags) : : );	\
	} while(0);

#define interrupt_restore(flags)					\
	do {								\
		asm("msr primask, %[in]" : : [in] "r"(flags) : );	\
	} while(0);

/*
 * No way to read jiffies from hw in case the standard CORTEX-M sys tick timer
 * is used
 */
#ifdef CONFIG_HAS_CORTEXM_SYSTICKTMR
#define ARCH_NEEDS_INTERRUPTS_FOR_JIFFIES
#endif

#endif /* __BATHOS_ARCH_CORTEX_M__ */
