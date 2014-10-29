/*
 * Boot code, ram and flash (according to the sections being used)
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 */
.global vectors
.global _entry_ram
.global _entry_rom
.global raise
.extern bathos_setup
.extern bathos_main

/*
 * This file includes the initial assembly stuff. It sets the stack
 * pointer and zeroes the BSS. For rom-boot it also copies data to RAM.
 *
 * Different ELF sections are used to differentiate the behaviour
 */

/* The vectors are used when booting from flash */
.section .vectors, "ax"
vectors:
	/* The first vector in Cortex is the stack pointer */
	.word 0x100003fc
	/* The second one is reset */
	.word _entry_rom + 1

	/*
	 * We don't have interrupts, so there's no need to allocate
	 * vectors. However, we neet at least 8 of them, as vector 7
	 * is used for the checksum. So align to 0x20
	 */
.align 5, 0


/* A trivial data section to remember the entry point, used in raise() */
.data
_entry_save:
	.word 0

/* RAM boot: the first kB of RAM is unused, place the stack here */
.section .text.ramboot
_entry_ram:
	mov	r0, pc
	sub	r0, #12
	mov	sp, r0
	/* Save the entry point */
	ldr	r0, =_entry_ram + 1
	ldr	r1, =_entry_save
	str	r0, [r1]

	b	_bss_loop

/* ROM boot: put the stack before data (the lds puts data 1k into RAM) */
.section .text.romboot
_entry_rom:
	ldr	r0, =_entry_rom + 1
	ldr	r1, =_entry_save
	str	r0, [r1]
	/* FIXME: check flash speed */
	ldr	r0, =_sdata
	ldr	r1, =_edata
	ldr	r2, =_erom
	/* stack is before data */
	mov	r3, r0
	sub	r3, #4
	mov	sp, r3
	/* copy from r2 to r0, until r1. We know it's aligned at 16b */
10:
	cmp	r0, r1
	bge	_bss_loop
	ldmia	r2!, {r4, r5, r6, r7}
	stmia	r0!, {r4, r5, r6, r7}
	b	10b
.ltorg

/* What follows is common to ROM and RAM boot: clear bss and call main */
.section .text.anyboot
_bss_loop:
	ldr	r1, =__bss_start
	ldr	r2, =__bss_end
	mov	r0, #0
0:
	cmp	r1, r2
	bge	1f
	str	r0, [r1]
	add	r1, #4
	b	0b

1:
	bl bathos_setup
	bl bathos_main
	/* Fall through to raise() */

raise: /* raise is called by libgcc on division by 0 */
	ldr	r1, =_entry_save
	ldr	r0, [r1]
	mov	pc, r0


