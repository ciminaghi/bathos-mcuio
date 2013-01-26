#ifndef __BATHOS_DELAY_H__
#define __BATHOS_DELAY_H__
#include <arch/delay.h>

#ifndef HAS_ARCH_UDELAY

static inline void __arch_udelay(int usec)
{
	volatile int i = usec * 10;

	while (i--)
		;
}

#endif

static inline void udelay(int usec)
{
	__arch_udelay(usec);
}

static inline void mdelay(int m)
{
	while (m--)
		udelay(1000);
}

#endif /* __BATHOS_DELAY_H__ */
