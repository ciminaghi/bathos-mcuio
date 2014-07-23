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
	const struct pwm *pwm = &pwms[idx];
	uint8_t id[4];

	switch(reg) {

		case 0x00: /* identifier, return PWXX */
			id[0] = 'P';
			id[1] = 'W';
			id[2] = '0' + (idx / 10);
			id[3] = '0' + (idx % 10);
			memcpy(out, id, sizeof(id));
			break;

		case 0x04: /* capabilities
			    * bit 7-0: #nbits */
			*out = pwm->nbits;
			break;

		case 0x08: /* current value */
			*out = pwm_get(idx);
			break;

		case 0x0c: /* status and ctrl */
			/* bit 0 of this reg is 'enabled' bit */
			*out = pwm_enabled(idx) ? 0x1 : 0x0;
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

	switch(reg) {

		case 0x08: /* current value */
			ret = pwm_set(idx, *__in);
			break;

		case 0x0c: /* status and ctrl */
			if ((*__in) & 0x1)
				ret = pwm_en(idx);
			else
				ret = pwm_dis(idx);
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