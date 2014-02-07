#include <stdint.h>

#define THOS_QUARTZ		(16UL * 1000 * 1000)
#define HZ			(THOS_QUARTZ / 256 / 256) /* 244 (+.140625) */

#include <avr/io.h>

extern void timer_init(void);
extern void pll_init(void);

#ifdef MCU_atmega8
#include <arch/hw_atmega8.h>
#elif MCU_atmega32u4
#include <arch/hw_atmega32u4.h>
#else
#error "Invalid atmel MCU defined"
#endif
