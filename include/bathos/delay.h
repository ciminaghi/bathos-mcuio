#ifndef __BATHOS_DELAY_H__
#define __BATHOS_DELAY_H__
#include <arch/bathos-arch.h>
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

#ifdef ARCH_NEEDS_INTERRUPTS_FOR_JIFFIES
/*
 * Initcalls are run with interrupts disabled. If interrupts are needed
 * to get the jiffies counter going, the udelay init function shall be called
 * later on, when interrupts are enabled
 */
extern int udelay_init(void);
#else
static inline int udelay_init(void)
{
	return 0;
}
#endif

static inline void mdelay(int m)
{
	while (m--)
		udelay(1000);
}

#endif /* __BATHOS_DELAY_H__ */
