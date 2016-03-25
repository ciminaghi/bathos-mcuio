/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 * Modified for STM32F4X by Davide Ciminaghi <ciminaghi@gnudd.com> 2014
 */
#ifndef __STM32F4X_CLOCKS_H__
#define __STM32F4X_CLOCKS_H__

#include <bathos/bitops.h>
#include <bathos/io.h>
#include <mach/hw.h>
#include <stdint.h>
#include <arch/bathos-arch.h>

#define RCC_CR				0x0
# define HSION				BIT(0)
# define HSIRDY				BIT(1)
# define HSEON				BIT(16)
# define HSERDY				BIT(17)
# define HSEBYP				BIT(18)
# define CSSON				BIT(19)
# define PLLON				BIT(24)
# define PLLRDY				BIT(25)
# define PLLI2SON			BIT(26)
# define PLLI2SRDY			BIT(27)
# define PLLSAION			BIT(28)
# define PLLSAYRDY			BIT(29)

#define RCC_PLLCFGR			0x4
#define RCC_CFGR			0x8

enum pllsrc {
	HSI = 0,
	HSE = 1,
};

enum apb_divider {
	APB_ERROR = -1,
	APB_1 = 0,
	APB_2 = 4,
	APB_4 = 5,
	APB_8 = 6,
	APB_16 = 7,
};

enum ahb_divider {
	AHB_ERROR = -1,
	AHB_1 = 0,
	AHB_2 = 8,
	AHB_4 = 9,
	AHB_8 = 10,
	AHB_16 = 11,
	AHB_64 = 12,
	AHB_128 = 13,
	AHB_256 = 14,
	AHB_512 = 15,
};

enum stm32f4x_bus_id {
	AHB1 = 1,
	AHB2,
	AHB3,
	APB1,
	APB2,
};

static inline int stm32f4x_setup_pll(uint8_t _m, uint16_t _n, uint8_t _p,
				     enum pllsrc _src, uint8_t _q, uint8_t _r)
{
	uint32_t m = _m, n = _n, p = _p, src = _src, q = _q, r = _r;

	if (m < 2 || m > 63)
		return -1;
	if (n < 50 || n > 432)
		return -1;
	switch (p) {
	case 2:
		p = 0;
		break;
	case 4:
		p = 1;
		break;
	case 6:
		p = 2;
		break;
	case 8:
		p = 3;
		break;
	default:
		return -1;

	}
	if (src != HSI && src != HSE)
		return -1;
	if (q < 2 || q > 15)
		return -1;
	if (r < 2 || r > 7)
		return -1;
	writel(m | (n << 6) |
	       (p << 16) | (src << 22) | (q << 24) |
	       (r << 28), RCC_BASE + RCC_PLLCFGR);
	return 0;
}

static inline void stm32f4x_setup_prescalers(enum apb_divider apb1,
					    enum apb_divider apb2,
					    enum ahb_divider ahb)
{
	uint32_t tmp = readl(RCC_BASE + RCC_CFGR);

	tmp &= ~((7 << 13) | (7 << 10) | (0xf << 4));
	tmp |= (((uint32_t)apb1 << 10) | ((uint32_t)apb2 << 13) |
		((uint32_t)ahb << 4));
	writel(tmp, RCC_BASE + RCC_CFGR);
}

extern int stm32f4x_enable_peripheral_clock(enum stm32f4x_bus_id,
					    int periph_id);

#endif /* __STM32F4X_CLOCKS_H__ */
