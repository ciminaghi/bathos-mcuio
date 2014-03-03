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

#define time_after(a,b)		\
    ((int)(b) - (int)(a) < 0)

#define time_after_eq(a,b)	\
    (((int)(a) - (int)(b)) >= 0)

#endif /* __BATHOS_ARCH_H__ */
