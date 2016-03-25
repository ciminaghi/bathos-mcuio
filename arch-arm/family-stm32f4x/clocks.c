/*
 * Clocks initializations for the STM32FX family
 *
 * Copyright DOG HUNTER 2016
 * Author Davide Ciminaghi 2016
 * GPLv2 or later
 */
#include <generated/autoconf.h>
#include <family/clocks.h>

static enum apb_divider _apb_divider_to_val(unsigned int divider)
{
	switch (divider) {
	case 1:
		return APB_1;
	case 2:
		return APB_2;
	case 4:
		return APB_4;
	case 8:
		return APB_8;
	case 16:
		return APB_16;
	default:
		return APB_ERROR;
	}
	return APB_ERROR;
}

static enum ahb_divider _ahb_divider_to_val(unsigned int divider)
{
	switch (divider) {
	case 1:
		return AHB_1;
	case 2:
		return AHB_2;
	case 4:
		return AHB_4;
	case 8:
		return AHB_8;
	case 16:
		return AHB_16;
	case 64:
		return AHB_64;
	case 128:
		return AHB_128;
	case 256:
		return AHB_256;
	case 512:
		return AHB_512;
	default:
		return AHB_ERROR;
	}
	return AHB_ERROR;
}

/*
 * Main pll setup
 *
 * Fvco = Fpll_in * (PLLN / PLLM)
 * Fpll = Fvco / PLLP
 *
 * Clock source is HSE, 8MHz
 * N = 180, M = 4, P = 2  =>
 * Fvco = 8 * (180 / 4) = 360
 * Fpll = 360 / 2 = 180
 */
int stm32f4x_sysclock_init(void)
{
	uint32_t tmp;
	enum apb_divider apb1, apb2;
	enum ahb_divider ahb;

	/* Disable pll and HSE anyway */
	tmp = readl(RCC_BASE + RCC_CR);
	tmp &= ~(PLLON | HSEON);
	writel(tmp, RCC_BASE + RCC_CR);
	while (readl(RCC_BASE + RCC_CR) & (tmp & PLLRDY))
	    ;

	/* Configure main pll */
	if (stm32f4x_setup_pll(CONFIG_STM32F4X_PLL_M,
			       CONFIG_STM32F4X_PLL_N,
			       CONFIG_STM32F4X_PLL_P,
			       HSE,
			       CONFIG_STM32F4X_PLL_Q,
			       CONFIG_STM32F4X_PLL_R) < 0)
		return -1;
		
	/* Setup prescalers (APB2 -> /2 (90MHz), APB1 -> /4 (45MHz), AHB /1 */
	apb1 = _apb_divider_to_val(CONFIG_STM32F4X_APB1_PRESC);
	apb2 = _apb_divider_to_val(CONFIG_STM32F4X_APB2_PRESC);
	ahb = _ahb_divider_to_val(CONFIG_STM32F4X_AHB_PRESC);
	if ((apb1 < 0) || (apb2 < 0) || (ahb < 0))
		return -1;
	stm32f4x_setup_prescalers(apb1, apb2, ahb);

	/* Turn HSE on and wait for HSE steady */
	tmp = readl(RCC_BASE + RCC_CR);
	tmp |= HSEON;
	writel(tmp, RCC_BASE + RCC_CR);
	while (!(readl(RCC_BASE + RCC_CR) & HSERDY))
	    ;

	/* Turn main pll on and wait for pll lock */
	tmp = readl(RCC_BASE + RCC_CR);
	tmp |= PLLON;
	writel(tmp, RCC_BASE + RCC_CR);
	while (!(readl(RCC_BASE + RCC_CR) & PLLRDY))
	    ;
	/* Set PLLP as main system clock */
	tmp = readl(RCC_BASE + RCC_CFGR);
	tmp &= ~3;
	tmp |= 2;
	writel(tmp, RCC_BASE + RCC_CFGR);
	return 0;
}

int stm32f4x_enable_peripheral_clock(enum stm32f4x_bus_id bus, int periph_id)
{
	uint32_t tmp;
	static const int offsets[] = {
		[AHB1] = 0x30,
		[AHB2] = 0x34,
		[AHB3] = 0x38,
		[APB1] = 0x40,
		[APB2] = 0x44,
	};
	int offset = offsets[bus];

	if (bus < AHB1 || bus > APB2)
		return -1;
	tmp = readl(RCC_BASE + offset);
	tmp |= 1 << periph_id;
	writel(tmp, RCC_BASE + offset);
	return 0;
}
