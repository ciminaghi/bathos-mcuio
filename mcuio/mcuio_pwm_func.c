/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <arch/hw.h>
#include <bathos/pwm.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <bathos/init.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

#define MCUIO_PWM_DEVICE 0x1236
#define MCUIO_PWM_VENDOR 0x0001

static const unsigned int PROGMEM u32_length = 4;
static const unsigned int PROGMEM pwms_ctrl_length = 0xfc0;

static const struct mcuio_func_descriptor PROGMEM pwm_descr = {
	.device = MCUIO_PWM_DEVICE,
	.vendor = MCUIO_PWM_VENDOR,
	.rev = 0,
	/* PWMs class */
	.class = 0x00000005,
};

static const unsigned int PROGMEM pwm_descr_length = sizeof(pwm_descr);

extern struct mcuio_function pwm;
static struct mcuio_function_runtime pwm_rt;

static int pwm_ctrl_rddw(const struct mcuio_range *r, unsigned offset,
			  uint32_t *out, int fill)
{
	unsigned idx = offset / 0x40;
	unsigned reg = offset % 0x40;
	struct pwm pwm;

	get_pwm(&pwm, pwms, idx);

	switch(reg) {

		case 0x00: /* label */
			memcpy(out, &pwm.label[0], sizeof(*out));
			flip4((uint8_t*)out);
				/* FIXME: to be done only if
				endiennes differs on MPU*/

			break;

		case 0x04: /* capabilities
				bits 23-0: timing resolution (ns)
				bit 24: can change period
				bit 25: can change duty */
			*out = pwm.tim_res_ns;
			*out &= 0x00ffffff;
			if (pwm.ops.set_period)
				*out |= 1l << 24;
			if (pwm.ops.set_duty)
				*out |= 1l << 25;
			break;

		case 0x08: /* max multiplier */
			*out = pwm.tim_max_mul;
			break;

		case 0x0c: /* status and ctrl
				* bit 0: enabled
				* bit 1: invert polarity */
			*out = pwm_enabled(idx) ? 0x1 : 0x0;
			if ((pwm.ops.get_polarity) &&
				pwm.ops.get_polarity(&pwm))
				*out |= (1 << 1);
			break;

		case 0x10: /* period multiplier */
			*out = pwm.ops.get_period(&pwm);
			break;

		case 0x14: /* duty multiplier */
			*out = pwm.ops.get_duty(&pwm);
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

static int pwm_ctrl_wrdw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	unsigned idx = offset / 0x40;
	unsigned reg = offset % 0x40;
	int ret = 0;
	struct pwm pwm;

	get_pwm(&pwm, pwms, idx);

	switch(reg) {

		case 0x0c: /* status and ctrl (bit0: enable) */
			if ((*__in) & 0x1)
				ret = pwm.ops.enable(&pwm);
			else
				pwm.ops.disable(&pwm);
			if (pwm.ops.set_polarity)
				pwm.ops.set_polarity(&pwm, ((*__in) >> 1) & 0x1);
			break;

		case 0x10: /* set period multiplier */
			if (!pwm.ops.set_period)
				ret = -EINVAL;
			ret = pwm.ops.set_period(&pwm, *__in);
			break;

		case 0x14: /* set duty cycle multiplier */
			if (!pwm.ops.set_duty)
				ret = -EINVAL;
			ret = pwm.ops.set_duty(&pwm, *__in);
			break;

		default:
			return -EINVAL;
	}

	return ret;
}

const struct mcuio_range_ops PROGMEM pwm_ctrl_ops = {
	.rd = { NULL, NULL, pwm_ctrl_rddw, NULL, },
	.wr = { NULL, NULL, pwm_ctrl_wrdw, NULL, },
};

static const struct mcuio_range PROGMEM pwm_ranges[] = {
	/* PWM func descriptor */
	{
		.start = 0,
		.length = &pwm_descr_length,
		.rd_target = &pwm_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dword 0x8, #PWMs */
	{
		.start = 0x008,
		.length = &u32_length,
		.rd_target = &num_pwm,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords starting from 0x40, PWMs status and control */
	{
		.start = 0x40,
		.length = &pwms_ctrl_length,
		.rd_target = pwms,
		.ops = &pwm_ctrl_ops,
	},
};

declare_mcuio_function(pwm, pwm_ranges, NULL, NULL, &pwm_rt);
