/*
 * Our I/O abstraction(s)
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 */
#ifndef __IO_H__
#define __IO_H__
/* Our I/O model is based on the regs array */

extern volatile uint32_t regs[];

/* Some files use the readl/writel I/O model */
static inline uint32_t readl(unsigned long reg)
{
	return regs[reg / 4];
}
static inline void writel(uint32_t val, unsigned long reg)
{
	regs[reg / 4] = val;
}

#endif /* __IO_H__ */
