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
#define ADMUX_INT      (0x0 << 6)
#define ADMUX_AVCC     (0x1 << 6)
#define ADMUX_INT_CAP  (0x3 << 6)

/* ADMUX define for bits 5-0 (MUX) */
#define ADMUX_SEL(x) (x < 8 ? x : 0x20 + x)

/* ADCSRA defines */
#define ADCSRA_ADEN (1 << 7)
#define ADCSRA_ADSC (1 << 6)

static inline u16 adc_en()
{
	_SFR_IO8(REG_ADSRA) |= ADCSRA_ADEN;
}

static inline u16 adc_dis()
{
	_SFR_IO8(REG_ADSRA) &= ~ADCSRA_ADEN;
}

static inline u16 adc_en_in(int adc)
{
	if (adc < 8)
		_SFR_IO8(REG_DIDR0) &= ~(1 << adc);
	else
		_SFR_IO8(REG_DIDR2) &= ~(1 << (adc - 8));
}

static inline u16 adc_dis_in(int adc)
{
	if (adc < 8)
		_SFR_IO8(REG_DIDR0) |= (1 << adc);
	else
		_SFR_IO8(REG_DIDR2) |= (1 << (adc - 8));
}

static inline u16 adc_dis_in_all()
{
	_SFR_IO8(REG_DIDR0) = 0xff;
	_SFR_IO8(REG_DIDR2) = 0xff;
}

static inline u16 adc_set_ref(int refs)
{
	_SFR_IO8(REG_ADMUX) &= ~0xc0;
	_SFR_IO8(REG_ADMUX) |= refs;
}

static inline u16 adc_sel_in(int adc)
{
	_SFR_IO8(REG_ADMUX) &= ~0x3f;
	_SFR_IO8(REG_ADMUX) |= ADMUX_SEL(adc);
}

static inline u16 adc_start()
{
	_SFR_IO8(REG_ADSRA) |= ADCSRA_ADSC;
}

static inline u32 adc_data()
{
	u32 d = 0;
	d = _SFR_IO8(REG_ADCL);
	d |= (_SFR_IO8(REG_ADCH) << 8);
}

#endif /* __ADC_ATMEGA32U4_H__ */
