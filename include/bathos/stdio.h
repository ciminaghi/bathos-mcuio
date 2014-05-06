#ifndef __BATHOS_STDIO_H__
#define __BATHOS_STDIO_H__
#include <stdarg.h>

#ifndef NULL
#define NULL 0
#endif

#include <generated/autoconf.h>

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

struct bathos_pipe;

extern struct bathos_pipe *bathos_stdout;
extern struct bathos_pipe *bathos_stdin;

extern void console_putc(int c);

#if defined CONFIG_EARLY_CONSOLE
extern int console_early_init(void);
#else
static inline int console_early_init(void)
{
	return 0;
}
#endif

#endif /* __BATHOS_STDIO_H__ */
