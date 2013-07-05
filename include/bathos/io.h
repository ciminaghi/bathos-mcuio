/*
 * Our I/O abstraction(s)
 * Alessandro Rubini, 2009-2013 GNU GPL2 or later
 */
#ifndef __IO_H__
#define __IO_H__
#include <stdint.h>
#include <arch/hw.h>

#ifndef __SIZEOF_INT__
#error "Please define __SIZEOF_INT__ or use a newer compiler"
#endif

/*
 * Our I/O model is based on the regs array. This assumes the regs array
 * lists 32-bit registers, but AVR is an 8-bit architecture, so single it out
 */
#if __SIZEOF_INT__ < 4
extern volatile uint8_t regs[];
#else
extern volatile uint32_t regs[];
#endif

/*
 * Some files use the readl/writel I/O model. Offer them, with their
 * 16-bit and 8-bit equivalents too, but only for 32-bit processors.
 */
#if __SIZEOF_INT__ == 4

static inline uint32_t readl(unsigned long reg)
{
	return regs[reg / 4];
}
static inline void writel(uint32_t val, unsigned long reg)
{
	regs[reg / 4] = val;
}

static inline uint16_t readw(unsigned long reg)
{
	volatile uint16_t *regs16 = (void *)regs;
	return regs16[reg / 2];
}
static inline void writew(uint16_t val, unsigned long reg)
{
	volatile uint16_t *regs16 = (void *)regs;
	regs16[reg / 2] = val;
}

static inline uint8_t readb(unsigned long reg)
{
	volatile uint8_t *regs8 = (void *)regs;
	return regs8[reg];
}
static inline void writeb(uint8_t val, unsigned long reg)
{
	volatile uint8_t *regs8 = (void *)regs;
	regs8[reg] = val;
}

#else /* only readb/writeb for 8-bit processors */

static inline uint8_t readb(unsigned long reg)
{
	return regs[reg];
}
static inline void writeb(uint8_t val, unsigned long reg)
{
	regs[reg] = val;
}

#endif /* __SIZEOF_INT__ */

#endif /* __IO_H__ */
