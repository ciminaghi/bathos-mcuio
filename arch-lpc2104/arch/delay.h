/*
 * Simple hack for a device-specific delay function
 * Alessandro Rubini, 2011, GNU GPL2 or later
 */
#ifndef __LPC2104_DELAY_H__
#define __LPC2104_DELAY_H__

static inline int __arch_udelay(int u)
{
	int ret;
	asm volatile ("0:\tsubs %0,%0,#1\n\tbpl 0b\n"
		      : "=r" (ret) : "0" (u*3) /* 12MHz/4 */ );
	return ret;
}

#define HAS_ARCH_UDELAY

#endif /* __LPC2104_DELAY_H__ */
