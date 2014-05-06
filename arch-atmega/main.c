/*
 * Main for the avr. Things we want to do with interrupts enabled before
 * calling the regular bathos main function
 */
#include <arch/hw.h>
#include <generated/autoconf.h>
#include <bathos/delay.h>


int avr_bathos_main(void)
{
	udelay_init();
#if defined CONFIG_USB_UART
	/* Without this, USB doesn't seem to work well */
	mdelay(200);
#endif
	return bathos_main();
}
