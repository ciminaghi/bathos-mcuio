/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* nrf51 family rtc driver, just counts jiffies */

#include <arch/hw.h>
#include <arch/systick.h>
#include <bathos/io.h>
#include <bathos/nrf51-rtc.h>

#define RTC_TASK_START	0x000

#define RTC_EVENT_TICK	0x100

#define RTC_INTENSET	0x304
#define RTC_INTENCLR	0x308
#define RTC_EVTEN	0x340
#define RTC_EVTENSET	0x344
#define RTC_EVTENCLR	0x348
#define EVT_TICK	0x01

#define RTC_COUNTER	0x504
#define RTC_PRESCALER	0x508
#define RTC_CC(c)	(0x540 + (c)*4)


int nrf51_rtc_init(const struct nrf51_rtc_platform_data *plat)
{
	uint32_t prescaler = (LFCLK / HZ) - 1;
	uint32_t evten = EVT_TICK;
	uint32_t b = (uint32_t)plat->base;

	writel(prescaler, b + RTC_PRESCALER);
	/* Enable irq on tick */
	writel(evten, b + RTC_EVTENSET);
	writel(evten, b + RTC_INTENSET);
	/* Start counter */
	writel(1, b + RTC_TASK_START);
	return 0;
}

void nrf51_irq_handler(struct nrf51_rtc_platform_data *plat)
{
	uint32_t b = (uint32_t)plat->base;

	writel(0, b + RTC_EVENT_TICK);
	bathos_arm_sys_tick_timer_handler();
}

