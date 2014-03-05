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

/* Check for data/program pointers */

extern const void *__end_text;

static inline int is_data_pointer(const void *ptr)
{
	return ptr >= __end_text;
}

static inline int is_pgm_pointer(const void *ptr)
{
	return ptr < __end_text;
}

static inline int __check_data_pointer(const void *ptr, const char *s)
{
	int ret = is_data_pointer(ptr) ? 0 : -1;
#ifdef CONFIG_VERBOSE_PTR_ERRORS
	printf("%s: invalid data ptr %p\n", s, ptr);
#endif
	return ret;
}

static inline int __check_pgm_pointer(const void *ptr, const char *s)
{
	int ret = is_pgm_pointer(ptr) ? 0 : -1;
#ifdef CONFIG_VERBOSE_PTR_ERRORS
	printf("%s: invalid data ptr %p\n", s, ptr);
#endif
	return ret;
}

#ifdef CONFIG_CHECK_PTR_ERRORS
#define check_data_pointer(ptr) __check_data_pointer(ptr, __func__)
#define check_pgm_pointer(ptr) __check_pgm_pointer(ptr, __func__)
#else
static inline int check_data_pointer(const void *ptr)
{
	return 0;
}

static inline int check_pgm_pointer(const void *ptr)
{
	return 0;
}
#endif

#endif /* __BATHOS_ARCH_H__ */
