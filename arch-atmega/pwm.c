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

static uint8_t t0_ref = 0;
static uint8_t t1_ref = 0;
static uint8_t t3_ref = 0;
static uint8_t t4_ref = 0;

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

static void check_init_timer0(int id)
{
	if (t0_ref == 0) {
		t0_ref = 1;
		init_timer0();
	}
	else if (!pwm_enabled(id))
		t0_ref++;
}

static void check_deinit_timer0(int id)
{
	if (pwm_enabled(id))
		t0_ref--;

	if (t0_ref == 0)
		deinit_timer0();
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

static void check_init_timer1(int id)
{
	if (t1_ref == 0) {
		t1_ref = 1;
		init_timer1();
	}
	else if (!pwm_enabled(id))
		t1_ref++;
}

static void check_deinit_timer1(int id)
{
	if (pwm_enabled(id))
		t1_ref--;

	if (t1_ref == 0)
		deinit_timer1();
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

static void check_init_timer3(int id)
{
	if (t3_ref == 0) {
		t3_ref = 1;
		init_timer3();
	}
	else if (!pwm_enabled(id))
		t3_ref++;
}

static void check_deinit_timer3(int id)
{
	if (pwm_enabled(id))
		t3_ref--;

	if (t3_ref == 0)
		deinit_timer3();
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

static void check_init_timer4(int id)
{
	if (t4_ref == 0) {
		t4_ref = 1;
		init_timer4();
	}
	else if (!pwm_enabled(id))
		t4_ref++;
}

static void check_deinit_timer4(int id)
{
	if (pwm_enabled(id))
		t4_ref--;

	if (t4_ref == 0)
		deinit_timer4();
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

/* OC0B output */
static int pwm_en_0b(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_init_timer0(id);
	TCCR0A |= (1 << COM0B1);
	DDRD |= (1 << DDD0);
	pwm_stat |= (1 << id);
	return 0;
}

static void pwm_dis_0b(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_deinit_timer0(id);
	TCCR0A &= ~(1 << COM0B1);
	DDRD &= ~(1 << DDD0);
	pwm_stat &= ~(1 << id);
}

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
	uint32_t max;
	__copy_dword(&max, &pwm->tim_max_mul);
	return max + 1;
}

static int pwm_get_polarity_0b(const struct pwm *pwm)
{
	return TCCR0A & (1 << COM0B0) ? 1 : 0;
}

static int pwm_set_polarity_0b(struct pwm *pwm, uint32_t val)
{
	if (val)
		TCCR0A |= (1 << COM0B0);
	else
		TCCR0A &= ~(1 << COM0B0);
	return 0;
}

/* OC1A output */
static int pwm_en_1a(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_init_timer1(id);
	TCCR1A |= (1 << COM1A1);
	DDRB |= (1 << DDB5);
	pwm_stat |= (1 << id);
	return 0;
}

static void pwm_dis_1a(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_deinit_timer1(id);
	TCCR1A &= ~(1 << COM1A1);
	DDRB &= ~(1 << DDB5);
	pwm_stat &= ~(1 << id);
}

static int pwm_set_duty_1a(struct pwm *pwm, uint32_t val)
{
	OCR1AH = val >> 8;
	OCR1AL = val & 0xff;
	return 0;
}

static uint32_t pwm_get_duty_1a(struct pwm *pwm)
{
	return OCR1A;
}

static int pwm_get_polarity_1a(const struct pwm *pwm)
{
	return TCCR1A & (1 << COM1A0) ? 1 : 0;
}

static int pwm_set_polarity_1a(struct pwm *pwm, uint32_t val)
{
	if (val)
		TCCR1A |= (1 << COM1A0);
	else
		TCCR1A &= ~(1 << COM1A0);
	return 0;
}

/* OC1B output */
static int pwm_en_1b(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_init_timer1(id);
	TCCR1A |= (1 << COM1B1);
	DDRB |= (1 << DDB6);
	pwm_stat |= (1 << id);
	return 0;
}

static void pwm_dis_1b(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_deinit_timer1(id);
	TCCR1A &= ~(1 << COM1B1);
	DDRB &= ~(1 << DDB6);
	pwm_stat &= ~(1 << id);
}

static int pwm_set_duty_1b(struct pwm *pwm, uint32_t val)
{
	OCR1BH = val >> 8;
	OCR1BL = val & 0xff;
	return 0;
}

static uint32_t pwm_get_duty_1b(struct pwm *pwm)
{
	return OCR1B;
}

static int pwm_get_polarity_1b(const struct pwm *pwm)
{
	return TCCR1A & (1 << COM1B0) ? 1 : 0;
}

static int pwm_set_polarity_1b(struct pwm *pwm, uint32_t val)
{
	if (val)
		TCCR1A |= (1 << COM1B0);
	else
		TCCR1A &= ~(1 << COM1B0);
	return 0;
}

/* OC1C output */
static int pwm_en_1c(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_init_timer1(id);
	TCCR1A |= (1 << COM1C1);
	DDRB |= (1 << DDB7);
	pwm_stat |= (1 << id);
	return 0;
}

static void pwm_dis_1c(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_deinit_timer1(id);
	TCCR1A &= ~(1 << COM1C1);
	DDRB &= ~(1 << DDB7);
	pwm_stat &= ~(1 << id);
}

static int pwm_set_duty_1c(struct pwm *pwm, uint32_t val)
{
	OCR1CH = val >> 8;
	OCR1CL = val & 0xff;
	return 0;
}

static uint32_t pwm_get_duty_1c(struct pwm *pwm)
{
	return OCR1C;
}

static int pwm_get_polarity_1c(const struct pwm *pwm)
{
	return TCCR1A & (1 << COM1C0) ? 1 : 0;
}

static int pwm_set_polarity_1c(struct pwm *pwm, uint32_t val)
{
	if (val)
		TCCR1A |= (1 << COM1C0);
	else
		TCCR1A &= ~(1 << COM1C0);
	return 0;
}

/* OC3A output */
static int pwm_en_3a(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_init_timer3(id);
	TCCR3A |= (1 << COM3A1);
	DDRC |= (1 << DDC6);
	pwm_stat |= (1 << id);
	return 0;
}

static void pwm_dis_3a(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_deinit_timer3(id);
	TCCR3A &= ~(1 << COM3A1);
	DDRC &= ~(1 << DDC6);
	pwm_stat &= ~(1 << id);
}

static int pwm_set_duty_3a(struct pwm *pwm, uint32_t val)
{
	OCR3AH = val >> 8;
	OCR3AL = val & 0xff;
	return 0;
}

static uint32_t pwm_get_duty_3a(struct pwm *pwm)
{
	return OCR3A;
}

static int pwm_get_polarity_3a(const struct pwm *pwm)
{
	return TCCR3A & (1 << COM3A0) ? 1 : 0;
}

static int pwm_set_polarity_3a(struct pwm *pwm, uint32_t val)
{
	if (val)
		TCCR3A |= (1 << COM3A0);
	else
		TCCR3A &= ~(1 << COM3A0);
	return 0;
}

/* OC4D output */
static int pwm_en_4d(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_init_timer4(id);
	TCCR4C |= (1 << COM4D1) | (1 << PWM4D);
	DDRD |= (1 << DDD7);
	return 0;
}

static void pwm_dis_4d(struct pwm *pwm)
{
	int id = pwm_id(pwm);
	check_deinit_timer4(id);
	TCCR4C &= ~((1 < COM4D1) | (1 < PWM4D));
	DDRD &= ~(1 << DDD7);
}

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
	return 0;
}

static int pwm_set_polarity_4d(struct pwm *pwm, uint32_t val)
{
	if (val)
		TCCR4B |= (1 << PWM4X);
	else
		TCCR4B &= ~(1 << PWM4X);
	return 0;
}

#define NPWM 6
const uint32_t PROGMEM num_pwm = NPWM;

/* FIXME: labels should be configurable. Here, yun board mapping is
 * temporarly fixed in the src */
const struct pwm PROGMEM pwms[NPWM] = {
	{ /* OC0B */
		.label = "D3",
		.tim_res_ns = 15625,
		.tim_max_mul = 255,
		.ops = {pwm_en_0b,
			pwm_dis_0b,
			pwm_get_period_default,
			pwm_get_duty_0b,
			NULL,
			pwm_set_duty_0b,
			pwm_get_polarity_0b,
			pwm_set_polarity_0b,
		}
	},
	{ /* OC1A */
		.label = "D9",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.ops = {pwm_en_1a,
			pwm_dis_1a,
			pwm_get_period_timer1,
			pwm_get_duty_1a,
			pwm_set_period_timer1,
			pwm_set_duty_1a,
			pwm_get_polarity_1a,
			pwm_set_polarity_1a,
		}
	},
	{ /* OC1B */
		.label = "D10",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.ops = {pwm_en_1b,
			pwm_dis_1b,
			pwm_get_period_timer1,
			pwm_get_duty_1b,
			pwm_set_period_timer1,
			pwm_set_duty_1b,
			pwm_get_polarity_1b,
			pwm_set_polarity_1b,
		}
	},
	{ /* OC1C */
		.label = "D11",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.ops = {pwm_en_1c,
			pwm_dis_1c,
			pwm_get_period_timer1,
			pwm_get_duty_1c,
			pwm_set_period_timer1,
			pwm_set_duty_1c,
			pwm_get_polarity_1c,
			pwm_set_polarity_1c,
		}
	},
	{ /* OC3A */
		.label = "D5",
		.tim_res_ns = 500,
		.tim_max_mul = 65535,
		.ops = {pwm_en_3a,
			pwm_dis_3a,
			pwm_get_period_timer3,
			pwm_get_duty_3a,
			pwm_set_period_timer3,
			pwm_set_duty_3a,
			pwm_get_polarity_3a,
			pwm_set_polarity_3a,
		}
	},
	{ /* OC4D */
		.label = "D6",
		.tim_res_ns = 64000,
		.tim_max_mul = 255,
		.ops = {pwm_en_4d,
			pwm_dis_4d,
			pwm_get_period_timer4,
			pwm_get_duty_4d,
			pwm_set_period_timer4,
			pwm_set_duty_4d,
			pwm_get_polarity_4d,
			pwm_set_polarity_4d,
		}
	},

};
