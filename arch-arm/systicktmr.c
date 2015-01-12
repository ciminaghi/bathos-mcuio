/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/bitops.h>
#include <bathos/errno.h>
#include <bathos/io.h>
#include <bathos/stdio.h>
#include <bathos/init.h>
#include <arch/hw.h>

#define REG_SYST_CSR	0xe000e010

#define CSR_ENABLE	0x1
#define CSR_TICKINT	0x2
#define CSR_CLOCKSOURCE 0x4

#define REG_SYST_RVR	0xe000e014
#define REG_SYST_CVR	0xe000e018
#define REG_SYST_CALIB	0xe000e01c

#define TENMS_MASK (BIT_MASK(24) - 1)
#define SKEW_MASK  (BIT_MASK(30))

#ifdef CONFIG_CORTEX_M_SYSTICKTMR_USE_EXT_CLOCK
#define SYST_CSR_VALUE (CSR_TICKINT | CSR_CLOCKSOURCE)
#else
#define SYST_CSR_VALUE (CSR_TICKINT)
#endif

/* Gets reload value for external reference clock */
int __attribute__((weak)) mach_get_systick_tmr_reload(uint32_t *out)
{
	uint32_t calib = regs[REG_SYST_CALIB];
	uint32_t tenms = calib & TENMS_MASK;
	uint32_t reload;

	if (!tenms) {
		printf("%s: calib register is 0, error initializing tick tmr\n",
		       __func__);
		return -EINVAL;
	}
	if (calib & SKEW_MASK)
		printf("WARN: %s: calib reg has SKEW bit set\n", __func__);
	reload = tenms * 100 / HZ;
	if (reload > TENMS_MASK || !reload) {
		printf("ERR: %s: invalid reload value for HZ %u\n", __func__,
		       HZ);
		return -EINVAL;
	}
	*out = reload;
	return 0;
}

#ifdef CONFIG_CORTEXM_SYSTICKTMR_USE_EXT_CLOCK
/* Clock is external reference clock, reload value is machine dependent */
static inline int get_reload(uint32_t *r)
{
	return mach_get_systick_tmr_reload(r);
}
#else
/* Clock is CPU clock */
static inline int get_reload(uint32_t *r)
{
	uint32_t out;

	out = THOS_QUARTZ / HZ;
	if (!out || out > TENMS_MASK) {
		printf("ERR: %s: reload val is out of range\n", __func__);
		return -EINVAL;
	}
	*r = out;
	return 0;
}
#endif

static int init_systick_tmr(void)
{
	int ret = 0;
	uint32_t reload;


	ret = get_reload(&reload);
	if (ret < 0)
		return ret;
	/* Program reload */
	regs[REG_SYST_RVR] = reload;
	/* Clean current value (reset value of register is undefined) */
	regs[REG_SYST_CVR] = 0;
	/* Program CSR register and start timer */
	regs[REG_SYST_CSR] = SYST_CSR_VALUE | CSR_ENABLE;
	return 0;
}
core_initcall(init_systick_tmr);
