/*
 * Main header for Bathos
 *  Alessandro Rubini, 2009-2012 GNU GPL2 or later
 */
#ifndef __BATHOS_H__
#define __BATHOS_H__
#include <stdarg.h>
#include <arch/bathos-arch.h>

/* These 4 are actually pp_printf and friends */
extern int printf(const char *fmt, ...)
        __attribute__((format(printf,1,2)));

extern int sprintf(char *s, const char *fmt, ...)
        __attribute__((format(printf,2,3)));

extern int vprintf(const char *fmt, va_list args);

extern int vsprintf(char *buf, const char *, va_list)
        __attribute__ ((format (printf, 2, 0)));

/* Other misc bathos stuff */
extern int bathos_main(void);
extern int bathos_setup(void);

extern void putc(int c);
extern int puts(const char *s);

/*
 * The architecture may define __get_jiffies if the hardware doesn't
 * provide a single 32-bit increasing counter at some address
 */
#ifdef __get_jiffies
#  define jiffies __get_jiffies()
#else
  extern volatile unsigned long jiffies;
#endif

/* And finally the task definition */
struct bathos_task {
	char *name;
	void *(*job)(void *);
	int (*init)(void *);
	void *arg;
	unsigned long period;
	unsigned long release;
};

#define __task __attribute__((section(".task"),__used__))

extern struct bathos_task __task_begin[], __task_end[];

#endif /* __BATHOS_H__ */
