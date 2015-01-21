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
