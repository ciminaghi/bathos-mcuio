#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <avr/io.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/adc.h>
#include <bathos/bitops.h>
#include <bathos/errno.h>

#define NADC 14

const uint32_t PROGMEM num_adc = NADC;
const uint32_t PROGMEM min_per = 4000; /* FIXME should be computed from HZ */
const uint32_t PROGMEM max_mul = 0xffffffff;
uint32_t ch_stat = 0;

struct adc PROGMEM adcs[NADC] = {
	[0 ... NADC - 1] = {
		.nbits = 10,
		.is_signed = 0,
		.tresp_ns = 800},
};

struct adc *adc_get(unsigned adc_id)
{
	struct adc *adc = _adc_get(adc_id);
	if (!adc)
		return NULL;
	/* FIXME: perform registers initialization on adc->id hw ADC */
	return adc;
}

void adc_release(struct adc *adc)
{
	_adc_release(adc);
	/* FIXME: perform registers de-initialization */
}

uint32_t adc_sample(struct adc *adc)
{
	/* FIXME: to do */
	return 0;
}
