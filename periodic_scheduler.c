
#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/init.h>
#include <bathos/sys_timer.h>
#include <arch/hw.h>

declare_event(sys_timer_tick);

static struct bathos_task *__find_next_task(void)
{
	struct bathos_task *p;
	struct bathos_task *t;
	for (t = p = __task_begin; p < __task_end; p++)
		if (p->release < t->release)
			t = p;
	return t;
}

static int pts_init(void)
{
	unsigned long now = jiffies;
	struct bathos_task *p, *next;
	for (p = __task_begin; p < __task_end; p++) {
		printf("Task: %s\n", p->name);
		if (p->init) p->init(p->arg);
	}
	for (p = __task_begin; p < __task_end; p++)
		p->release += now + 2;
	next = __find_next_task();
	return sys_timer_enqueue_tick(next->release - now, NULL);
}

rom_initcall(pts_init);

static void pts_handle(struct event_handler_data *d)
{
	struct bathos_task *next;
	do {
		next = __find_next_task();
		if (time_before(jiffies, next->release))
			break;
		next->arg = next->job(next->arg);
		next->release += next->period;
	} while(1);
	sys_timer_enqueue_tick(next->release - jiffies, NULL);
}

declare_event_handler(sys_timer_tick, NULL, pts_handle, NULL);
