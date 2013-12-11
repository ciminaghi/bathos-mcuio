
#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/sys_timer.h>
#include <arch/hw.h>

declare_event(sys_timer_tick);

static int pts_init(struct event_handler_data *d)
{
	unsigned long now = jiffies;
	struct bathos_task *p;
	for (p = __task_begin; p < __task_end; p++) {
		printf("Task: %s\n", p->name);
		if (p->init) p->init(p->arg);
	}
	for (p = __task_begin; p < __task_end; p++)
		p->release += now + 2;
	return sys_timer_enqueue_tick(HZ/100, NULL);
	return 0;
}

static int pts_handle(struct event_handler_data *d)
{
	struct bathos_task *p;
	struct bathos_task *t;

	for (t = p = __task_begin; p < __task_end; p++)
		if (p->release < t->release)
			t = p;
	if (time_before(jiffies, t->release))
		goto end;
	t->arg = t->job(t->arg);
	t->release += t->period;
end:
	return sys_timer_enqueue_tick(HZ/100, NULL);
	return 0;
}

declare_event_handler(sys_timer_tick, pts_init, pts_handle, NULL);
