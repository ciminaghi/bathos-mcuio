/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <bathos/delay.h>
#include <bathos/stdio.h>
#include <bathos/adc.h>

#include <analogin_api.h>

static analogin_t ain[16]; /* FIXME dinamically allocate ain, depending on
				num_adc */

int adc_init()
{
	int i;

	for (i = 0; i < num_adc; i++) {
		analogin_init(&ain[i], adcs[i].hw_idx);
	}

	return 0;
}

core_initcall(adc_init);

void adc_en()
{
}

void adc_dis()
{
}

int adc_enabled()
{
	return 1;
}

void flip4(uint8_t *ptr)
{
	/* FIXME */
}

uint32_t adc_sample(const struct adc *adc)
{
	return analogin_read_u16(&ain[adc - adcs]);
}
