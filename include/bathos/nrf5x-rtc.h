/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* nrf5x family rtc driver header file */

struct nrf5x_rtc_platform_data {
	void *base;
	int irq;
};

extern int nrf5x_rtc_init(const struct nrf5x_rtc_platform_data *plat);
extern void nrf5x_irq_handler(struct nrf5x_rtc_platform_data *plat);
