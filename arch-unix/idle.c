#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/idle.h>
#include <bathos/sys_timer.h>
#include <arch/hw.h>

#include <errno.h>
#include <sys/select.h>

/*
 * idle implementation for unix, select on stdin with a timeout for detecting
 * events
 */
void idle(void)
{
	int stat;
	struct timeval next_timeout;

	unsigned long now, next;
	void *data;

	now = jiffies;
	if (sys_timer_get_next_tick(&next, &data) < 0)
		/* No next tick */
		return;
	if (time_before(next, now))
		goto end;
	next_timeout.tv_usec = ((1000000 * (next - now)) / HZ) % 1000000;
	next_timeout.tv_sec = ((1000000 * (next - now)) / HZ) / 1000000;
	stat = select(0, NULL, NULL, NULL, &next_timeout);
	if (stat < 0 && errno == EINTR)
		return ;
	if (stat < 0) {
		sleep(1);
		return;
	}
end:
	trigger_event(&event_name(sys_timer_tick), NULL, EVT_PRIO_MAX);
}
