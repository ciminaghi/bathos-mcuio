/*
 * A set of tasks that print timely messages to the serial port
 * Alessandro Rubini, 2009 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <arch/hw.h>

static void *uart_out(void *arg)
{
	char *s = arg;
#ifdef CONFIG_STDOUT_UART
	puts(s);
#else
	while (*s)
		console_putc(*s);
#endif
	return arg;
}

static struct bathos_task __task t_quarter = {
	.name = "quarter", .period = HZ/4,
	.job = uart_out, .arg = "."
};

static struct bathos_task __task t_second = {
	.name = "second", .period = HZ,
	.job = uart_out, .arg = "S",
	.release = 1,
};

static struct bathos_task __task t_10second = {
	.name = "10second", .period = 10 * HZ,
	.job = uart_out, .arg = "\n",
	.release = 2,
};

static struct bathos_task __task t_minute = {
	.name = "minute", .period = 60 * HZ,
	.job = uart_out, .arg = "minute!\n",
	.release = 3,
};
