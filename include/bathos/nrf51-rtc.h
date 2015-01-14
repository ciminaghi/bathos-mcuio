/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* nrf51 family rtc driver header file */

struct nrf51_rtc_platform_data {
	void *base;
	int irq;
};

extern int nrf51_rtc_init(const struct nrf51_rtc_platform_data *plat);
extern void nrf51_irq_handler(struct nrf51_rtc_platform_data *plat);
