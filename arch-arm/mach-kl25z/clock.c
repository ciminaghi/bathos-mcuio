#include <stdint.h>
#include <arch/hw.h>
#include <generated/autoconf.h>


#if defined CONFIG_KL25Z_MCG_FEI

/* 32KHz resonator */
void clocks_init(void)
{
	volatile uint8_t *regs8 = (void *)regs;
	uint8_t tmp;

	regs[REG_COPC] = 0;
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

	regs[REG_COPC] = 0;
	/* PORTA = 1 */
	regs[REG_SCGC5] |= SIM_SCGC5_PORTA_MASK;
	/* OUTDIV1 = 1, OUTDIV4 = 1 */
	regs[REG_CLKDIV1] = SIM_CLKDIV1_OUTDIV1(0x01) |
	    SIM_CLKDIV1_OUTDIV4(0x01);
	/*
	 * System oscillator drives 32 kHz clock for various peripherals
	 * (OSC32KSEL=0)
	 */
	regs[REG_SOPT1] &= ~SIM_SOPT1_OSC32KSEL(0x03);
	/* Select PLL as a clock source for various peripherals (PLLFLLSEL=1) */
	/* Clock source for TPM counter clock is MCGFLLCLK or MCGPLLCLK/2 */
	regs[REG_SOPT2] |= SIM_SOPT2_PLLFLLSEL_MASK;
	regs[REG_SOPT2] = (regs[REG_SOPT2] & ~(SIM_SOPT2_TPMSRC(0x02))) | \
	    SIM_SOPT2_TPMSRC(0x01);

	/* PORTA PCR18, ISF = 0, MUX = 0 */
	regs[REG_PORT_PCR(PORTA, 18)] &= ~0x01000700;
	/* PORTA PCR19, ISF = 0, MUX = 0 */
	regs[REG_PORT_PCR(PORTA, 19)] &= ~0x01000700;
	/* Switch to FBE Mode */
	/* OSC0_CR: ERCLKEN=0, EREFSTEN=0, SC2P=0, SC4P=0, SC8P=0, SC16P=0 */
	regs8[REG_OSC0_CR] = 0;
	/* MCG_C2: LOCRE0=0,??=0,RANGE0=2,HGO0=0,EREFS0=1,LP=0,IRCS=0 */
	regs8[REG_MCG_C2] = (MCG_C2_RANGE0(0x02) | MCG_C2_EREFS0_MASK);
	/* MCG_C1: CLKS=2,FRDIV=3,IREFS=0,IRCLKEN=0,IREFSTEN=0 */
	regs8[REG_MCG_C1] = (MCG_C1_CLKS(0x02) | MCG_C1_FRDIV(0x03));
	/* MCG_C4: DMX32=0,DRST_DRS=0 */
	regs8[REG_MCG_C4] &= ~((MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x03)));
	/* MCG_C5: ??=0,PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=1 */
	regs8[REG_MCG_C5] = MCG_C5_PRDIV0(0x01);
	/* MCG_C6: LOLIE0=0,PLLS=0,CME0=0,VDIV0=0 */
	regs8[REG_MCG_C6] = 0;
	/*
	 * Check that the source of the FLL reference clock is
	 * the external reference clock.
	 */
	while((regs8[REG_MCG_S] & MCG_S_IREFST_MASK) != 0);
	/* Wait until external reference */
	while((regs8[REG_MCG_S] & MCG_S_CLKST_MASK) != 8);
	/* Switch to PBE mode */
	/*   Select PLL as MCG source (PLLS=1) */
	regs8[REG_MCG_C6] = MCG_C6_PLLS_MASK;
	/* Wait until PLL locked */
	while((regs8[REG_MCG_S] & MCG_S_LOCK0_MASK) == 0);
	/* Switch to PEE mode */
	/*    Select PLL output (CLKS=0) */
	/*    FLL external reference divider (FRDIV=3) */
	/*    External reference clock for FLL (IREFS=0) */
	regs8[REG_MCG_C1] = MCG_C1_FRDIV(0x03);
	/* Wait until PLL output */
	while((regs8[REG_MCG_S] & MCG_S_CLKST_MASK) != 0x0CU);
}

#elif defined KL25Z_MCG_BLPE

#error "CONFIG_KL25Z_MCG_BLPE presently unsupported"

#else

#error "KL25Z: INVALID CLOCK CONFIGURATION"

#endif
