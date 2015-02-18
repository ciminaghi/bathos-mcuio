/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 * Modified for KL25Z Davide Ciminaghi <ciminaghi@gnudd.com> 2014
 */
#ifndef __KL25Z_HW_H__
#define __KL25Z_HW_H__

#define IRQ_DMA0		0
#define IRQ_DMA1		1
#define IRQ_DMA2		2
#define IRQ_DMA3		3
#define IRQ_FTFA		5
#define IRQ_PMC			6
#define IRQ_LLWU		7
#define IRQ_I2C0		8
#define IRQ_I2C1		9
#define IRQ_SPI0		10
#define IRQ_SPI1		11
#define IRQ_UART0		12
#define IRQ_UART1		13
#define IRQ_UART2		14
#define IRQ_ADC0		15
#define IRQ_CMP0		16
#define IRQ_TPM0		17
#define IRQ_TPM1		18
#define IRQ_TPM2		19
#define IRQ_RTC			20
#define IRQ_RTC_SEC		21
#define IRQ_PIT			22
#define IRQ_USBOTG		24
#define IRQ_DAC0		25
#define IRQ_TSI0		26
#define IRQ_MCG			27
#define IRQ_LPTMR0		28
#define IRQ_PORTA		30
#define IRQ_PORTD		31

#define REG_SOPT1		(0x40047000 / 4) /* write */

/* SOPT1 Bit Fields */
#define SIM_SOPT1_OSC32KSEL_MASK		0xC0000u
#define SIM_SOPT1_OSC32KSEL_SHIFT		18
#define SIM_SOPT1_OSC32KSEL(x)						\
    (((uint32_t)(((uint32_t)(x)) << SIM_SOPT1_OSC32KSEL_SHIFT)) &	\
     SIM_SOPT1_OSC32KSEL_MASK)
#define SIM_SOPT1_USBVSTBY_MASK			0x20000000u
#define SIM_SOPT1_USBVSTBY_SHIFT		29
#define SIM_SOPT1_USBSSTBY_MASK			0x40000000u
#define SIM_SOPT1_USBSSTBY_SHIFT		30
#define SIM_SOPT1_USBREGEN_MASK			0x80000000u
#define SIM_SOPT1_USBREGEN_SHIFT		31

#define REG_SOPT1CFG		(0x40047004 / 4)
/* SOPT1CFG Bit Fields */
#define SIM_SOPT1CFG_URWE_MASK                   0x1000000u
#define SIM_SOPT1CFG_URWE_SHIFT                  24
#define SIM_SOPT1CFG_UVSWE_MASK                  0x2000000u
#define SIM_SOPT1CFG_UVSWE_SHIFT                 25
#define SIM_SOPT1CFG_USSWE_MASK                  0x4000000u
#define SIM_SOPT1CFG_USSWE_SHIFT                 26


#define REG_SOPT2		(0x40048004 / 4)
/* SOPT2 Bit Fields */
#define SIM_SOPT2_RTCCLKOUTSEL_MASK		0x10u
#define SIM_SOPT2_RTCCLKOUTSEL_SHIFT		4
#define SIM_SOPT2_CLKOUTSEL_MASK		0xE0u
#define SIM_SOPT2_CLKOUTSEL_SHIFT		5
#define SIM_SOPT2_CLKOUTSEL(x)					  \
    (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_CLKOUTSEL_SHIFT)) & \
     SIM_SOPT2_CLKOUTSEL_MASK)
#define SIM_SOPT2_PLLFLLSEL_MASK		0x10000u
#define SIM_SOPT2_PLLFLLSEL_SHIFT		16
#define SIM_SOPT2_USBSRC_MASK			0x40000u
#define SIM_SOPT2_USBSRC_SHIFT			18
#define SIM_SOPT2_TPMSRC_MASK			0x3000000u
#define SIM_SOPT2_TPMSRC_SHIFT			24
#define SIM_SOPT2_TPMSRC(x)				       \
    (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_TPMSRC_SHIFT)) & \
     SIM_SOPT2_TPMSRC_MASK)
