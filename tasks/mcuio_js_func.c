#include <arch/hw.h>
#include <bathos/gpio.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

static struct mcuio_function_runtime js_rt;

static const struct mcuio_func_descriptor PROGMEM js_descr = {
	/* dev 1 , vendor 1 */
	.device = 0x0001,
	.vendor = 0x0001,
	.rev = 0,
	/* shield class */
	.class = 0x0000000c,
};

static const unsigned int PROGMEM js_descr_length = sizeof(js_descr);

static const struct mcuio_range PROGMEM js_ranges[] = {
	/* GPIO func descriptor */
	{
		.start = 0,
		.length = &js_descr_length,
		.rd_target = &js_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
};

declare_mcuio_function(js, js_ranges, NULL, NULL, &js_rt);
