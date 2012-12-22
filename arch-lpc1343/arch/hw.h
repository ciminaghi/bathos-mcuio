/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 */
#ifndef __LPC1343_HW_H__
#define __LPC1343_HW_H__
#include <stdint.h>
#include <bathos/io.h>

#define THOS_QUARTZ		(12 * 1000 * 1000)
#define HZ			100


/* uart */
#define REG_U0THR		(0x40008000 / 4)
#define REG_U0LSR		(0x40008014 / 4)
#define REG_U0LSR_THRE		0x20

/* clock control */
#define REG_AHBCLKCTRL		(0x40048080 / 4)
#define REG_AHBCLKCTRL_SYS	(1 << 0)
#define REG_AHBCLKCTRL_ROM	(1 << 1)
#define REG_AHBCLKCTRL_RAM	(1 << 2)
#define REG_AHBCLKCTRL_FLASHR	(1 << 3)
#define REG_AHBCLKCTRL_FLASHA	(1 << 4)
#define REG_AHBCLKCTRL_I2C	(1 << 5)
#define REG_AHBCLKCTRL_GPIO	(1 << 6)
#define REG_AHBCLKCTRL_CT16B0	(1 << 7)
#define REG_AHBCLKCTRL_CT16B1	(1 << 8)
#define REG_AHBCLKCTRL_CT32B0	(1 << 9)
#define REG_AHBCLKCTRL_CT32B1	(1 << 10)
#define REG_AHBCLKCTRL_SSP	(1 << 11)
#define REG_AHBCLKCTRL_UART	(1 << 12)
#define REG_AHBCLKCTRL_ADC	(1 << 13)
#define REG_AHBCLKCTRL_USBREG	(1 << 14)
#define REG_AHBCLKCTRL_WDT	(1 << 15)
#define REG_AHBCLKCTRL_IOCON	(1 << 16)

/* counter 0 */
#define REG_TMR32B0TCR		(0x40014004 / 4)
#define REG_TMR32B0TC		(0x40014008 / 4)
#define REG_TMR32B0PR		(0x4001400c / 4)
#define REG_TMR32B0MCR		(0x40014014 / 4)
#define REG_TMR32B0MR0		(0x40014018 / 4)
#define REG_TMR32B0MR1		(0x4001401c / 4)
#define REG_TMR32B0MR2		(0x40014020 / 4)
#define REG_TMR32B0MR3		(0x40014024 / 4)
#define REG_TMR32B0PWMC		(0x40014074 / 4)

/* counter 1 */
#define REG_TMR32B1TCR		(0x40018004 / 4)
#define REG_TMR32B1TC		(0x40018008 / 4)
#define REG_TMR32B1PR		(0x4001800c / 4)

/* gpio port 3 */
#define REG_GPIO3DAT		(0x50033ffc / 4)
#define REG_GPIO3DIR		(0x50038000 / 4)

#endif /* __LPC1343_HW_H__ */
