/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

/* cortex-m0 registers */

#define REG_NVIC_ISER		(0xe000e100 / 4)
#define REG_NVIC_ICER		(0xe000e180 / 4)
#define REG_NVIC_ISPR		(0xe000e200 / 4)
#define REG_NVIC_ICPR		(0xe000e280 / 4)
#define REG_NVIC_IP(prio)	((0xe000e400 + (prio)*0x4) / 4)

#define REG_SCB_CPUID		(0xe000ed00 / 4)
#define REG_SCB_ICSR		(0xe000ed04 / 4)
#define REG_SCB_VTOR		(0xe000ed08 / 4)
#define REG_SCB_AIRCR		(0xe000ed0c / 4)
#define REG_SCB_SCR		(0xe000ed10 / 4)
#define REG_SCB_CCR		(0xe000ed14 / 4)
#define REG_SCB_SHP2		(0xe000ed1c / 4)
#define REG_SCB_SHP3		(0xe000ed20 / 4)
