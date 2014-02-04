#include <arch/hw.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

uint32_t gpio_configs[2];
uint32_t gpio_data[2];

static struct mcuio_func_descriptor gpio_descr = {
	.device = 0x0001,
	.vendor = 0xbeef,
	.rev = 0,
	/* GPIOs class */
	.class = 0x00000002,
};

static uint32_t gpio_ro_range[] = {
	[0] = ('P' << 8 | '1' << 0),
	/* 8 GPIOS */
	[1] = 8,
	/* All OUTPUTS only */
	[2 ... 3] = 0x02020202,
	/* Events, none available for now */
	[10 ... 13] = 0,
};

static struct mcuio_function_runtime gpio_rt;

int gpio_data_wrb(const struct mcuio_range *r, unsigned offset,
		  const uint32_t *__in, int fill)
{
	const uint8_t *in = (const uint8_t *)__in;
	if (fill)
		return -EPERM;
	*(uint8_t *)(r->target + offset) = *in;
	printf("%s: gpio %d-%d, new status = 0x%02x\n",
	       __func__, offset/32, offset/32 + 7, *in);
	return sizeof(uint8_t);
}

int gpio_data_wrw(const struct mcuio_range *r, unsigned offset,
		  const uint32_t *__in, int fill)
{
	const uint16_t *in = (const uint16_t *)__in;
	if (fill)
		return -EPERM;
	*(uint16_t *)(r->target + offset) = *in;
	printf("%s: gpio %d-%d, new status = 0x%04x\n",
	       __func__, offset/32, offset/32 + 15, *in);
	return sizeof(uint16_t);
}

int gpio_data_wrdw(const struct mcuio_range *r, unsigned offset,
		   const uint32_t *in, int fill)
{
	*(uint32_t *)(r->target + offset) = *in;
	if (fill)
		return -EPERM;
	printf("%s: gpio %d-%d, new status = 0x%08x\n",
	       __func__, offset/32, offset/32 + 31, *in);
	return sizeof(uint32_t);
}

int gpio_data_wrq(const struct mcuio_range *r, unsigned offset,
		  const uint32_t *in, int fill)
{
	return -EPERM;
}

const struct mcuio_range_ops gpio_data_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_rddw, mcuio_rdq, },
	.wr = { gpio_data_wrb, gpio_data_wrw, gpio_data_wrdw, gpio_data_wrq, },
};

static const struct mcuio_range gpio_ranges[] = {
	{
		.start = 0,
		.length = sizeof(gpio_descr),
		.target = &gpio_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	{
		.start = 8,
		.length = sizeof(gpio_ro_range),
		.target = (char *)gpio_ro_range,
		.ops = &default_mcuio_range_ro_ops,
	},
	{
		.start = 0x40,
		.length = 32,
		.target = (char *)gpio_configs,
		.ops = &default_mcuio_range_ops,
	},
	{
		.start = 0x70,
		.length = 4,
		.target = (char *)gpio_data,
		.ops = &gpio_data_ops,
	},
};


declare_mcuio_function(gpio, gpio_ranges, NULL, NULL, &gpio_rt);
