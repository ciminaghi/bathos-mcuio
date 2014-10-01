#ifndef __ADC_H__
#define __ADC_H__

#include <bathos/stdio.h>
#include <arch/bathos-arch.h>
#include <arch/adc.h>

extern const uint32_t num_adc;   /* number of supported adc's */
extern const uint32_t min_per_us;/* minimum period, in us */
extern const uint32_t max_mul;   /* max value for multiplier */

#define BATHOS_ADC_FLAG_NONE		0
#define BATHOS_ADC_FLAG_SIGNED	(1 << 0)
#define BATHOS_ADC(idx, f, vr) \
	{.hw_idx = idx, .flags = f, .vref_uv = vr}

struct adc {
	uint8_t flags;		/* flags, see above defines */
	uint8_t hw_idx;	/* idx of adc on hw chip */
	uint32_t vref_uv;	/* Voltage resolution, in microvolts */
};

extern const struct adc adcs[];

/* To be called at startup */
extern int adc_init();

/* enable / disable / enabled status */
extern void adc_en();
extern void adc_dis();
extern int adc_enabled();

/* Correct sequence for sampling is: adc_get, adc_sample, adc_release.
 * adc_sample is arch-specific, get and release are partially
 * defined here (see _adc_get and _adc_release) and partially
 * arch-specific */

extern uint32_t adc_sample(const struct adc *adc);

#endif
