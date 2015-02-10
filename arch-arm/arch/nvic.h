/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <generated/autoconf.h>

#if defined CONFIG_CPU_CORTEX_M0 || defined CONFIG_CPU_CORTEX_M0_PLUS
#include <arch/nvic-cortex-m0.h>
#include <arch/scb-cortex-m0.h>
#endif

