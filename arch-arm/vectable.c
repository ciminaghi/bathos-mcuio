/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <aurelio@aureliocolosimo.it>
 */

#include <stdint.h>
#include <generated/autoconf.h>
#include <bathos/event.h>
#include <bathos/init.h>
#include <bathos/io.h>
#include <bathos/irq.h>

#define VECTABLE_ALIGNMENT CONFIG_VECTABLE_ALIGNMENT

#define VECTOR_TO_IRQNO(v) ((v) - 16)
#define IRQNO_TO_VECTOR(n) ((n) + 16)

#define FLASH_VECTORS \
	__attribute__((section(".vectors"),aligned(VECTABLE_ALIGNMENT)))

#define RAM_VECTORS \
	__attribute__((aligned(VECTABLE_ALIGNMENT)))

#ifndef CONFIG_RELOCATE_VECTORS_TABLE
#define VECTORS FLASH_VECTORS
#else
#define VECTORS RAM_VECTORS
#endif

#define STACK_ADDR ((bathos_irq_handler)CONFIG_STACK_ADDR)

extern void _romboot_start(void);
extern void _hard_fault_handler(void);

#if defined CONFIG_CPU_CORTEX_M0 || defined CONFIG_CPU_CORTEX_M0_PLUS || \
defined CONFIG_CPU_CORTEX_M3
static inline int __get_vector(void)
{
	int vector;

	asm("mrs %[out], ipsr" : [out] "=r"(vector) : : );
	return vector & 0xff;
}
#else
/* To be implemented for non cortex cpus */
static inline int __get_vector(void)
{
	return -1;
}
#endif

static void dummy_exc_handler(void)
{
}

void __attribute__((weak)) bathos_nmi_handler(void)
{
}

void __attribute__((naked)) _hard_fault_handler(void)
{
	while(1);
}

/*
 * The default irq handler just reads the irq number and triggers the relevant
 * interrupt event
 */
static void default_irq_handler(void)
{
	trigger_interrupt_event(VECTOR_TO_IRQNO(__get_vector()));
}

#ifdef CONFIG_RELOCATE_VECTORS_TABLE
/* Temporary table to be stored in flash when relocation is configured */
bathos_irq_handler flash_vec_table[] FLASH_VECTORS = {
	[0] = STACK_ADDR,
	[1] = _romboot_start,
}
#endif

/*
 * This will belong to the .vectors section in case of no vectors relocation
 * (and the linker shall be instructed to place .vectors at the right address).
 * Otherwise it will be a regular .data array, and an initcall shall set VTOR
 * to the proper value __before__ enabling interrupts and requesting irq
 * redirection (see relocate_vectors_table below).
  */
bathos_irq_handler vec_table[] VECTORS = {
	[0] = STACK_ADDR,
	[1] = _romboot_start,
	[2] = bathos_nmi_handler,
	[3] = _hard_fault_handler,
	[4 ... 15] = dummy_exc_handler,
	/* Irq */
	[16 ... 16 + CONFIG_NR_INTERRUPTS - 1] = default_irq_handler,
};

#ifdef CONFIG_RELOCATE_VECTORS_TABLE
static int relocate_vectors_table(void)
{
	regs[REG_SCB_VTOR] = (uint32_t)vec_table;
}
core_initcall(relocate_vectors_table);

int arch_set_irq_vector(int irq, bathos_irq_handler handler)
{
	int v = IRQNO_TO_VECTOR(irq);

	if (v < 0 || v >= ARRAY_SIZE(vec_table))
		return -EINVAL;

	vec_table[IRQNO_TO_VECTOR(irq)] = handler;
	return 0;
}
#endif
