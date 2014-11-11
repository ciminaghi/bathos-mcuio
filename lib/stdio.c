#include <bathos/stdio.h>
#include <bathos/pipe.h>

struct bathos_pipe *bathos_stdout;
struct bathos_pipe *bathos_stdin;

/*
 * Dummy console (used when no console is selected and console_putc()
 * is not available
 */
#ifdef CONFIG_CONSOLE_NULL
void console_putc(int c)
{
}
#endif

void __attribute__((weak)) putc(int c)
{
	if (c == '\n')
		putc('\r');
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
