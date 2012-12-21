#include <stdarg.h>

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

extern volatile unsigned long jiffies;

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
