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
	unsigned long now = jiffies;
	unsigned long next;
	void *data;
	if (sys_timer_get_next_tick(&next, &data) < 0)
		/* No next tick */
		return;
	if (time_before(now, next))
		return;
	trigger_event(&event_name(sys_timer_tick), data, EVT_PRIO_MAX);
}

int bathos_main(void)
{
	printf("Hello, Bathos is speaking (built on %s)\n", __DATE__);
	while(1) {
		handle_events();
		idle();
	}
	return 0;
}
