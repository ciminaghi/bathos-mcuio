/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

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

/* Status (enabled/disabled) of each pwm (max 32) */
static uint32_t pwm_stat = 0;

int pwm_enabled(int idx)
{
	return (pwm_stat & (1 << idx)) ? 1 : 0;
}

static uint8_t t_ref[];

static void init_timer0(void)
{
	/* Enable Fast PWM Mode on Timer 0. Period = 4ms, as
	 set in arch/atmega-32u4.c (pwm can not change it, since Timer 0
	 is used as main system timer) */
	TCCR0A |= (1 << WGM00) | (1 << WGM01);
}

static void deinit_timer0(void)
{
	/* Disable Fast PWM Mode on timer0 */
	TCCR0A &= ~((1 << WGM00) | (1 << WGM01));
}

static void check_init_timer(struct pwm *pwm)
{
	if (t_ref[pwm->tim_id] == 0) {
		t_ref[pwm->tim_id] = 1;
		pwm->ops.init_timer();
	}
	else if (!pwm_enabled(pwm->tim_id))
		t_ref[pwm->tim_id]++;
}

static void check_deinit_timer(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	if (pwm_enabled(id))
		t_ref[id]--;

	if (t_ref[id] == 0)
		pwm->ops.deinit_timer();
}

static void init_timer1(void)
{
	/* Enable Fast PWM Mode and WGM 14 on Timer 1. Default period 20ms. */
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
	ICR1 = 0;
}

static void init_timer3(void)
{
	/* Enable Fast PWM Mode and WGM 14 on Timer 3. Default period 20ms. */
	TCCR3A |= (1 << WGM11);
	TCCR3B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	ICR3 = 39999;
}

static void deinit_timer3(void)
{
	/* Disable Fast PWM Mode and clock on timer3 */
	TCCR3A &= ~(1 << WGM11);
	TCCR3B &= ~((1 << WGM13) | (1 << WGM12) |
		(1 << CS10) | (1 << CS11) | (1 << CS12));
	ICR3 = 0;
}

static void init_timer4(void)
{
	/* Set timer4. Default period 16.384ms */
	TCCR4B |= (1 << CS43) | (1 << CS41) | (1 << CS40);
}

static void deinit_timer4(void)
{
	/* Reset timer4. */
	TCCR4B &= ~((1 << CS43) | (1 << CS41) | (1 << CS40));
}

/* Warning: since timer1 is shared by 1A, 1B and 1C outputs, a set
 * of period in each of them causes a change of period for the
 * other outputs */
static int pwm_set_period_timer1(struct pwm *pwm, uint32_t val)
{
	ICR1 = val - 1;
	return 0;
}

static uint32_t pwm_get_period_timer1(struct pwm *pwm)
{
	return ICR1 + 1;
}

/* Warning: since timer3 is shared by 3A, 3B and 3C outputs, a set
 * of period in each of them causes a change of period for the
 * other outputs */
static int pwm_set_period_timer3(struct pwm *pwm, uint32_t val)
{
	ICR3 = val - 1;
	return 0;
}

static uint32_t pwm_get_period_timer3(struct pwm *pwm)
{
	return ICR3 + 1;
}

static int pwm_set_period_timer4(struct pwm *pwm, uint32_t val)
{
	OCR4C = val - 1;
	return 0;
}

static uint32_t pwm_get_period_timer4(struct pwm *pwm)
{
	return OCR4C + 1;
}

static int pwm_en(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_init_timer(pwm);
	_SFR_MEM8(pwm->ctl_reg) |= pwm->en_msk;
	_SFR_MEM8(pwm->dir_reg) |= pwm->dir_msk;

	pwm_stat |= (1 << id);
	return 0;
}

static void pwm_dis(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_deinit_timer(pwm);
	_SFR_MEM8(pwm->ctl_reg) &= ~pwm->en_msk;
	_SFR_MEM8(pwm->dir_reg) &= ~pwm->dir_msk;


	pwm_stat &= ~(1 << id);
}

static int pwm_get_polarity(const struct pwm *pwm)
{
	return _SFR_MEM8(pwm->ctl_reg) & pwm->pol_msk ? 1 : 0;
}

static int pwm_set_polarity(struct pwm *pwm, uint32_t val)
{
	if (val)
		_SFR_MEM8(pwm->ctl_reg) |= pwm->pol_msk;
	else
		_SFR_MEM8(pwm->ctl_reg) &= ~pwm->pol_msk;
	return 0;
}

/* OC0B output */
static int pwm_set_duty_0b(struct pwm *pwm, uint32_t val)
{
	OCR0B = *((uint8_t*)&val);
	return 0;
}

static uint32_t pwm_get_duty_0b(struct pwm *pwm)
{
	return OCR0B;
}

static uint32_t pwm_get_period_default(struct pwm *pwm)
{
	return pwm->tim_max_mul + 1;
}

/* OC1A, OC1B, OC1C, OC3A output duty cycle handling */

static int pwm_set_duty_1_3(struct pwm *pwm, uint32_t val)
{
	if (val > 0)
		val--;
	_SFR_MEM16(pwm->duty_reg_l) = val;
	return 0;
}

static uint32_t pwm_get_duty_1_3(struct pwm *pwm)
{
	uint32_t ret = _SFR_MEM16(pwm->duty_reg_l);
	return ret + 1;
}

