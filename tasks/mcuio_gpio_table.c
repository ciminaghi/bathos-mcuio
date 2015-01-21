/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#define MCUIO_GPIO_PORT_LABEL ('P' << 8 | ('0' + MCUIO_GPIO_PORT))

#include <bathos/gpio.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"
#include "mcuio_gpio_func.h"

#define gpio_port_label(a) xcat(gpio_port_label_, a)
#define gpio_labels_size(a) xcat(gpio_labels_size_, a)
#define gpio_caps_size(a) xcat(gpio_caps_size_, a)
#define gpio_evts_caps_size(a) xcat(gpio_evts_caps_size_, a)
#define gpio_labels_size(a) xcat(gpio_labels_size_, a)
#define gpio_labels_start(a) xcat(gpio_labels_start_, a)
#define gpio_caps_start(a) xcat(gpio_caps_start_, a)
#define gpio_evts_caps_start(a) xcat(gpio_evts_caps_start_, a)
#define gpio_func_name(a) xcat(gpio_, a)
#define gpio_irq_event_name(a) xcat(mcuio_irq

static const uint32_t PROGMEM gpio_ro_range1[] = {
	[0] = MCUIO_GPIO_PORT_LABEL,
	/* Number of gpios is defined on the command line */
	[1] = MCUIO_NGPIO,
};

static const unsigned int PROGMEM gpio_ro_range1_length =
	sizeof(gpio_ro_range1);

extern const unsigned int gpio_labels_size(MCUIO_GPIO_PORT);
extern const unsigned int gpio_caps_size(MCUIO_GPIO_PORT);
extern const unsigned int gpio_evts_caps_size(MCUIO_GPIO_PORT);
static const unsigned int PROGMEM __gpio_labels_size =
	(const unsigned int)&gpio_labels_size(MCUIO_GPIO_PORT);
static const unsigned int PROGMEM __gpio_caps_size =
	(const unsigned int)&gpio_caps_size(MCUIO_GPIO_PORT);
static const unsigned int PROGMEM __gpio_evts_caps_size =
	(const unsigned int)&gpio_evts_caps_size(MCUIO_GPIO_PORT);

extern const char PROGMEM gpio_labels_start(MCUIO_GPIO_PORT)[];
extern const uint32_t PROGMEM gpio_caps_start(MCUIO_GPIO_PORT)[];
extern const uint32_t PROGMEM gpio_evts_caps_start(MCUIO_GPIO_PORT)[];

static const unsigned int PROGMEM gpio_evts_status_length = 8;
static const unsigned int PROGMEM gpio_data_length = 8;
static const unsigned int PROGMEM gpio_modes_size = 0x200;
static const unsigned int PROGMEM gpio_evts_masks_size = 0x200;
static struct mcuio_gpio_port_status port_status;
static const struct mcuio_gpio_port_data PROGMEM port_data = {
	.status = &port_status,
	.gpio_start = MCUIO_GPIO_PORT * MCUIO_GPIOS_PER_FUNC,
	.gpio_end = MCUIO_GPIO_PORT * MCUIO_GPIOS_PER_FUNC + MCUIO_NGPIO - 1,
};

static const struct mcuio_func_descriptor PROGMEM gpio_descr = {
	.device = CONFIG_MCUIO_GPIO_DEVICE,
	.vendor = CONFIG_MCUIO_GPIO_VENDOR,
	.rev = 0,
	/* GPIOs class */
	.class = 0x00000002,
};

static const unsigned int PROGMEM gpio_descr_length = sizeof(gpio_descr);


static const struct mcuio_range PROGMEM gpio_ranges[] = {
	/* GPIO func descriptor */
	{
		.start = 0,
		.length = &gpio_descr_length,
		.rd_target = &gpio_descr,
		.ops = &default_mcuio_range_ro_ops,
		.priv = &port_data,
	},
	/* dwords 0x8 and 0xc, port name and number of gpios */
	{
		.start = 8,
		.length = &gpio_ro_range1_length,
		.rd_target = (char *)gpio_ro_range1,
		.ops = &default_mcuio_range_ro_ops,
		.priv = &port_data,
	},
	/* dwords 0x10 .. 0x10f, gpio labels */
	{
		.start = 0x10,
		.length = &__gpio_labels_size,
		.rd_target = gpio_labels_start(MCUIO_GPIO_PORT),
		.ops = &default_mcuio_range_ro_ops,
		.priv = &port_data,
	},
	/* dwords 0x110 .. 0x30f, gpio capabilities */
	{
		.start = 0x110,
		.length = &__gpio_caps_size,
		.rd_target = gpio_caps_start(MCUIO_GPIO_PORT),
		.ops = &default_mcuio_range_ro_ops,
		.priv = &port_data,
	},
	/* dwords 0x310 .. 0x40f, gpio events capabilities */
	{
		.start = 0x310,
		.length = &__gpio_evts_caps_size,
		.rd_target = gpio_evts_caps_start(MCUIO_GPIO_PORT),
		.ops = &default_mcuio_range_ro_ops,
		.priv = &port_data,
	},
	/* dwords 0x510 .. 0x70f, actual gpio modes */
	{
		.start = 0x510,
		.length = &gpio_modes_size,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_modes_ops,
		.priv = &port_data,
	},
	/*
	  dwords 0x710 .. 0x90f, actual gpio events masks,
	  currently not implemented
	*/
	{
		.start = 0x710,
		.length = &gpio_evts_masks_size,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_evts_ops,
		.priv = &port_data,
	},
	/* dwords 0x910 0x914, gpio data */
	{
		.start = 0x910,
		.length = &gpio_data_length,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_data_ops,
		.priv = &port_data,
	},
	/* dwords 0x918 0x91c, gpio set */
	{
		.start = 0x918,
		.length = &gpio_data_length,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_set_ops,
		.priv = &port_data,
	},
	/* dwords 0x920 0x924, gpio clear */
	{
		.start = 0x920,
		.length = &gpio_data_length,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_clr_ops,
		.priv = &port_data,
	},
	/*
	  dwords 0x928 0x92c gpio events status, currently
	  not implemented
	*/
	{
		.start = 0x928,
		.length = &gpio_evts_status_length,
		.ops = &gpio_evts_status_ops,
		.priv = &port_data,
	},
};

declare_mcuio_function(gpio_func_name(MCUIO_GPIO_PORT), gpio_ranges, NULL, NULL,
		       &mcuio_func_common_runtime);

static void __gpio_evt_handle(struct event_handler_data *ed)
{
	gpio_evt_handle(ed, &port_data);
}

declare_event_handler(gpio_evt, NULL, __gpio_evt_handle, NULL);
