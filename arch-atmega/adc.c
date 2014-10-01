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

#define VRES_UV (5000000 / (1 << 10))

/* Order of this mapping is Arduino YUN / Linino ONE */
const struct adc PROGMEM adcs[] = {
	BATHOS_ADC(7, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A0 */
	BATHOS_ADC(6, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A1 */
	BATHOS_ADC(5, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A2 */
	BATHOS_ADC(4, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A3 */
	BATHOS_ADC(1, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A4 */
	BATHOS_ADC(0, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A5 */
};

const uint32_t PROGMEM num_adc = ARRAY_SIZE(adcs);
const uint32_t PROGMEM min_per = (1000000 / HZ);
const uint32_t PROGMEM max_mul = 0xffffffff;

int adc_init()
{
	int i;
	/* set reference to Vcc */
	adc_set_ref(ADMUX_AVCC);

	/* set ADC prescalar to 128 - max resolution */
	adc_set_ps(0x7);

	/* disable all */
	adc_dis();
	for (i = 0; i < ARRAY_SIZE(adcs); i++)
		adc_dis_in(adcs[i].hw_idx);

	return 0;
}

rom_initcall(adc_init);

uint32_t adc_sample(const struct adc *adc)
{
	uint32_t data;
	adc_en_in(adc->hw_idx);
	adc_sel_in(adc->hw_idx);
	adc_start();
	while (ADCSRA & (1 << ADSC));
	data = adc_data();
	adc_dis_in(adc->hw_idx);
	return data;
}
