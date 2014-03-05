#ifndef __BATHOS_ARCH_H__
#define __BATHOS_ARCH_H__

#include <stddef.h>
#include <bathos/string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

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
	return ptr >= (const void *)&__end_text;
}

static inline int is_pgm_pointer(const void *ptr)
{
	return ptr < (const void *)&__end_text;
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

/*
 * Copy byte from data to data
 */
static int __copy_byte_dd(uint8_t *dst, const uint8_t *src)
{
	*dst = *src;
	return 0;
}

/*
 * Copy byte from program to data
 */
static int __copy_byte_pd(uint8_t *dst, const uint8_t *src)
{
	*dst = pgm_read_byte_near(src);
	return 0;
}

/*
 * Copy word from data to data
 */
static int __copy_word_dd(uint16_t *dst, const uint16_t *src)
{
	*dst = *src;
	return 0;
}

/*
 * Copy word from program to data
 */
static int __copy_word_pd(uint16_t *dst, const uint16_t *src)
{
	*dst = pgm_read_word_near(src);
	return 0;
}

/*
 * Copy dword from data to data
 */
static int __copy_dword_dd(uint32_t *dst, const uint32_t *src)
{
	*dst = *src;
	return 0;
}

/*
 * Copy dword from program to data
 */
static int __copy_dword_pd(uint32_t *dst, const uint32_t *src)
{
	*dst = pgm_read_dword_near(src);
	return 0;
}

static inline int __copy_byte(uint8_t *dst, const uint8_t *src)
{
	if (!is_pgm_pointer(src))
		__copy_byte_pd(dst, src);
	return __copy_byte_dd(dst, src);
}

static inline int __copy_word(uint16_t *dst, const uint16_t *src)
{
	if (is_data_pointer(src) < 0)
		return __copy_word_dd(dst, src);
	return __copy_word_pd(dst, src);
}

static inline int __copy_dword(uint32_t *dst, const uint32_t *src)
{
	if (is_data_pointer(src) < 0)
		return __copy_dword_dd(dst, src);
	return __copy_dword_pd(dst, src);
}

static inline void *memcpy_p(void *dst, const void *src, int size)
{
	if (check_data_pointer(dst) < 0)
		return NULL;
	if (is_data_pointer(src))
		return memcpy(dst, src, size);
	return memcpy_P(dst, src, size);
}

#endif /* __BATHOS_ARCH_H__ */
