#include <arch/bathos-arch.h>

extern char *strcpy(char * d, char *s);
extern int strlen(char *s);
extern int strnlen(char *s, int count);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, int n);
extern void *memcpy(void *d, const void *s, int count);

#ifndef ARCH_IS_HARVARD
#define memcpy_p memcpy
#endif

extern void *memset(void *d, int c, int count);
