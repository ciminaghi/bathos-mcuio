/*
 * Trivial string.c, mainly from U-Boot sources
 *
 * Copyright 2010, Alessandro Rubini <rubini@gnudd.com>
 * Copytight 2010, BTicino SpA www.bticino.it
 *
 * Released according to the GNU GPL version 2
 */
#include <bathos/string.h>

char *strcpy(char * d, char *s)
{
	char *res = d;

	while ((*d++ = *s++) != '\0')
		;
	return res;
}

int strlen(char *s)
{
	char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

int strnlen(char *s, int count)
{
	char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

void *memcpy(void *d, void *s, int count)
{
	unsigned long *dl = (unsigned long *)d, *sl = (unsigned long *)s;
	char *d8, *s8;

	/* while all data is aligned (common case), copy a word at a time */
	if ( (((unsigned long)d | (unsigned long)s) & (sizeof(*dl) - 1)) == 0) {
		while (count >= sizeof(*dl)) {
			*dl++ = *sl++;
			count -= sizeof(*dl);
		}
	}
	/* copy the reset one byte at a time */
	d8 = (char *)dl;
	s8 = (char *)sl;
	while (count--)
		*d8++ = *s8++;

	return d;
}

void * memset(void *s, int c, int count)
{
	unsigned long *sl = (unsigned long *) s;
	unsigned long cl = 0;
	char *s8;
	int i;

	/* do it one word at a time (32 bits or 64 bits) while possible */
	if ( ((unsigned long)s & (sizeof(*sl) - 1)) == 0) {
		for (i = 0; i < sizeof(*sl); i++) {
			cl <<= 8;
			cl |= c & 0xff;
		}
		while (count >= sizeof(*sl)) {
			*sl++ = cl;
			count -= sizeof(*sl);
		}
	}
	/* fill 8 bits at a time */
	s8 = (char *)sl;
	while (count--)
		*s8++ = c;

	return s;
}
