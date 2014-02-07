/* We need to provide interrupt vectors here */
.extern do_jiffies_irq
.section .vectors, "ax"

	rjmp _bathos_start /* 0 */
	nop /* 2 */
	nop /* 4 */
	nop /* 6 */
	nop /* 8 */
	nop /* a */
	nop /* c */
	nop /* e */
	nop /* 10 */
	nop /* 12 */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	rjmp __vector_10   /* 14 */
	nop
	/* rjmp cippa */
	rjmp __vector_11   /* 16 */
	/* rjmp lippa */
	.rep 50
	nop
	.endr
	
#if 0	
	/* We use one interrupt only, to count jiffies: always jump there */
	.rep 25
	nop
	.endr

	rjmp do_jiffies_irq
#endif

/* Fllowins is boot code */
.section .boot, "ax"

.global _bathos_start

/* The following two symbols are mandated, or libgcc code creeps in */
.global __do_copy_data
.global __do_clear_bss

/* The following is defined in the linker script */
.extern __data_load_start
.extern __data_load_end
.extern __data_start
.extern __bss_start
.extern __bss_end
_bathos_start:
	/* r1  must always be 0 by convention, and clear irq in status  */
	eor r1, r1
	out 0x3f, r1
	/* set SP at address 0x3d, 0x3e -- assume 1k RAM + 0x60 for regs */
	ldi r28, lo8(1023 + 0x60)
	ldi r29, hi8(1023 + 0x60)
	out 0x3d, r28
	out 0x3e, r29

	/* Then, we must copy data from program memory to RAM */
__do_copy_data:
	/* Z (r30, 31) loads from program memory */
	ldi r30, lo8(__data_load_start)
	ldi r31, hi8(__data_load_start)
	/* "cpc" only compares with a register, so save high part here */
	ldi r16, hi8(__data_load_end)
	/* X (r26, r27) stores in RAM */
	ldi r26, lo8(__data_start)
	ldi r27, hi8(__data_start)

	/* __data_load_end (and start) is provided by linker script */
0:	cpi r30, lo8(__data_load_end)
	cpc r31, r16
	brge 1f
	lpm r2, Z+
	st X+, r2
	rjmp 0b

	/* Now, clear bss, use X again, and r16 for high part of end */
__do_clear_bss:
1:	ldi r26, lo8(__bss_start)
	ldi r27, hi8(__bss_start)
	ldi r16, hi8(__bss_end)
0:	cpi r26, lo8(__bss_end)
	cpc r27, r16
	brge 1f
	st X+, r1
	rjmp 0b
	
1:	/* Finally, run real code */
	rcall bathos_setup
	sei
	rcall bathos_main
	rjmp _bathos_start
