/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * Comes from original bathos boot.S for lpc-1343 and lpc2104
 * by Alessandro Rubini
 */
#include <generated/autoconf.h>

.global _romboot_start
.global _ramboot_start
.global _do_romboot
.extern __bss_start__
.extern __bss_end__
.extern arm_bathos_main

#define STACK_ADDR CONFIG_STACK_ADDR

.section .text.romboot
/* We start here */
_romboot_start:
#if defined CONFIG_OPENSDA_DEBUG
	b       _romboot_start
#endif


/* Common romboot code */
.section .text.romboot_common
_do_romboot:
	/* Start with interrupts disabled */
        ldr     r0, =1
        msr     primask, r0

        /* Copy data to RAM */
        ldr     r0, =_sdata
        ldr     r1, =_edata
        ldr     r2, =_erom
        /* copy from r2 to r0, until r1. We know it's aligned at 16b */
10:
        cmp     r0, r1
        bge     2f
        ldmia   r2!, {r4, r5, r6, r7}
        stmia   r0!, {r4, r5, r6, r7}
        b       10b
_ramboot_start: 
2:
        /* clear BSS */
        mov     r0, #0
        ldr     r1, =__bss_start__
        ldr     r2, =__bss_end__

0:
        cmp     r1, r2
        bge     1f
        str     r0, [r1]
        add     r1, #4
        b       0b
1:
        bl 	bathos_setup

        /* Enable interrupts and start main */
        ldr     r0, =0
        msr     primask, r0
        bl 	arm_bathos_main
dead:
        b  dead
