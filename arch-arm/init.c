#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/init.h>
#include <bathos/pipe.h>
#include <bathos/shell.h>
#include <generated/autoconf.h>
#include <arch/hw.h>

int stdio_init(void)
{
	return 0;
}
rom_initcall(stdio_init);

int bathos_setup(void)
{
	/* Call low level machine init (clocks, plls, ...) */
	mach_ll_init();
	return 0;
}
