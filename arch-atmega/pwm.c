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

#define NPWM 4

#define pwm_warn_no_device(i) \
	printf("%s warning: no such pwm device, idx=%d\n", __func__, i);


const uint32_t PROGMEM num_pwm = NPWM;

/* Status (enabled/disabled) of each pwm (max 32) */
uint32_t pwm_stat = 0;

/* FIXME: labels should be configurable. Here, yun board mapping is
 * temporarly fixed in the src */
const struct pwm PROGMEM pwms[NPWM] = {
	{ /* OC1C */
		.nbits = 16,
		.label = "D11",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
	},
	{ /* OC0B */
		.nbits = 8,
		.label = "D3",
		.tim_res_ns = 62500,
		.tim_max_mul = 255,
	},
	{ /* OC1A */
		.nbits = 16,
		.label = "D9",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
	},
	{ /* OC1B */
		.nbits = 16,
		.label = "D10",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
	},
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

static void init_timer1(void)
{
	/* Enable Fast PWM Mode and WGM 14 on Timer 1. Period 20ms. */
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	ICR1 = 39999;
}

static void deinit_timer1(void)
{
	/* Disable Fast PWM Mode and clock on timer1 */
	TCCR1A &= ~(1 << WGM11);
	TCCR1B &= ~((1 << WGM13) | (1 << WGM12) |
		(1 << CS10) | (1 << CS11) | (1 << CS12));
}

/* Warning: these functions are now generic to all pwms (only 4), and use a
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
			if (!(pwm_stat & 0xd))
				init_timer1();
			TCCR1A |= (1 << COM1C1);
			DDRB |= (1 << DDB7);
			break;
		case 1:
			if (!(pwm_stat & 0x2))
				init_timer0();
			TCCR0A |= (1 << COM0B1);
			DDRD |= (1 << DDD0);
			break;
		case 2:
			TCCR1A |= (1 << COM1A1);
			if (!(pwm_stat & 0x0d))
				init_timer1();
			DDRB |= (1 << DDB5);
			break;
		case 3:
			if (!(pwm_stat & 0x0d))
				init_timer1();
			TCCR1A |= (1 << COM1B1);
			DDRB |= (1 << DDB6);
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
			TCCR1A &= ~(1 << COM1C1);
			DDRB &= ~(1 << DDB7);
			if (!(pwm_stat & 0xd))
				deinit_timer1();
			break;
		case 1:
			TCCR0A &= ~(1 << COM0B1);
			DDRD &= ~(1 << DDD0);
			if (!(pwm_stat & 0x1))
				deinit_timer0();
			break;
		case 2:
			TCCR1A &= ~(1 << COM1A1);
			DDRB &= ~(1 << DDB5);
			if (!(pwm_stat & 0x0d))
				deinit_timer1();
			break;
		case 3:
			TCCR1A &= ~(1 << COM1B1);
			DDRB &= ~(1 << DDB6);
			if (!(pwm_stat & 0x0d))
				deinit_timer1();
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
			OCR1CH = val >> 8;
			OCR1CL = val & 0xff;
			break;
		case 1:
			OCR0B = *((uint8_t*)&val);
			break;
		case 2:
			OCR1AH = val >> 8;
			OCR1AL = val & 0xff;
			break;
		case 3:
			OCR1BH = val >> 8;
			OCR1BL = val & 0xff;
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
			ret = OCR1C;
			break;
		case 1:
			ret = OCR0B;
			break;
		case 2:
			ret = OCR1A;
			break;
		case 3:
			ret = OCR1B;
			break;
		default:
			break;
	}

	return ret;
}
