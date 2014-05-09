#ifndef __ADC_H__
#define __ADC_H__

#include <avr/io.h>

extern const uint32_t num_adc;   /* number of supported adc's */
extern const uint32_t min_per_us;/* minimum period, in us */
extern const uint32_t max_mul;   /* max value for multiplier */

struct adc {
	uint8_t nbits;
	uint8_t is_signed;
	uint32_t tresp_ns;
};

extern struct adc adcs[];

/* To be called at startup */
extern void adc_init();

/* Correct sequence for sampling is: adc_get, adc_sample, adc_release.
 * adc_sample is arch-specific, get and release are partially
 * defined here (see _adc_get and _adc_release) and partially
 * arch-specific */

extern uint32_t adc_sample(struct adc *adc);
extern struct adc *adc_get(unsigned adc_id);
extern void adc_release(struct adc *adc);

/* _adc_get and _adc_release are macros to be called by arch-specific
 * code */
uint32_t ch_stat = 0;

static inline int _adc_id(struct adc *adc)
{
	return adc - adcs;
}

static inline struct adc *_adc_get(unsigned adc_id)
{
	struct adc *adc;
	if (adc_id >= num_adc)
		return NULL;
	adc = &adcs[adc_id];
	if (ch_stat & (1 << adc_id))
		return NULL;
	ch_stat |= (1 << adc_id);
	return &adcs[adc_id];
}

static inline void _adc_release(struct adc *adc)
{
	ch_stat &= ~(1 << _adc_id(adc));
}

#endif
