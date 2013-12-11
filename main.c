/*
 * Main function: a welcome message and a simple scheduler
 * Alessandro Rubini, 2009 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <arch/hw.h>

int bathos_main(void)
{
	printf("Hello, Bathos is speaking (built on %s)\n", __DATE__);
	events_init();
	while(1)
		handle_events();
	return 0;
}
