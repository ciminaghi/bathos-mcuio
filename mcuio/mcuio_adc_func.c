/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <arch/hw.h>
#include <bathos/adc.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <bathos/init.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

#define MCUIO_ADC_DEVICE 0x1235
#define MCUIO_ADC_VENDOR 0x0001

static const unsigned int PROGMEM u32_length = 4;
static const unsigned int PROGMEM adcs_ctrl_length = 0xfc0;

static const struct mcuio_func_descriptor PROGMEM adc_descr = {
	.device = MCUIO_ADC_DEVICE,
	.vendor = MCUIO_ADC_VENDOR,
	.rev = 0,
	/* ADCs class */
	.class = 0x00000003,
};

static const unsigned int PROGMEM adc_descr_length = sizeof(adc_descr);

extern struct mcuio_function adc;
static struct mcuio_function_runtime adc_rt;

static int adc_ctrl_rddw(const struct mcuio_range *r, unsigned offset,
			  uint32_t *out, int fill)
{
	unsigned idx = offset / 0x40;
	unsigned reg = offset % 0x40;
	uint8_t id[4];
	struct adc *adc;
#ifdef ARCH_IS_HARVARD
	struct adc _adc;
	memcpy_p(&_adc, &adcs[idx], sizeof(_adc));
	adc = &_adc;
#else
	adc = &adcs[idx];
#endif

	switch(reg) {

		/* FIXME: labels should be configurable.
		 * Here, labels are taken from adcs index */

		case 0x00: /* identifier, return AX */
			id[0] = 'A';
			id[1] = (idx >= 10) ? '0' + (idx / 10) : '0' + idx;
			id[2] = (idx >= 10) ? '0' + (idx % 10) : '\0';
			id[3] = '\0';
			memcpy(out, id, sizeof(id));
			flip4((uint8_t*)out);
				/* FIXME: to be done only if
				endiennes differs on MPU*/
			break;

		case 0x04: /* flags
			    * bit 0: signed */
			*out = adc->flags;
			break;

		case 0x08: /* voltage resolution (in uV) */
			*out = adc->vref_uv;
			break;

		case 0x0c: /* data */
			if (!adc_enabled())
				return -EACCES;
			*out = adc_sample(adc);
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

const struct mcuio_range_ops PROGMEM adc_ctrl_ops = {
	.rd = { NULL, NULL, adc_ctrl_rddw, NULL, },
	.wr = { NULL, NULL, NULL, NULL, },
};

static int adc_gen_ctrl_rddw(const struct mcuio_range *r, unsigned offset,
			  uint32_t *out, int fill)
{
	*out = adc_enabled();
	return 0;
}

static int adc_gen_ctrl_wrdw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	if (*__in) {
		adc_init();
		adc_en();
	}
	else
		adc_dis();
	return 0;
}

const struct mcuio_range_ops PROGMEM adc_gen_ctrl_ops = {
	.rd = { NULL, NULL, adc_gen_ctrl_rddw, NULL, },
	.wr = { NULL, NULL, adc_gen_ctrl_wrdw, NULL, },
};

static const struct mcuio_range PROGMEM adc_ranges[] = {
	/* ADC func descriptor */
	{
		.start = 0,
		.length = &adc_descr_length,
		.rd_target = &adc_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords 0x8, #ADCs and min period for reads */
	{
		.start = 0x008,
		.length = &u32_length,
		.rd_target = &num_adc,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dword 0x10, max value for period multiplier */
	{
		.start = 0x010,
		.length = &u32_length,
		.rd_target = &max_mul,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dword 0x14, general ADC control */
	{
		/* bit 0: enable */
		.start = 0x014,
		.length = &u32_length,
		.rd_target = NULL,
		.ops = &adc_gen_ctrl_ops,
	},
	/* dwords starting from 0x40, ADCs status and control */
	{
		.start = 0x40,
		.length = &adcs_ctrl_length,
		.rd_target = adcs,
		.ops = &adc_ctrl_ops,
	},
};

declare_mcuio_function(adc, adc_ranges, NULL, NULL, &adc_rt);
