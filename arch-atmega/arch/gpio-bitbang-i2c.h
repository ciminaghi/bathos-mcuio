#ifndef __GPIO_BITBANG_H__
#define __GPIO_BITBANG_H__

#define setsda __atmega_bitbang_setsda
#define setscl __atmega_bitbang_setscl
#define getscl __atmega_bitbang_getscl
#define getsda __atmega_bitbang_getsda


static inline void __atmega_bitbang_setsda(int v)
{
	/* SDA -> output */
	DDRD |= _BV(DDD1);
	/* Set value */
	if (v) {
		PORTD |= _BV(PORTD1);
		/* Make it an input again */
		DDRD &= ~_BV(DDD1);
		return;
	}
	PORTD &= ~_BV(PORTD1);
}

static inline void __atmega_bitbang_setscl(int v)
{
	/* SCL -> output */
	DDRD |= _BV(DDD0);
	/* Set value */
	if (v) {
		PORTD |= _BV(PORTD0);
		/* Make it an input again */
		DDRD &= ~_BV(DDD0);
		return;
	}
	PORTD &= ~_BV(PORTD0);
}

static inline int __atmega_bitbang_getscl(void)
{
	return PIND & _BV(PIND0);
}

static inline int __atmega_bitbang_getsda(void)
{
	return PIND & _BV(PIND1);
}

/* No udelay for i2c ! */
#define i2c_udelay(a)

#endif /* __GPIO_BITBANG_H__ */
