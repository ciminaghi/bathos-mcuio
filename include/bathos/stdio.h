#ifndef __BATHOS_STDIO_H__
#define __BATHOS_STDIO_H__
#include <stdarg.h>

/* These 4 are actually pp_printf and friends */
extern int printf(const char *fmt, ...)
        __attribute__((format(printf,1,2)));

extern int sprintf(char *s, const char *fmt, ...)
        __attribute__((format(printf,2,3)));

extern int vprintf(const char *fmt, va_list args);

extern int vsprintf(char *buf, const char *, va_list)
        __attribute__ ((format (printf, 2, 0)));

/* Puts is not actually "standard", as it doesn't add the trailing newline */
extern void putc(int c);
extern int puts(const char *s);

#endif /* __BATHOS_STDIO_H__ */
