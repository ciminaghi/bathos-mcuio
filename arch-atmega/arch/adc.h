/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

/*
 * ATMEGA adc definitions
 */
#ifndef __ATMEGA_ADC_H__
#define __ATMEGA_ADC_H__

#include <bathos/types.h>
#include <bathos/errno.h>
#include <generated/autoconf.h>
#include <avr/io.h>

#if defined CONFIG_MACH_ATMEGA32U4
#include <arch/adc-atmega32u4.h>
#endif
#if defined CONFIG_MACH_ATMEGA8
#include <arch/adc-atmega8.h>
#endif

#endif /* __ATMEGA_ADC_H__ */
