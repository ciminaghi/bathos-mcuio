/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 * Modified for KL25Z Davide Ciminaghi <ciminaghi@gnudd.com> 2014
 */
#ifndef __KL25Z_HW_H__
#define __KL25Z_HW_H__

#define NVIC_IRQ_DMA0 		0
#define NVIC_IRQ_DMA1 		1
#define NVIC_IRQ_DMA2 		2
#define NVIC_IRQ_DMA3 		3
#define NVIC_IRQ_FTFA 		5
#define NVIC_IRQ_PMC 		6
#define NVIC_IRQ_LLWU		7
#define NVIC_IRQ_I2C0		8
#define NVIC_IRQ_I2C1		9
#define NVIC_IRQ_SPI0		10
#define NVIC_IRQ_SPI1		11
#define NVIC_IRQ_UART0		12
#define NVIC_IRQ_UART1		13
#define NVIC_IRQ_UART2		14
#define NVIC_IRQ_ADC0		15
#define NVIC_IRQ_CMP0		16
#define NVIC_IRQ_TPM0		17
#define NVIC_IRQ_TPM1		18
#define NVIC_IRQ_TPM2		19
#define NVIC_IRQ_RTC		20
#define NVIC_IRQ_RTC_SEC	21
#define NVIC_IRQ_PIT		22
#define NVIC_IRQ_USBOTG		24
#define NVIC_IRQ_DAC0		25
#define NVIC_IRQ_TSI0		26
#define NVIC_IRQ_MCG		27
#define NVIC_IRQ_LPTMR0		28
#define NVIC_IRQ_PORTA		30
#define NVIC_IRQ_PORTD		31

#define REG_PIT_MCR		(0x40037000 / 4)
#define PIT_MCR_FRZ_MASK	0x1u
#define PIT_MCR_MDIS_MASK	0x2u

#define REG_PIT_LTMR64H		(0x400370E0 / 4)
#define REG_PIT_LTMR64L		(0x400370E4 / 4)
#define REG_PIT_LDVAL(ch)	((0x40037100 + (ch)*0x10) / 4)
#define REG_PIT_CVAL(ch)	((0x40037104 + (ch)*0x10) / 4)

#define REG_PIT_TCTRL(ch)	((0x40037108 + (ch)*0x10) / 4)
#define PIT_TCTRL_TEN_MASK	0x1u
#define PIT_TCTRL_TIE_MASK	0x2u
#define PIT_TCTRL_CHN_MASK	0x4u

#define REG_PIT_TFLG(ch)	((0x4003710C + (ch)*0x10) / 4)
#define PIT_TFLG_TIF_MASK	0x1u

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

#define REG_SIM_SCGC5		(0x40048038 / 4)

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

#define GPIO_PORT_BASE		(0x400ff000)

#define REG_GPIO_PDOR(port)	((GPIO_PORT_BASE + (port)*0x40) / 4)
#define REG_GPIO_PSOR(port)	(((GPIO_PORT_BASE + (port)*0x40 + 0x4) / 4))
#define REG_GPIO_PCOR(port)	(((GPIO_PORT_BASE + (port)*0x40 + 0x8) / 4))
#define REG_GPIO_PTOR(port)	(((GPIO_PORT_BASE + (port)*0x40 + 0xc) / 4))
#define REG_GPIO_PDIR(port)	(((GPIO_PORT_BASE + (port)*0x40 + 0x10) / 4))
#define REG_GPIO_PDDR(port)	(((GPIO_PORT_BASE + (port)*0x40 + 0x14) / 4))

#define REG_UART_BDH(n)		(0x4006a000 + (n)*0x1000)
#define REG_UART_BDL(n)		(0x4006a001 + (n)*0x1000)
#define REG_UART_C1(n)		(0x4006a002 + (n)*0x1000)

#define REG_UART_C2(n)		(0x4006a003 + (n)*0x1000)
#define UART_C2_TIE_MASK	0x80
#define UART_C2_TCIE_MASK	0x40
#define UART_C2_RIE_MASK	0x20
#define UART_C2_ILIE_MASK	0x10
#define UART_C2_TE_MASK		0x08
#define UART_C2_RE_MASK		0x04
#define UART_C2_RWU_MASK	0x02
#define UART_C2_SBK_MASK	0x01

