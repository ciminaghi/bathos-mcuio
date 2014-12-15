/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <bathos/init.h>
#include <bathos/types.h>
#include <bathos/gpio.h>
#include <mach/gpio.h>

/* ADCs */

#define VRES_UV (5000000 / (1 << 10))

#ifdef CONFIG_MBED_DRIVERS

#include <analogin_api.h>

const struct adc adcs[] = {
	BATHOS_ADC((uint8_t)PTE30, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A0 */
	BATHOS_ADC((uint8_t)PTE29, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A1 */
	BATHOS_ADC((uint8_t)PTE24, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A2 */
	BATHOS_ADC((uint8_t)PTE25, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A3 */
	BATHOS_ADC((uint8_t)PTE21, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A4 */
	BATHOS_ADC((uint8_t)PTE20, BATHOS_ADC_FLAG_NONE, VRES_UV), /* A5 */
};

const uint32_t num_adc = ARRAY_SIZE(adcs);
const uint32_t min_per = (1000000 / HZ);
const uint32_t max_mul = 0xffffffff;

#endif

/* GPIOs */
char gpio_labels_start[MCUIO_NGPIO * 4];
char gpio_caps_start[MCUIO_NGPIO];
char gpio_evts_caps_start[MCUIO_NGPIO];

static int __gpio_init(void)
{
	int i, n;
	/* FIXME this configures a full-featured set of gpios. Must be fixed
	 * with actual values */
	for (i = 0; i < MCUIO_NGPIO; i++) {
		n = i % 32;
		gpio_labels_start[i*4 + 3] = 'A' + (i / 32);
		gpio_labels_start[i*4 + 2] = '0' + (n / 10);
		gpio_labels_start[i*4 + 1] = '0' + (n % 10);
		gpio_labels_start[i*4] = '\0';
		gpio_caps_start[i] = 0xff;
		gpio_evts_caps_start[i] = 0xff;
	}
	return 0;
}

core_initcall(__gpio_init);
