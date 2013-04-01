#ifndef __BATHOS_DELAY_H__
#define __BATHOS_DELAY_H__
#include <arch/delay.h>

#ifdef HAS_ARCH_UDELAY

static inline void udelay(int usec)
{
	__arch_udelay(usec);
}

#else

extern void generic_udelay(int usec);

static inline void udelay(int usec)
{
	generic_udelay(usec);
}

#endif

static inline void mdelay(int m)
{
	while (m--)
		udelay(1000);
}

#endif /* __BATHOS_DELAY_H__ */
