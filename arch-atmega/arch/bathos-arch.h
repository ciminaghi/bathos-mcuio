#ifndef __BATHOS_ARCH_H__
#define __BATHOS_ARCH_H__

#include <stddef.h>
#include <bathos/string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define ARCH_NEEDS_INTERRUPTS_FOR_JIFFIES 1

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

static inline int __copy_byte(uint8_t *dst, const uint8_t *src)
{
	*dst = pgm_read_byte_near(src);
	return 0;
}

static inline int __copy_word(uint16_t *dst, const uint16_t *src)
{
	*dst = pgm_read_word_near(src);
	return 0;
}

static inline int __copy_dword(uint32_t *dst, const uint32_t *src)
{
	*dst = pgm_read_dword_near(src);
	return 0;
}

static inline void *memcpy_p(void *dst, const void *src, int size)
{
	return memcpy_P(dst, src, size);
}

#endif /* __BATHOS_ARCH_H__ */
