/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#ifndef __ADC_ATMEGA32U4_H__
#define __ADC_ATMEGA32U4_H__

#include <avr/io.h>

/* ADMUX defines for bits 7,6 (REFS1/REFS0) */
#define ADMUX_AREF     0
#define ADMUX_AVCC     (1 << REFS0)
#define ADMUX_INT_CAP  ((1 << REFS0) | (1 << REFS1))

static inline void adc_en()
{
	ADCSRA |= (1 << ADEN);
}

static inline void adc_dis()
{
	ADCSRA &= ~(1 << ADEN);
}

static inline int adc_enabled()
{
	return (ADCSRA & (1 << ADEN)) ? 1 : 0;
}

static inline void adc_en_in(int adc)
{
	if (adc < 8)
		DIDR0 &= ~(1 << adc);
	else
		DIDR2 &= ~(1 << (adc - 8));
}

static inline void adc_dis_in(int adc)
{
	if (adc < 8)
		DIDR0 |= (1 << adc);
	else
		DIDR2 |= (1 << (adc - 8));
}

static inline void adc_dis_in_all()
{
	DIDR0 = 0xff;
	DIDR2 = 0xff;
}

/* call it with one of ADMUX_AREF, ADMUX_AVCC, ADMUX_INT_CAP */
static inline void adc_set_ref(int refs)
{
	ADMUX &= ~0xc0;
	ADMUX |= refs;
}

/* set adc prescaler */
static inline void adc_set_ps(int ps)
{
	ADCSRA |= (ps & 0x7);
}

static inline void adc_sel_in(int adc)
{
	ADMUX &= ~0x1f;
	ADMUX |= (adc < 8 ? adc : 0x20 + adc);
}

static inline void adc_start()
{
	ADCSRA |= (1 << ADSC);
}

static inline u32 adc_data()
{
	u32 d = 0;
	d = ADCL;
	d |= ADCH << 8;
	return d;
}

#endif /* __ADC_ATMEGA32U4_H__ */
