/*
 * Main function: a welcome message and a simple scheduler
 * Alessandro Rubini, 2009 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/idle.h>
#include <bathos/errno.h>
#include <bathos/sys_timer.h>
#include <arch/hw.h>

int bathos_errno;

/*
 * Dummy default idle
 */
void __attribute__((weak)) idle(void)
{
	trigger_event(&event_name(hw_timer_tick), NULL, EVT_PRIO_MAX);
}

int bathos_main(void)
{
#if !CONFIG_CONSOLE_NULL
	printf("Hello, Bathos is speaking (%s built on " __DATE__ ")\n",
	       BATHOS_GIT);
#endif
	while(1) {
		while(pending_events())
			handle_events();
		idle();
	}
	return 0;
}