#define REG_UART_S1(n)		(0x4006a004 + (n)*0x1000)
#define UART_S1_TDRE_MASK	0x80
#define UART_S1_TC_MASK		0x40
#define UART_S1_RDRF_MASK	0x20
#define UART_S1_IDLE_MASK	0x10
#define UART_S1_OR_MASK		0x08
#define UART_S1_NF_MASK		0x04
#define UART_S1_FE_MASK		0x02
#define UART_S1_PF_MASK		0x01

#define REG_UART_S2(n)		(0x4006a005 + (n)*0x1000)
#define REG_UART_C3(n)		(0x4006a006 + (n)*0x1000)
#define REG_UART_D(n)		(0x4006a007 + (n)*0x1000)

#define REG_USB0_PERID		(0x40072000 / 1)
#define REG_USB0_IDCOMP		(0x40072004 / 1)
#define REG_USB0_REV		(0x40072008 / 1)
#define REG_USB0_ADDINFO	(0x4007200c / 1)
#define REG_USB0_OTGISTAT	(0x40072010 / 1)
#define REG_USB0_OTGICR		(0x40072014 / 1)
#define REG_USB0_OTGSTAT	(0x40072018 / 1)
#define REG_USB0_OTGCTL		(0x4007201c / 1)

#define REG_USB0_ISTAT		(0x40072080 / 1)
/* USB_ISTAT and USB_INTEN mask */
#define USB_ISTAT_STALL_MASK		0x80
#define USB_ISTAT_ATTACH_MASK		0x40
#define USB_ISTAT_RESUME_MASK		0x20
#define USB_ISTAT_SLEEP_MASK		0x10
#define USB_ISTAT_TOKDNE_MASK		0x08
#define USB_ISTAT_SOFTOK_MASK		0x04
#define USB_ISTAT_ERROR_MASK		0x02
#define USB_ISTAT_USBRST_MASK		0x01

#define REG_USB0_INTEN		(0x40072084 / 1)
#define REG_USB0_ERRSTAT	(0x40072088 / 1)
#define REG_USB0_ERREN		(0x4007208c / 1)
#define REG_USB0_STAT		(0x40072090 / 1)

#define REG_USB0_CTL		(0x40072094 / 1)
/* USB_CTL mask */
#define USB_CTL_JSTATE_MASK		0x80
#define USB_CTL_SE0_MASK		0x40
#define USB_CTL_TXSUSPENDTOKENB_MASK	0x20
#define USB_CTL_RESET_MASK		0x10
#define USB_CTL_HOSTMODEEN_MASK		0x08
#define USB_CTL_RESUME_MASK		0x04
#define USB_CTL_ODDRST_MASK		0x02
#define USB_CTL_USBENSOFEN_MASK		0x01

#define REG_USB0_ADDR		(0x40072098 / 1)
#define REG_USB0_BDTPAGE1	(0x4007209c / 1)
#define REG_USB0_FRMNUML	(0x400720a0 / 1)
#define REG_USB0_FRMNUMH	(0x400720a4 / 1)
#define REG_USB0_TOKEN		(0x400720a8 / 1)
#define REG_USB0_SOFTHLD	(0x400720ac / 1)
#define REG_USB0_BDTPAGE2	(0x400720b0 / 1)
#define REG_USB0_BDTPAGE3	(0x400720b4 / 1)
#define REG_USB0_ENDPT(n)	((0x400720c0 + (n)*0x4) / 1)
#define REG_USB0_USBCTRL	(0x40072100 / 1)
#define REG_USB0_OBSERVE	(0x40072104 / 1)

#define REG_USB0_CONTROL	(0x40072108 / 1)
#define USB_CONTROL_DPPULLOPNONOTG_MASK	0x10

#define REG_USB0_USBTRC0	(0x4007210c / 1)
#define REG_USB0_USBFRMADJUST	(0x40072114 / 1)

extern void clocks_init();

extern void jiffies_init();

#endif /* __KL25Z_HW_H__ */
