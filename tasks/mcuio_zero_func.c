#include <arch/hw.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"


static struct mcuio_func_descriptor PROGMEM zero_descr = {
	.device = 0xdead,
	.vendor = 0xbeef,
	.rev = 0,
	.class = 0,
};

const char PROGMEM zero_ro_contents[8] = "mcuio";

char PROGMEM zero_rw_contents[8] = "deadbeef";

static struct mcuio_function_runtime zero_rt;

static const struct mcuio_range PROGMEM zero_ranges[] = {
	{
		.start = 0,
		.length = sizeof(zero_descr),
		.rd_target = &zero_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	{
		.start = 8,
		.length = sizeof(zero_ro_contents),
		.rd_target = (char *)zero_ro_contents,
		.ops = &default_mcuio_range_ro_ops,
	},
	{
		.start = 16,
		.length = sizeof(zero_rw_contents),
		.rd_target = zero_rw_contents,
		.wr_target = zero_rw_contents,
		.ops = &default_mcuio_range_ops,
	},
};


declare_mcuio_function(zero, zero_ranges, NULL, NULL, &zero_rt);
