#include <generated/autoconf.h>

.global _romboot_start
.global _ramboot_start
.global raise
.global _hard_fault_handler
.extern __bss_start__
.extern __bss_end__
.extern main
.extern hard_fault_handler

#define STACK_ADDR CONFIG_STACK_ADDR

/* Provide a temporary vector table in case relocation has to be performed */
#ifdef CONFIG_RELOCATE_VECTORS_TABLE
.section .vectors, "ax"

vectors:
	.word STACK_ADDR         /* Stack address */
	.word _romboot_start + 1
	.word nmi_handler + 1
	.word _hard_fault_handler + 1
	.rep 45                  /* Lazily, "reset" at each interrupt or trap */
	.word trapme + 1 /* First instruction address */
	.endr
#endif /* CONFIG_RELOCATE_VECTORS_TABLE */

.section .flash_config
	.word 0xffffffff
	.word 0xffffffff
	.word 0xffffffff
	.word 0xffffff7e
	
.section .text.romboot

_romboot_start:

#if defined OPENSDA_DEBUG
	b	_romboot_start
#endif
	
	/* Copy data to RAM */
	ldr	r0, =_sdata
	ldr	r1, =_edata
	ldr	r2, =_erom

	/* copy from r2 to r0, until r1. We know it's aligned at 16b */
10:
	cmp	r0, r1
	bge	2f
	ldmia	r2!, {r4, r5, r6, r7}
	stmia	r0!, {r4, r5, r6, r7}
	b	10b
	
_ramboot_start:	
2:
	/* clear BSS */
	mov	r0, #0
	ldr	r1, =__bss_start__
	ldr	r2, =__bss_end__

0:
	cmp	r1, r2
	bge	1f
	str	r0, [r1]
	add	r1, #4
	b	0b
1:
	bl bathos_setup
	bl bathos_main
dead:
	b  dead

nmi_handler:
	b nmi_handler

	
_hard_fault_handler:
	mov    r0, lr
	mrs  r1, psp
	mrs  r2, msp
	bl	hard_fault_handler


trapme:
	b trapme

