#ifndef __BATHOS_ARCH_H__
#define __BATHOS_ARCH_H__

#include <avr/interrupt.h>

#define interrupt_disable(flags) \
    do {			 \
        flags = SREG;		 \
	cli();			 \
    } while(0)

#define interrupt_restore(flags) \
    SREG = flags

#endif /* __BATHOS_ARCH_H__ */
