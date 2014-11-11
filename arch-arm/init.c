#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/init.h>
#include <bathos/pipe.h>
#include <bathos/shell.h>
#include <bathos/debug_bitbang.h>
#include <generated/autoconf.h>
#include <arch/hw.h>
#include <bathos/gpio.h>

#ifndef CONFIG_STDOUT
#define CONFIG_STDOUT "null"
#endif
#ifndef CONFIG_STDIN
#define CONFIG_STDIN "null"
#endif

void stdio_init(void)
{
	bathos_stdout = pipe_open(CONFIG_STDOUT, BATHOS_MODE_OUTPUT, NULL);
	bathos_stdin = pipe_open(CONFIG_STDIN, BATHOS_MODE_INPUT, NULL);
}
core_initcall(stdio_init);

#if defined CONFIG_EARLY_CONSOLE
int bathos_setup(void)
{
	console_early_init();
	console_putc('.');
	do_initcalls();
}
#endif

#ifdef CONFIG_DEBUG_BITBANG
struct bathos_dev __bitbang_dev __attribute__((section(".bathos_devices"))) = {
	.name = "debug-bitbang",
	.ops = &bitbang_dev_ops,
};
#endif
