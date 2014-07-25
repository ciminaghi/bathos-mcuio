#ifndef __PWM_H__
#define __PWM_H__

#include <bathos/stdio.h>
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
};

/* pwm output
 * tim_res_ns: resolution of timing (min period one may set)
 * tim_max_mul: max value for multiplier (see pwm operations above)
 */
struct pwm {
	char label[4];
	uint32_t tim_res_ns;
	uint32_t tim_max_mul;
	struct pwm_ops ops;
};

extern const struct pwm PROGMEM pwms[];

extern int pwm_enabled(int idx);

static inline int pwm_id(const struct pwm *pwm)
{
	return pwm - pwms;
}

#endif
