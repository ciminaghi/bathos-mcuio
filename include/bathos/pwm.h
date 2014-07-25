#ifndef __PWM_H__
#define __PWM_H__

#include <bathos/stdio.h>
#include <arch/bathos-arch.h>

extern const uint32_t num_pwm;   /* number of supported pwm's */

struct pwm;

struct pwm_ops {
	int (*enable)(struct pwm *pwm);
	void (*disable)(struct pwm *pwm);
	int (*set)(struct pwm *pwm, uint32_t val);
	uint32_t (*get)();
};

struct pwm {
	uint8_t nbits;
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
