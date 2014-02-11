#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/init.h>
#include <bathos/jiffies.h>
#include <arch/hw.h>
#include <avr/sleep.h>


/*
 * idle implementation for arch-atmega
 */
void idle(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}
