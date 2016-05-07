/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __MCUIO_GPIO_FUNC_H__
#define __MCUIO_GPIO_FUNC_H__

#include <bathos/bathos.h>
#include <stdint.h>

#include "mcuio-function.h"

/* Each port has a maximum of 64 gpios */
struct mcuio_gpio_port_status {
	uint32_t gpio_events_status[2];
	uint32_t gpio_events_falling[2];
	uint32_t gpio_events_rising[2];
	uint32_t gpio_events_enable[2];
	uint32_t gpio_events_high[2];
	uint32_t gpio_events_low[2];
};

struct mcuio_gpio_port_data {
	struct mcuio_gpio_port_status *status;
	int port_index;
	int gpio_start;
	int gpio_end;
	const struct mcuio_function * PROGMEM function;
};

extern const struct mcuio_range_ops PROGMEM gpio_data_ops;
extern const struct mcuio_range_ops PROGMEM gpio_set_ops;
extern const struct mcuio_range_ops PROGMEM gpio_clr_ops;
extern const struct mcuio_range_ops PROGMEM gpio_modes_ops;
extern const struct mcuio_range_ops PROGMEM gpio_evts_ops;
extern const struct mcuio_range_ops PROGMEM gpio_evts_status_ops;

/* Common handler for gpio event */
extern void gpio_evt_handle(struct event_handler_data *ed,
			    const struct mcuio_gpio_port_data * PROGMEM _pd);

#define MCUIO_GPIOS_PER_FUNC 64

#endif /* __MCUIO_GPIO_FUNC_H__ */
