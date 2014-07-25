#ifndef __PWM_H__
#define __PWM_H__

#include <bathos/stdio.h>
#include <arch/bathos-arch.h>

extern const uint32_t num_pwm;   /* number of supported pwm's */

struct pwm {
	uint8_t nbits;
	char label[4];
	uint32_t tim_res_ns;
	uint32_t tim_max_mul;
};

extern const struct pwm PROGMEM pwms[];

/* enable / disable / enabled status */
extern int pwm_en(int idx);
extern int pwm_dis(int idx);
extern int pwm_enabled(int idx);

/* Correct sequence for pwm is: pwm_en, pwm_set, pwm_dis
 * val must be pwms[idx].nbits bits value */
extern int pwm_set(int idx, uint32_t val);

/* Read current duty cycle value */
extern uint32_t pwm_get(int idx);

#endif
