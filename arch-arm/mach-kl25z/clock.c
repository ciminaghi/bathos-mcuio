#include <stdint.h>
#include <arch/hw.h>
#include <generated/autoconf.h>


#if defined CONFIG_KL25Z_MCG_FEI

/* 32KHz resonator */
void clocks_init(void)
{
	volatile uint8_t *regs8 = (void *)regs;
	uint8_t tmp;

	/* OUTDIV1 = 0, OUTDIV4 = 2 */
	regs[REG_CLKDIV1] = 0x00020000UL;
	/* CLKS = 0, FRDIV = 0, IREFS = 1, IRCLKEN = 1, IREFSTEN = 0 */
	regs8[REG_MCG_C1] = 0x06;
	/* LOCRE0 = 0, RANGE0 = 0, HGO0 = 0, EREFS0 = 0, LP = 0, IRCS = 0 */
	regs8[REG_MCG_C2] = 0;
	/* DMX32 = 0, DRST_DRS = 1 */
	tmp = regs8[REG_MCG_C4] & ~0xc0;
	regs8[REG_MCG_C4] = tmp | 0x20;
	/* ERCLKEN=1, EREFSTEN=0, SC2P=0, SC4P=0, SC8P=0, SC16P=0 */
	regs8[REG_OSC0_CR] = 0x80;
	/* PLLCLKEN0 = 0, PLLSTEN0 = 0, PRDIV0 = 0 */
	regs8[REG_MCG_C5] = 0;
	/*  LOLIE0 = 0, PLLS = 0, CME0 = 0, VDIV0 = 0 */
	regs8[REG_MCG_C6] = 0;
	/*
	 * Check that the source of the FLL reference clock is the internal
	 * reference clock
	 */
	while(!(regs8[REG_MCG_S] & MCG_S_IREFST_MASK));
	/* Wait until output of the FLL is selected */
	while((regs8[REG_MCG_S] & 0x0c));
}

#elif defined CONFIG_KL25Z_MCG_PEE

/* External xtal */
void clocks_init(void)
{
	volatile uint8_t *regs8 = (void *)regs;

	/* PORTA = 1 */
	regs[REG_SCGC5] |= 0x200;
	/* OUTDIV1 = 1, OUTDIV4 = 1 */
	regs[REG_CLKDIV1] = 0x10010000;
	/* PORTA PCR18, ISF = 0, MUX = 0 */
	regs[REG_PORT_PCR(PORTA, 18)] &= ~0x01000700;
	/* PORTA PCR19, ISF = 0, MUX = 0 */
	regs[REG_PORT_PCR(PORTA, 19)] &= ~0x01000700;
	/* ERCLKEN = 1, EREFSTEN = 0, SC2P = 1, SC4P = 0, SC8P = 0, SC16P = 1 */
	regs8[REG_OSC0_CR] = 0x89;
	/* LOCRE0=0, RANGE0=2, HGO0=0, EREFS0=1, LP=0, IRCS=0 */
	regs8[REG_MCG_C2] = 0x24;
	/* CLKS = 2, FRDIV = 3, IREFS = 0, IRCLKEN = 1, IREFSTEN = 0 */
	regs8[REG_MCG_C1] = 0x9a;
	/* DMX32 = 0, DRST_DRS = 0 */
	regs8[REG_MCG_C4] &= ~0xe0;
	/* PLLCLKEN0 = 0, PLLSTEN0 = 0, PRDIV0 = 1 */
	regs8[REG_MCG_C5] &= 0x01;
	/* LOLIE0 = 0, PLLS = 0, CME0 = 0, VDIV0 = 0 */
	regs8[REG_MCG_C6] = 0;
	/*
	 * Check that the source of the FLL reference clock is the external
	 * reference clock
	 */
	while((regs8[REG_MCG_S] & MCG_S_IREFST_MASK));
	/* Wait until external reference clock is selected as MCG output */
	while((regs8[REG_MCG_S] & 0x0c) != 0x08);
	/* Switch to PBE Mode */
	/* LOLIE0 = 0, PLLS = 1, CME0 = 0, VDIV0 = 0 */
	regs8[REG_MCG_C6] = 0x40;
	/* Wait until external reference clock is selected as MCG output */
	while((regs8[REG_MCG_S] & 0x0c) != 0x08);
	/* Wait until locked */
	while(!(regs8[REG_MCG_S] & MCG_S_LOCK0_MASK));
	/* Switch to PEE Mode */
	/* CLKS = 0, FRDIV = 3, IREFS = 0, IRCLKEN = 1, IREFSTEN = 0 */
	regs8[REG_MCG_C1] = 0x1a;
	/* Wait until output of the PLL is selected */
	while((regs8[REG_MCG_S] & 0x0c) != 0x0c);
}

#elif defined KL25Z_MCG_BLPE

#error "CONFIG_KL25Z_MCG_BLPE presently unsupported"

#else

#error "KL25Z: INVALID CLOCK CONFIGURATION"

#endif