/* OC4D output */
static int pwm_set_duty_4d(struct pwm *pwm, uint32_t val)
{
	OCR4D = val;
	return 0;
}

static uint32_t pwm_get_duty_4d(struct pwm *pwm)
{
	return OCR4D;
}

static int pwm_get_polarity_4d(const struct pwm *pwm)
{
	return (TCCR4B & (1 << PWM4X)) ? 1 : 0;
}

static int pwm_set_polarity_4d(struct pwm *pwm, uint32_t val)
{
	if (val)
		TCCR4B |= (1 << PWM4X);
	else
		TCCR4B &= ~(1 << PWM4X);
	return 0;
}

/* FIXME: labels should be configurable. Here, yun board mapping is
 * temporarly fixed in the src */
const struct pwm PROGMEM pwms[] = {
	{ /* OC0B */
		.label = "D3",
		.tim_res_ns = 15625,
		.tim_max_mul = 255,
		.tim_id = 0,
		.ctl_reg = _SFR_MEM_ADDR(TCCR0A),
		.en_msk = 1 << COM0B1,
		.pol_msk = 1 << COM0B0,
		.dir_reg = _SFR_MEM_ADDR(DDRD),
		.dir_msk = (1 << DDD0),
		.ops = {pwm_en,
			pwm_dis,
			pwm_get_period_default,
			pwm_get_duty_0b,
			NULL,
			pwm_set_duty_0b,
			pwm_get_polarity,
			pwm_set_polarity,
			init_timer0,
			deinit_timer0,
		}
	},
	{ /* OC1A */
		.label = "D9",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.tim_id = 1,
		.ctl_reg = _SFR_MEM_ADDR(TCCR1A),
		.en_msk = 1 << COM1A1,
		.pol_msk = 1 << COM1A0,
		.dir_reg = _SFR_MEM_ADDR(DDRB),
		.dir_msk = 1 << DDB5,
		.duty_reg_l = _SFR_MEM_ADDR(OCR1AL),
		.ops = {pwm_en,
			pwm_dis,
			pwm_get_period_timer1,
			pwm_get_duty_1_3,
			pwm_set_period_timer1,
			pwm_set_duty_1_3,
			pwm_get_polarity,
			pwm_set_polarity,
			init_timer1,
			deinit_timer1,
		}
	},
	{ /* OC1B */
		.label = "D10",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.tim_id = 1,
		.ctl_reg = _SFR_MEM_ADDR(TCCR1A),
		.en_msk = 1 << COM1B1,
		.pol_msk = 1 << COM1B0,
		.dir_reg = _SFR_MEM_ADDR(DDRB),
		.dir_msk = 1 << DDB6,
		.duty_reg_l = _SFR_MEM_ADDR(OCR1BL),
		.ops = {pwm_en,
			pwm_dis,
			pwm_get_period_timer1,
			pwm_get_duty_1_3,
			pwm_set_period_timer1,
			pwm_set_duty_1_3,
			pwm_get_polarity,
			pwm_set_polarity,
			init_timer1,
			deinit_timer1,
		}
	},
	{ /* OC1C */
		.label = "D11",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.tim_id = 1,
		.ctl_reg = _SFR_MEM_ADDR(TCCR1A),
		.en_msk = 1 << COM1C1,
		.pol_msk = 1 << COM1C0,
		.dir_reg = _SFR_MEM_ADDR(DDRB),
		.dir_msk = 1 << DDB7,
		.duty_reg_l = _SFR_MEM_ADDR(OCR1CL),
		.ops = {pwm_en,
			pwm_dis,
			pwm_get_period_timer1,
			pwm_get_duty_1_3,
			pwm_set_period_timer1,
			pwm_set_duty_1_3,
			pwm_get_polarity,
			pwm_set_polarity,
			init_timer1,
			deinit_timer1,
		}
	},
	{ /* OC3A */
		.label = "D5",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.tim_id = 3,
		.ctl_reg = _SFR_MEM_ADDR(TCCR3A),
		.en_msk = 1 << COM3A1,
		.pol_msk = 1 << COM3A0,
		.dir_reg = _SFR_MEM_ADDR(DDRC),
		.dir_msk = 1 << DDC6,
		.duty_reg_l = _SFR_MEM_ADDR(OCR3AL),
		.ops = {pwm_en,
			pwm_dis,
			pwm_get_period_timer3,
			pwm_get_duty_1_3,
			pwm_set_period_timer3,
			pwm_set_duty_1_3,
			pwm_get_polarity,
			pwm_set_polarity,
			init_timer3,
			deinit_timer3,
		}
	},
	{ /* OC4D */
		.label = "D6",
		.tim_res_ns = 64000,
		.tim_max_mul = 255,
		.tim_id = 0,
		.ctl_reg = _SFR_MEM_ADDR(TCCR4C),
		.en_msk = (1 << COM4D1) | (1 << PWM4D),
		.dir_reg = _SFR_MEM_ADDR(DDRD),
		.dir_msk = 1 << DDD7,
		.ops = {pwm_en,
			pwm_dis,
			pwm_get_period_timer4,
			pwm_get_duty_4d,
			pwm_set_period_timer4,
			pwm_set_duty_4d,
			pwm_get_polarity_4d,
			pwm_set_polarity_4d,
			init_timer4,
			deinit_timer4,
		}
	},

};

const uint32_t PROGMEM num_pwm = ARRAY_SIZE(pwms);

static uint8_t t_ref[ARRAY_SIZE(pwms)];
