#include <bathos/stdio.h>

int puts(const char *s)
{
	while (*s)
		putc (*s++);
	return 0;
}
