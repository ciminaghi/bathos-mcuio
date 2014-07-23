#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <avr/io.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/pwm.h>
#include <bathos/bitops.h>
#include <bathos/errno.h>
#include <bathos/delay.h>
#include <bathos/stdio.h>

#define NPWM 2

#define pwm_warn_no_device(i) \
	printf("%s warning: no such pwm device, idx=%d\n", __func__, i);


const uint32_t PROGMEM num_pwm = NPWM;

/* Status (enabled/disabled) of each pwm (max 32) */
uint32_t pwm_stat = 0;

const struct pwm PROGMEM pwms[NPWM] = {
	[0 ... 1] = { /* OC0A, OC0B */
		.nbits = 8},
};

static void init_timer0(void)
{
	/* Enable Fast PWM Mode and clock on Timer 0. Period = 16ms */
	TCCR0A |= (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS00) | (1 << CS02);
	TCCR0B &= ~(1 << CS01);
}

static void deinit_timer0(void)
{
	/* Disable Fast PWM Mode and clock on timer0 */
	TCCR0A &= ~((1 << WGM00) | (1 << WGM01));
	TCCR0B &= ~(1 << CS00);
}

/* Warning: these functions are now generic to all pwms (only 2), and use a
 * switch to choose which pwm output is to be handled; they should
 * be better implemented as a call to function pointers which
 * should be declared inside struct pwm */

int pwm_en(int idx)
{
	if (idx >= NPWM) {
		pwm_warn_no_device(idx);
		return -ENODEV;
	}


	switch (idx) {
		case 0:
			if (!(pwm_stat & 0x3))
				init_timer0();
			TCCR0A |= (1 << COM0A1);
			DDRB |= (1 << DDB7);
			break;
		case 1:
			if (!(pwm_stat & 0x3))
				init_timer0();
			TCCR0A |= (1 << COM0B1);
			DDRD |= (1 << DDD0);
			break;
		default:
			return -ENODEV;
	}

	pwm_stat |= (1 << idx);

	return 0;
}

int pwm_dis(int idx)
{
	if (idx >= NPWM) {
		pwm_warn_no_device(idx);
		return -ENODEV;
	}

	pwm_stat &= ~(1 << idx);

	switch (idx) {
		case 0:
			TCCR0A &= ~(1 << COM0A1);
			DDRB &= ~(1 << DDB7);
			if (!(pwm_stat & 0x3))
				deinit_timer0();
			break;
		case 1:
			TCCR0A &= ~(1 << COM0B1);
			DDRD &= ~(1 << DDD0);
			if (!(pwm_stat & 0x3))
				deinit_timer0();
			break;
		default:
			return -ENODEV;
	}

	return 0;
}

int pwm_enabled(int idx)
{
	if (idx >= NPWM) {
		pwm_warn_no_device(idx);
		return 0;
	}

	return (pwm_stat & (1 << idx)) ? 1 : 0;
}

int pwm_set(int idx, uint32_t val)
{
	if (idx >= NPWM) {
		pwm_warn_no_device(idx);
		return -ENODEV;
	}

	switch (idx) {
		case 0:
			OCR0A = *((uint8_t*)&val);
			break;
		case 1:
			OCR0B = *((uint8_t*)&val);
			break;
		default:
			return -ENODEV;
	}

	return 0;
}

extern uint32_t pwm_get(int idx)
{
	uint32_t ret = 0;

	if (idx >= NPWM) {
		pwm_warn_no_device(idx);
		return -ENODEV;
	}

	switch (idx) {
		case 0:
			ret = OCR0A;
			break;
		case 1:
			ret = OCR0B;
			break;
		default:
			break;
	}

	return ret;
}