#define SIM_SOPT2_UART0SRC_MASK			0xC000000u
#define SIM_SOPT2_UART0SRC_SHIFT		26
#define SIM_SOPT2_UART0SRC(x)					 \
    (((uint32_t)(((uint32_t)(x)) << SIM_SOPT2_UART0SRC_SHIFT)) & \
     SIM_SOPT2_UART0SRC_MASK)


#define REG_SOPT4		(0x4004800C / 4)
#define REG_SOPT5		(0x40048010 / 4)
#define REG_SOPT7		(0x40048018 / 4)
#define REG_SDID		(0x40048024 / 4)

#define REG_SCGC4		(0x40048034 / 4)
#define SIM_SCGC4_I2C0_MASK		0x40
#define SIM_SCGC4_I2C0_SHIFT		6
#define SIM_SCGC4_I2C1_MASK		0x80
#define SIM_SCGC4_I2C1_SHIFT		7
#define SIM_SCGC4_UART0_MASK		0x400
#define SIM_SCGC4_UART0_SHIFT		10
#define SIM_SCGC4_UART1_MASK		0x800
#define SIM_SCGC4_UART1_SHIFT		11
#define SIM_SCGC4_UART2_MASK		0x1000
#define SIM_SCGC4_UART2_SHIFT		12
#define SIM_SCGC4_USBOTG_MASK		0x40000
#define SIM_SCGC4_USBOTG_SHIFT		18
#define SIM_SCGC4_CMP_MASK		0x80000
#define SIM_SCGC4_CMP_SHIFT		19
#define SIM_SCGC4_SPI0_MASK		0x400000u
#define SIM_SCGC4_SPI0_SHIFT		22
#define SIM_SCGC4_SPI1_MASK		0x800000u
#define SIM_SCGC4_SPI1_SHIFT		23

#define REG_SCGC5		(0x40048038 / 4)

/* SCGC5 Bit Fields */
#define SIM_SCGC5_LPTMR_MASK			0x1u
#define SIM_SCGC5_LPTMR_SHIFT			0
#define SIM_SCGC5_TSI_MASK			0x20u
#define SIM_SCGC5_TSI_SHIFT			5
#define SIM_SCGC5_PORTA_MASK			0x200u
#define SIM_SCGC5_PORTA_SHIFT			9
#define SIM_SCGC5_PORTB_MASK			0x400u
#define SIM_SCGC5_PORTB_SHIFT			10
#define SIM_SCGC5_PORTC_MASK			0x800u
#define SIM_SCGC5_PORTC_SHIFT			11
#define SIM_SCGC5_PORTD_MASK			0x1000u
#define SIM_SCGC5_PORTD_SHIFT			12
#define SIM_SCGC5_PORTE_MASK			0x2000u
#define SIM_SCGC5_PORTE_SHIFT			13


#define REG_SCGC6		(0x4004803C / 4)
/* SCGC6 Bit Fields */
#define SIM_SCGC6_FTF_MASK			0x1u
#define SIM_SCGC6_FTF_SHIFT			0
#define SIM_SCGC6_DMAMUX_MASK			0x2u
#define SIM_SCGC6_DMAMUX_SHIFT			1
#define SIM_SCGC6_PIT_MASK			0x800000u
#define SIM_SCGC6_PIT_SHIFT			23
#define SIM_SCGC6_TPM0_MASK			0x1000000u
#define SIM_SCGC6_TPM0_SHIFT			24
#define SIM_SCGC6_TPM1_MASK			0x2000000u
#define SIM_SCGC6_TPM1_SHIFT			25
#define SIM_SCGC6_TPM2_MASK			0x4000000u
#define SIM_SCGC6_TPM2_SHIFT			26
#define SIM_SCGC6_ADC0_MASK			0x8000000u
#define SIM_SCGC6_ADC0_SHIFT			27
#define SIM_SCGC6_RTC_MASK			0x20000000u
#define SIM_SCGC6_RTC_SHIFT			29
#define SIM_SCGC6_DAC0_MASK			0x80000000u
#define SIM_SCGC6_DAC0_SHIFT			31

#define REG_SCGC7		(0x40048040 / 4)
#define REG_CLKDIV1		(0x40048044 / 4)

