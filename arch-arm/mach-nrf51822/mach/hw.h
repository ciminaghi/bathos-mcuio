/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 * Modified for KL25Z Davide Ciminaghi <ciminaghi@gnudd.com> 2014
 */
#ifndef __NRF51822_HW_H__
#define __NRF51822_HW_H__

#define BUS_FREQ		CONFIG_HFCLK
#define CPU_FREQ		CONFIG_HFCLK

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

#endif /* __NRF51822_HW_H__ */
