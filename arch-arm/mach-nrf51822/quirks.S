/*
 * This file contains ASM quirks to be executed immediately after boot
 * The nrf51822 needs all ram banks to be turned on (processor errata)
 */
	
.equ    NRF_POWER_RAMON_ADDRESS,            0x40000524
.equ    NRF_POWER_RAMON_RAMxON_ONMODE_Msk,  0x3

.section .text.romboot_quirks
.thumb_func
_romboot_quirks:
	ldr     r0, =NRF_POWER_RAMON_ADDRESS
	ldr     r2, [r0]
	movs    r1, #NRF_POWER_RAMON_RAMxON_ONMODE_Msk
	orr   	r2, r1
	str     r2, [r0]