/* CLKDIV1 Bit Fields */
#define SIM_CLKDIV1_OUTDIV4_MASK		0x70000u
#define SIM_CLKDIV1_OUTDIV4_SHIFT		16
#define SIM_CLKDIV1_OUTDIV4(x)					  \
    (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV1_OUTDIV4_SHIFT)) & \
     SIM_CLKDIV1_OUTDIV4_MASK)
#define SIM_CLKDIV1_OUTDIV1_MASK		0xF0000000u
#define SIM_CLKDIV1_OUTDIV1_SHIFT		28
#define SIM_CLKDIV1_OUTDIV1(x)					  \
    (((uint32_t)(((uint32_t)(x)) << SIM_CLKDIV1_OUTDIV1_SHIFT)) & \
     SIM_CLKDIV1_OUTDIV1_MASK)

#define REG_FCFG1		(0x4004804C / 4)
#define REG_FCFG2		(0x40048050 / 4)
#define REG_UIDMH		(0x40048058 / 4)
#define REG_UIDML		(0x4004805C / 4)
#define REG_UIDL		(0x40048060 / 4)
#define REG_COPC		(0x40048100 / 4)
#define REG_SRVCOP		(0x40048104 / 4)

#define REG_MCG_C1		(0x40064000 / 1)
/* C1 Bit Fields */
#define MCG_C1_IREFSTEN_MASK			0x1u
#define MCG_C1_IREFSTEN_SHIFT			0
#define MCG_C1_IRCLKEN_MASK			0x2u
#define MCG_C1_IRCLKEN_SHIFT			1
#define MCG_C1_IREFS_MASK			0x4u
#define MCG_C1_IREFS_SHIFT			2
#define MCG_C1_FRDIV_MASK			0x38u
#define MCG_C1_FRDIV_SHIFT			3
#define MCG_C1_FRDIV(x)					 \
    (((uint8_t)(((uint8_t)(x)) << MCG_C1_FRDIV_SHIFT)) & \
     MCG_C1_FRDIV_MASK)
#define MCG_C1_CLKS_MASK			0xC0u
#define MCG_C1_CLKS_SHIFT			6
#define MCG_C1_CLKS(x)					\
    (((uint8_t)(((uint8_t)(x)) << MCG_C1_CLKS_SHIFT)) & \
     MCG_C1_CLKS_MASK)

#define REG_MCG_C2		(0x40064001 / 1)
/* C2 Bit Fields */
#define MCG_C2_IRCS_MASK			0x1u
#define MCG_C2_IRCS_SHIFT			0
#define MCG_C2_LP_MASK				0x2u
#define MCG_C2_LP_SHIFT				1
#define MCG_C2_EREFS0_MASK			0x4u
#define MCG_C2_EREFS0_SHIFT			2
#define MCG_C2_HGO0_MASK			0x8u
#define MCG_C2_HGO0_SHIFT			3
#define MCG_C2_RANGE0_MASK			0x30u
#define MCG_C2_RANGE0_SHIFT			4
#define MCG_C2_RANGE0(x)				  \
    (((uint8_t)(((uint8_t)(x)) << MCG_C2_RANGE0_SHIFT)) & \
     MCG_C2_RANGE0_MASK)
#define MCG_C2_LOCRE0_MASK			0x80u
#define MCG_C2_LOCRE0_SHIFT			7

#define REG_MCG_C3		(0x40064002 / 1)
#define REG_MCG_C4		(0x40064003 / 1)
/* C4 Bit Fields */
#define MCG_C4_SCFTRIM_MASK			0x1u
#define MCG_C4_SCFTRIM_SHIFT			0
#define MCG_C4_FCTRIM_MASK			0x1Eu
#define MCG_C4_FCTRIM_SHIFT			1
#define MCG_C4_FCTRIM(x)				  \
    (((uint8_t)(((uint8_t)(x)) << MCG_C4_FCTRIM_SHIFT)) & \
     MCG_C4_FCTRIM_MASK)
#define MCG_C4_DRST_DRS_MASK			0x60u
#define MCG_C4_DRST_DRS_SHIFT			5
#define MCG_C4_DRST_DRS(x)				    \
    (((uint8_t)(((uint8_t)(x)) << MCG_C4_DRST_DRS_SHIFT)) & \
     MCG_C4_DRST_DRS_MASK)
