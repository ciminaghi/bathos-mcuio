/*
 * I initially wrote this code for wrpc-sw (ohwr.org)
 *
 * Copyright (C) 2013 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <bathos/bathos.h>
#include <bathos/stdio.h>
#include <bathos/io.h>
#include <bathos/jiffies.h>
#include <bathos/init.h>

static int udelay_lpj; /* loops per jiffy */

static inline void __delay(int count)
{
	while (count-- > 0)
		asm("");
}

static int verify_lpj(int lpj)
{
	unsigned long j;

	/* wait for the beginning of a tick */
	j = jiffies + 1;
	while (jiffies != j)
		;

	__delay(lpj);

	/* did it expire? */
	j = jiffies - j;
	pr_debug("check %i: %li\n", lpj, j);
	return j;
}

static int generic_udelay_init(void)
{
	int step = 512;
	int lpj = step, test_lpj;

	/* Increase until we get over it */
	while (verify_lpj(lpj) == 0) {
		lpj += step;
		step *= 2;
	}
	/* Ok, now we are over; half again and restart */
	lpj /= 2; step /= 4;

	/* So, *this* jpj is lower, and with two steps we are higher */
	while (step) {
		test_lpj = lpj + step;
		if (verify_lpj(test_lpj) == 0)
			lpj = test_lpj;
		step /= 2;
	}
	udelay_lpj = lpj;
	pr_debug("Loops per jiffy: %i\n", lpj);
	return 0;
}

#ifndef ARCH_NEEDS_INTERRUPTS_FOR_JIFFIES
/* subsys_initcall(generic_udelay_init); */
#else
int udelay_init(void)
{
	return generic_udelay_init();
}
#endif

void generic_udelay(unsigned usec)
{
	/* Sleep 10ms each time, to prevent overlfows */
	const int step = 10 * 1000;
	const int usec_per_jiffy = 1000L * 1000L / HZ;
	const int count_per_step = udelay_lpj * step / usec_per_jiffy;

	if (!count_per_step)
		pr_debug("W! missing udelay init\n");

	while (usec > step)  {
		__delay(count_per_step);
		usec -= step;
	}
	__delay(usec * udelay_lpj / usec_per_jiffy);
}

