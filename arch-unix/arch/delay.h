#ifndef __ARCH_UNIX_DELAY_H__
#define __ARCH_UNIX_DELAY_H__
#include <unistd.h>

static inline int __arch_udelay(int u)
{
	return usleep(u);
}

#define HAS_ARCH_UDELAY

#endif
