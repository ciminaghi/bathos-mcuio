/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 * Modified for KL25Z Davide Ciminaghi <ciminaghi@gnudd.com> 2014
 */
#ifndef __KL25Z_HW_H__
#define __KL25Z_HW_H__

#define CPU_FREQ		(12 * 1000 * 1000)
#define HZ			100


#define REG_SOPT1		(0x40047000 / 4) /* write */
#define REG_SOPT1CFG		(0x40047004 / 4)
#define REG_SOPT4		(0x4004800C / 4)
#define REG_SOPT5		(0x40048010 / 4)
#define REG_SOPT7		(0x40048018 / 4)
#define REG_SDID		(0x40048024 / 4)
#define REG_SCGC4		(0x40048034 / 4)
#define REG_SCGC5		(0x40048038 / 4)
#define REG_SCGC6		(0x4004803C / 4)
#define REG_SCGC7		(0x40048040 / 4)
#define REG_CLKDIV1		(0x40048044 / 4)
#define REG_FCFG1		(0x4004804C / 4)
#define REG_FCFG2		(0x40048050 / 4)
#define REG_UIDMH		(0x40048058 / 4)
#define REG_UIDML		(0x4004805C / 4)
#define REG_UIDL		(0x40048060 / 4)
#define REG_COPC		(0x40048100 / 4)
#define REG_SRVCOP		(0x40048104 / 4)

#define REG_MCG_C1		(0x40064000 / 1)
#define REG_MCG_C2		(0x40064001 / 1)
#define REG_MCG_C3		(0x40064002 / 1)
#define REG_MCG_C4		(0x40064003 / 1)
#define REG_MCG_C5		(0x40064004 / 1)
#define REG_MCG_C6		(0x40064005 / 1)
#define REG_MCG_S		(0x40064006 / 1)
#define MCG_S_IREFST_MASK	  0x10u
#define MCG_S_LOCK0_MASK          0x40u

#define REG_MCG_SC		(0x40064008 / 1)
#define REG_MCG_ATCVH		(0x4006400a / 1)
#define REG_MCG_ATCVL		(0x4006400b / 1)
#define REG_MCG_C7		(0x4006400c / 1)
#define REG_MCG_C8		(0x4006400d / 1)
#define REG_MCG_C9		(0x4006400e / 1)
#define REG_MCG_C10		(0x4006400f / 1)


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
