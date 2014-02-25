#include <bathos/stdio.h>

struct bathos_pipe *bathos_stdout;
struct bathos_pipe *bathos_stdin;

void __attribute__((weak)) putc(int c)
{
#ifdef CONFIG_COPY_STDOUT_TO_CONSOLE
	console_putc(c);
#endif
	if (!bathos_stdout)
		return;
	pipe_write(bathos_stdout, (char *)&c, 1);
}

int puts(const char *s)
{
	while (*s)
		putc (*s++);
	return 0;
}
