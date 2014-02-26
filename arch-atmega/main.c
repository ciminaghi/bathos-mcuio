/*
 * Main for the avr. Things we want to do with interrupts enabled before
 * calling the regular bathos main function
 */
#include <arch/hw.h>
#include <generated/autoconf.h>
#include <util/delay.h>


int avr_bathos_main(void)
{
	return bathos_main();
}
