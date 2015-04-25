/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#ifndef __PWM_H__
#define __PWM_H__

#include <bathos/stdio.h>
#include <bathos/string.h>
#include <arch/bathos-arch.h>

extern const uint32_t num_pwm;   /* number of supported pwm's */

struct pwm;

/* pwm operations
 * Each pwm output must implement the 'mandatory' functions in pwm_ops
 * list.
 * multipliers for period and duty are to be applied to resolution given
 * by tim_res_ns (see below): actual duty and period are given by:
 * period_ns = pwm->ops.get_period() * pwm->tim_res_ns;
 * duty_ns = pwm->ops.get_duty() * pwm->tim_res_ns;
 */
struct pwm_ops {
	/* mandatory */
	int (*enable)(struct pwm *pwm);
	void (*disable)(struct pwm *pwm);
	uint32_t (*get_period)(); /* multiplier for period */
	uint32_t (*get_duty)();   /* multiplier for duty */

	/* optional */
	int (*set_period)(struct pwm *pwm, uint32_t val);
	int (*set_duty)(struct pwm *pwm, uint32_t val);
	int (*get_polarity)(const struct pwm *pwm);
	int (*set_polarity)(struct pwm *pwm, uint32_t val);

	void (*init_timer)(void);
	void (*deinit_timer)(void);
};

/* pwm output
 * tim_res_ns: resolution of timing (min period one may set)
 * tim_max_mul: max value for multiplier (see pwm operations above)
 */
struct pwm {
#ifdef ARCH_IS_HARVARD
	int id;
#endif
	char label[4];
	uint32_t tim_res_ns;
	uint32_t tim_max_mul;
	int tim_id;
	int ctl_reg;
	uint8_t en_msk;
	uint8_t pol_msk;
	int dir_reg;
	uint8_t dir_msk;
	int duty_reg_l;
	struct pwm_ops ops;
};

extern const struct pwm PROGMEM pwms[];

extern int pwm_enabled(int idx);

static inline int pwm_id(const struct pwm *pwm)
{
#ifdef ARCH_IS_HARVARD
	return pwm->id;
#else
	return pwm - pwms;
#endif
}

static inline const struct pwm *get_pwm(struct pwm *pwm,
		const struct pwm pwms[], int idx)
{
	memcpy_p(pwm, &pwms[idx], sizeof(*pwm));
#ifdef ARCH_IS_HARVARD
	pwm->id = idx;
#endif
	return pwm;
}

static inline const struct pwm_ops *get_pwm_ops(const struct pwm *pwm,
		struct pwm_ops *out)
{
	memcpy_p(out, &pwm->ops, sizeof(*out));
	return out;
}

#endif
