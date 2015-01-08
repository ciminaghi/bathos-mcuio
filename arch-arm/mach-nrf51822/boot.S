.global _romboot_start
.global _ramboot_start
.global raise
.global _hard_fault_handler
.extern __bss_start__
.extern __bss_end__
.extern main
.extern hard_fault_handler

.section .text.romboot

.equ    NRF_POWER_RAMON_ADDRESS,            0x40000524
.equ    NRF_POWER_RAMON_RAMxON_ONMODE_Msk,  0x3

.thumb
.thumb_func
.align 1

_romboot_start:

	ldr     r0, =NRF_POWER_RAMON_ADDRESS
	ldr     r2, [r0]
	movs    r1, #NRF_POWER_RAMON_RAMxON_ONMODE_Msk
	orr   	r2, r1
	str     r2, [r0]

#if defined OPENSDA_DEBUG
_wait:
	b	_wait
#endif

	/* Start with interrupts disabled */
	ldr	r0, =1
	msr 	primask, r0
	
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

	/* Enable interrupts and start main */
	ldr	r0, =0
	msr 	primask, r0
	bl bathos_main
dead:
	b	dead


nmi_handler:
	b nmi_handler

	
_hard_fault_handler:
	mov    r0, lr
	mrs  r1, psp
	mrs  r2, msp
	bl	hard_fault_handler


trapme:
	b trapme

