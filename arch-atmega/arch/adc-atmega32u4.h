#ifndef __ADC_ATMEGA32U4_H__
#define __ADC_ATMEGA32U4_H__

#define REG_ADCL   0x78
#define REG_ADCH   0x79
#define REG_ADSRA  0x7a
#define REG_ADSRB  0x7b
#define REG_ADMUX  0x7c
#define REG_DIDR2  0x7d
#define REG_DIDR0  0x7e
#define REG_DIDR1  0x7f

/* ADMUX defines for bits 7,6 (REFS1/REFS0) */
#define ADMUX_INT      0x0
#define ADMUX_AVCC     0x1
#define ADMUX_INT_CAP  0x3

/* ADMUX define for bits 5-0 (MUX) */
#define ADMUX_SEL(x) (x < 8 ? x : 0x20 + x)

static inline u16 adc_enable()
{
	/* FIXME to do */
}

static inline u16 adc_set_ref(int adc)
{
	/* FIXME to do */
}

static inline u16 adc_sel_in(int adc)
{
	_SFR_IO8(REG_ADMUX) &= ~0x3f;
	_SFR_IO8(REG_ADMUX) |= ADMUX_SEL(adc);
}

static inline u16 adc_disable(int adc)
{
	/* FIXME to do */
}

#endif /* __ADC_ATMEGA32U4_H__ */
