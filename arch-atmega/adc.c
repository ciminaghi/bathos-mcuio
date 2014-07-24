#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <avr/io.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/adc.h>
#include <bathos/bitops.h>
#include <bathos/errno.h>
#include <bathos/delay.h>
#include <bathos/stdio.h>

#define NADC 8

const uint32_t PROGMEM num_adc = NADC;
const uint32_t PROGMEM min_per = 4000; /* FIXME should be computed from HZ */
const uint32_t PROGMEM max_mul = 0xffffffff;
uint32_t ch_stat = 0;

const struct adc PROGMEM adcs[NADC] = {
	[0 ... NADC - 1] = {
		.nbits = 10,
		.is_signed = 0,
		.tresp_ns = 8000},
};

int adc_init()
{
	int i;
	/* set reference to Vcc */
	adc_set_ref(ADMUX_AVCC);

	/* set ADC prescalar to 128 - max resolution */
	adc_set_ps(0x7);

	/* disable all */
	adc_dis();
	for (i = 0; i < NADC; i++)
		adc_dis_in(i);

	return 0;
}

rom_initcall(adc_init);

const struct adc *adc_get(unsigned adc_id)
{
	const struct adc *adc = _adc_get(adc_id);
	if (!adc)
		return NULL;
	adc_en_in(adc_id);
	adc_sel_in(adc_id);
	return adc;
}

void adc_release(const struct adc *adc)
{
	int adc_id = _adc_id(adc);
	_adc_release(adc);
	adc_dis_in(adc_id);
}

uint32_t adc_sample(const struct adc *adc)
{
	adc_start();
	while (ADCSRA & (1 << ADSC));
	return adc_data();
}