#define MCG_C4_DMX32_MASK			0x80u
#define MCG_C4_DMX32_SHIFT			7

#define REG_MCG_C5		(0x40064004 / 1)
/* C5 Bit Fields */
#define MCG_C5_PRDIV0_MASK			0x1Fu
#define MCG_C5_PRDIV0_SHIFT			0
#define MCG_C5_PRDIV0(x)				  \
    (((uint8_t)(((uint8_t)(x)) << MCG_C5_PRDIV0_SHIFT)) & \
     MCG_C5_PRDIV0_MASK)
#define MCG_C5_PLLSTEN0_MASK			0x20u
#define MCG_C5_PLLSTEN0_SHIFT			5
#define MCG_C5_PLLCLKEN0_MASK			0x40u
#define MCG_C5_PLLCLKEN0_SHIFT			6

#define REG_MCG_C6		(0x40064005 / 1)
/* C6 Bit Fields */
#define MCG_C6_VDIV0_MASK                       0x1Fu
#define MCG_C6_VDIV0_SHIFT                      0
#define MCG_C6_VDIV0(x)					 \
    (((uint8_t)(((uint8_t)(x)) << MCG_C6_VDIV0_SHIFT)) & \
     MCG_C6_VDIV0_MASK)
#define MCG_C6_CME0_MASK                        0x20u
#define MCG_C6_CME0_SHIFT                       5
#define MCG_C6_PLLS_MASK                        0x40u
#define MCG_C6_PLLS_SHIFT                       6
#define MCG_C6_LOLIE0_MASK                      0x80u
#define MCG_C6_LOLIE0_SHIFT                     7

#define REG_MCG_S		(0x40064006 / 1)
/* S Bit Fields */
#define MCG_S_IRCST_MASK                        0x1u
#define MCG_S_IRCST_SHIFT                       0
#define MCG_S_OSCINIT0_MASK                     0x2u
#define MCG_S_OSCINIT0_SHIFT                    1
#define MCG_S_CLKST_MASK                        0xCu
#define MCG_S_CLKST_SHIFT                       2
#define MCG_S_CLKST(x)					\
    (((uint8_t)(((uint8_t)(x)) << MCG_S_CLKST_SHIFT)) & \
     MCG_S_CLKST_MASK)
#define MCG_S_IREFST_MASK                       0x10u
#define MCG_S_IREFST_SHIFT                      4
#define MCG_S_PLLST_MASK                        0x20u
#define MCG_S_PLLST_SHIFT                       5
#define MCG_S_LOCK0_MASK                        0x40u
#define MCG_S_LOCK0_SHIFT                       6
#define MCG_S_LOLS_MASK                         0x80u
#define MCG_S_LOLS_SHIFT                        7

#define REG_MCG_SC		(0x40064008 / 1)
#define REG_MCG_ATCVH		(0x4006400a / 1)
#define REG_MCG_ATCVL		(0x4006400b / 1)
#define REG_MCG_C7		(0x4006400c / 1)
#define REG_MCG_C8		(0x4006400d / 1)
#define REG_MCG_C9		(0x4006400e / 1)
#define REG_MCG_C10		(0x4006400f / 1)

#define PORT_CG(p)		(1 << (9 + p))


#define PORT_BASE		(0x40049000)
#define PORTA			0
#define PORTB			1
#define PORTC			2
#define PORTD			3
#define PORTE			4

#define REG_PORT_PCR(port,pin)	((PORT_BASE + (port)*0x1000 + (pin)*0x4) / 4)
#define REG_PORT_GPCLR(port)	((PORT_BASE + (port)*0x1000 + 0x80) / 4)
#define REG_PORT_GPCHR(port)	((PORT_BASE + (port)*0x1000 + 0x84) / 4)
#define REG_PORT_ISFR(port)	((PORT_BASE + (port)*0x1000 + 0xA0) / 4)

#define REG_OSC0_CR		(0x40065000 / 1)


extern void clocks_init();



#endif /* __KL25Z_HW_H__ */
