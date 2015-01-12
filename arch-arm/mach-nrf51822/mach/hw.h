/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 * Modified for KL25Z Davide Ciminaghi <ciminaghi@gnudd.com> 2014
 */
#ifndef __NRF51822_HW_H__
#define __NRF51822_HW_H__

#define POWER_CLOCK_IRQ 0
#define RADIO_IRQ	1
#define UART0_IRQ	2
#define SPI0_TWI0_IRQ	3
#define SPI1_TWI1_IRQ	4
#define GPIOTE_IRQ	6
#define ADC_IRQ		7
#define TIMER0_IRQ	8
#define TIMER1_IRQ	9
#define TIMER2_IRQ	10
#define RTC0_IRQ	11
#define TEMP_IRQ	12
#define RNG_IRQ		13
#define ECB_IRQ		14
#define CCM_AAR_IRQ	15
#define WDT_IRQ		16
#define RTC1_IRQ	17
#define QDEC_IRQ	18
#define LPCOMP_COMP_IRQ 19
#define SWI0_IRQ	20
#define SWI1_IRQ	21
#define SWI2_IRQ	22
#define SWI3_IRQ	23
#define SWI4_IRQ	24
#define SWI5_IRQ	25

/* Peripheral ids */
#define POWER_ID	0x00
#define CLOCK_ID	0x00
#define MPU_ID		0x00
#define PU_ID		0x00
#define AMLI_ID		0x00
#define RADIO_ID	0x01
#define UART0_ID	0x02
#define SPI0_ID		0x03
#define TWI0_ID		0x03
#define SPI1_ID		0x04
#define TWI1_ID		0x04
#define SPIS1_ID	0x04
#define GPIO_ID		0x06
#define GPIOTE_ID	0x06
#define ADC_ID		0x07
#define TIMER0_ID	0x08
#define TIMER1_ID	0x09
#define TIMER2_ID	0x0a
#define RTC0_ID		0x0b
#define TEMP_ID		0x0c
#define RNG_ID		0x0d
#define ECB_ID		0x0e
#define AAR_ID		0x0f
#define CCM_ID		0x0f
#define WDT_ID		0x10
#define RTC1_ID		0x11
#define QDEC_ID		0x12
#define LPCOMP_ID	0x13
#define COMP_ID		0x13
#define SWI_ID		0x14
#define NVMC_ID		0x1e
#define PPI_ID		0x1f

/* Peripheral modules base addresses */

#define BASE(id) (0x40000000U + (0x1000 * (id)))

#define POWER_BASE	BASE(POWER_ID)
#define CLOCK_BASE	BASE(CLOCK_ID)
#define MPU_BASE	BASE(MPU_ID)
#define PU_BASE		BASE(PU_ID)
#define AMLI_BASE	BASE(AMLI_ID)
#define RADIO_BASE	BASE(RADIO_ID)
#define UART0_BASE	BASE(UART0_ID)
#define SPI0_BASE	BASE(SPIO_ID)
#define TWI0_BASE	BASE(TWI0_ID)
#define SPI1_BASE	BASE(SPI1_ID)
#define TWI1_BASE	BASE(TWI1_ID)
#define SPIS1_BASE	BASE(SPIS1_ID)
#define GPIOTE_BASE	BASE(GPIOTE_ID)
#define ADC_BASE	BASE(ADC_ID)
#define TIMER0_BASE	BASE(TIMER0_ID)
#define TIMER1_BASE	BASE(TIMER1_ID)
#define TIMER2_BASE	BASE(TIMER2_ID)
#define RTC0_BASE	BASE(RTC0_ID)
#define TEMP_BASE	BASE(TEMP_ID)
#define RNG_BASE	BASE(RNG_ID)
#define ECB_BASE	BASE(ECB_ID)
#define AAR_BASE	BASE(AAR_ID)
#define CCM_BASE	BASE(CCM_ID)
#define WDT_BASE	BASE(WDT_ID)
#define RTC1_BASE	BASE(RTC1_ID)
#define QDEC_BASE	BASE(QDEC_ID)
#define LPCOMP_BASE	BASE(LPCOMP_ID)
#define COMP_BASE	BASE(COMP_ID)
#define SWI_BASE	BASE(SWI_ID)
#define NVMC_BASE	BASE(NVMC_ID)
#define PPI_BASE	BASE(PPI_ID)
#define FICR_BASE	0x10000000UL
#define UICR_BASE	0x10001000UL
#define GPIO_BASE	0x50000000UL

/* MPU */
#define REG_MPU_PERR0		((MPU_BASE + 0x528) / 4)
#define REG_MPU_RLENR0		((MPU_BASE + 0x52c) / 4)
#define REG_MPU_PROTENSET0	((MPU_BASE + 0x600) / 4)
#define REG_MPU_PROTENSET1	((MPU_BASE + 0x604) / 4)
#define REG_MPU_DISDBG		((MPU_BASE + 0x608) / 4)

/* GPIO */
#define REG_GPIO_OUT    ((GPIO_BASE + 0x504) / 4)
#define REG_GPIO_OUTSET ((GPIO_BASE + 0x508) / 4)
#define REG_GPIO_OUTCLR ((GPIO_BASE + 0x50c) / 4)
#define REG_GPIO_IN     ((GPIO_BASE + 0x510) / 4)
#define REG_GPIO_DIR    ((GPIO_BASE + 0x514) / 4)
#define REG_GPIO_DIRSET ((GPIO_BASE + 0x518) / 4)
#define REG_GPIO_DIRCLR ((GPIO_BASE + 0x51c) / 4)
#define REG_PIN_CNF(i)  ((GPIO_BASE + 0x700 + (i) * 4) / 4)

#define PIN_CNF_DIR_IN	(0 << 0)
#define PIN_CNF_DIR_OUT (1 << 0)
#define PIN_CNF_IN_CON	(0 << 1)
#define PIN_CNF_IN_DIS	(1 << 1)
#define PIN_CNF_NOPULL	(0 << 2)
#define PIN_CNF_PULLDN	(1 << 2)
#define PIN_CNF_PULLUP	(2 << 2)
#define PIN_CNF_S0S1	(0 << 8)
#define PIN_CNF_H0S1	(1 << 8)
#define PIN_CNF_S0H1	(2 << 8)
#define PIN_CNF_H0H1	(3 << 8)
#define PIN_CNF_D0S1	(4 << 8)
#define PIN_CNF_D0H1	(5 << 8)
#define PIN_CNF_S0D1	(6 << 8)
#define PIN_CNF_H0D1	(7 << 8)
#define PIN_CNF_SNS_DIS (0 << 16)
#define PIN_CNF_SNS_HI	(1 << 16)
#define PIN_CNF_SNS_LO	(2 << 16)

#endif /* __NRF51822_HW_H__ */
