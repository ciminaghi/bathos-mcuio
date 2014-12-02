/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <bathos/io.h>
#include <bathos/types.h>
#include "hw.h"

#ifndef ___CORTEX_M0_NVIC_H__
#define ___CORTEX_M0_NVIC_H__

extern volatile uint32_t rom_vectors[];
extern volatile uint32_t ram_vectors[];
extern volatile uint32_t *vectors;

#define interrupt_disable(flags) \
	do { \
		flags = regs[REG_NVIC_ISER]; \
		regs[REG_NVIC_ICER] = 0xffffffff; \
	} while(0)

#define interrupt_restore(flags) \
	regs[REG_NVIC_ISER] = flags

static inline void nvic_init_ram()
{
	int i;
	for (i = 0; i < NVIC_CORE_NUM + NVIC_USER_NUM; i++)
		ram_vectors[i] = rom_vectors[i];
	vectors = ram_vectors;
	regs[REG_SCB_VTOR] = (uint32_t)vectors;
}

static inline void nvic_set_handler(uint32_t irqn, void (*irq_handler)(void))
{
	vectors[irqn + NVIC_CORE_NUM] = (uint32_t)irq_handler;
}

static inline void nvic_enable(uint32_t irqn)
{
	regs[REG_NVIC_ISER] = 1 << irqn;
}

static inline void nvic_disable(uint32_t irqn)
{
	regs[REG_NVIC_ICER] = 1 << irqn;
}

static inline void nvic_clear_pending(uint32_t irqn)
{
	regs[REG_NVIC_ICPR] = 1 << irqn;
}

static inline void nvic_set_pending(uint32_t irqn)
{
	regs[REG_NVIC_ISPR] = 1 << irqn;
}

#endif /* ___CORTEX_M0_NVIC_H__ */
