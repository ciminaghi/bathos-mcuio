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
#include <bathos/jiffies.h>
#include <bathos/sys_timer.h>
#include <mach/hw.h>

#define VECTABLE_ALIGNMENT CONFIG_VECTABLE_ALIGNMENT

#define VECTOR_TO_IRQNO(v) ((v) - 16)
#define IRQNO_TO_VECTOR(n) ((n) + 16)

#ifndef CONFIG_RELOCATE_VECTORS_TABLE
#define VECTORS __attribute__((section(".vectors"),aligned(VECTABLE_ALIGNMENT)))
#else
#define VECTORS __attribute__((aligned(VECTABLE_ALIGNMENT)))
#endif

#define STACK_ADDR ((bathos_irq_handler)CONFIG_STACK_ADDR)

extern void _romboot_start(void);
extern void _hard_fault_handler(void);

static void dummy_exc_handler(void)
{
}

void __attribute__((weak)) bathos_nmi_handler(void)
{
}

struct hw_stackframe {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	void *pc;
	uint32_t psr;
};

void __attribute__((naked)) hard_fault_handler(uint32_t lr, void *psp,
					       void *msp)
{
	struct hw_stackframe *frame;

	/* Find the active stack pointer (MSP or PSP) */
	if(lr & 0x4)
		frame = psp;
	else
		frame = msp;

	printf("** HARD FAULT **\r\n pc=%p\r\n  msp=%p\r\n  psp=%p\r\n",
	       frame->pc, msp, psp);

	while(1);
}

void __attribute__((naked)) _hard_fault_handler(void)
{
	asm("mov  r0, lr\n\
	     mrs  r1, psp\n\
	     mrs  r2, msp\n\
	     bl	  hard_fault_handler\n");
}

/* Interrupt handler for CORTEX-M system tick timer, vector 0xf everywhere */
static void default_systick_handler(void)
{
	jiffies++;
	trigger_event(&event_name(hw_timer_tick), NULL);
}

/*
 * The default irq handler just reads the irq number and triggers the relevant
 * interrupt event
 */
static void default_irq_handler(void)
{
	int vector;

	asm("mrs %[out], ipsr" : [out] "=r"(vector) : : );
	vector &= 0xff;
	trigger_interrupt_event(VECTOR_TO_IRQNO(vector));
}

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
	[4 ... 14] = dummy_exc_handler,
	[15] = default_systick_handler,
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
