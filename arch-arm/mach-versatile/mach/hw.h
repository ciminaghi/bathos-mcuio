/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#ifndef __VERSATILE_HW_H__
#define __VERSATILE_HW_H__
#include <stdint.h>
#include <bathos/io.h>

#define CPU_FREQ		(1 * 1000 * 1000)

/*
 * This timer can only be divided by 1, 16, 256. Choose a value that
 * still is a multiple of 4, 10, 25 and so on. It overflows in 19 hours
 */
/* This is now in Kconfig */
/* #define HZ			(CPU_FREQ / 16)	*/	/* 62500 */

/* timer 0 (from Linux-2.6.31) */
#define REG_TIMER_LOAD		(0x101e3000 / 4)
#define REG_TIMER_VALUE		(0x101e3004 / 4)
#define REG_TIMER_CTRL		(0x101e3008 / 4)
#define		TIMER_CTRL_ONESHOT		(1 << 0)
#define		TIMER_CTRL_32BIT		(1 << 1)
#define		TIMER_CTRL_DIV1			(0 << 2)
#define		TIMER_CTRL_DIV16		(1 << 2)
#define		TIMER_CTRL_DIV256		(2 << 2)
#define		TIMER_CTRL_IE			(1 << 5)
#define		TIMER_CTRL_PERIODIC		(1 << 6)
#define		TIMER_CTRL_ENABLE		(1 << 7)
#define REG_TIMER_INTCLR	(0x101e300c / 4)
#define REG_TIMER_RIS		(0x101e3010 / 4)
#define REG_TIMER_MIS		(0x101e3014 / 4)
#define REG_TIMER_BGLOAD	(0x101e3018 / 4)

#define IRQ_UART0		12
#define IRQ_UART1		13
#define IRQ_UART2		14

#endif /* __VERSATILE_HW_H__ */

