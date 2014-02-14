#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/init.h>
#include <arch/hw.h>

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

int stdio_init(void)
{
	bathos_stdout = pipe_open("usb-uart", BATHOS_MODE_OUTPUT, NULL);
	bathos_stdin = pipe_open("usb-uart", BATHOS_MODE_INPUT, NULL);
	return 0;
}
rom_initcall(stdio_init);

int __attribute__((section(".text.vectors)"))) bathos_setup(void)
{
	CPU_PRESCALE(0);
	/* Turn red led on */
	timer_init();

	do_initcalls();

	/* Interrupts are enabled by the calling assembly code */
	return 0;
}
