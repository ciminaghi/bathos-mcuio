
//#include <stdlib.h>
#include <bathos/errno.h>
#include <bathos/interrupt.h>

#include <bathos/bathos.h>
#include <bathos/event.h>
#include <bathos/init.h>
#include <bathos/types.h>
#include <bathos/jiffies.h>
#include <bathos/sys_timer.h>
#include <linux/list.h>

#define MAX_SCHEDULED_TICKS 32

declare_event(hw_timer_tick);

static struct list_head scheduled_ticks;
static struct list_head free_ticks;

struct scheduled_tick {
	unsigned long when;
	void *data;
	struct list_head list;
};

/* Avoid malloc */
static struct scheduled_tick ticks[MAX_SCHEDULED_TICKS];

static int system_timer_init(void)
{
	int i;
	printf("%s\n", __func__);
	INIT_LIST_HEAD(&scheduled_ticks);
	INIT_LIST_HEAD(&free_ticks);
	for (i = 0; i < MAX_SCHEDULED_TICKS; i++)
		list_add_tail(&ticks[i].list, &free_ticks);
	return 0;
}

rom_initcall(system_timer_init);

static struct scheduled_tick *alloc_tick(void)
{
	struct scheduled_tick *out = NULL;
	if (list_empty(&free_ticks))
		return out;
	out = list_entry(free_ticks.next, struct scheduled_tick, list);
	list_del_init(&out->list);
	return out;
}

static void free_tick(struct scheduled_tick *t)
{
	list_move(&t->list, &free_ticks);
}

int sys_timer_enqueue_tick(unsigned long evt_jiffies, void *data)
{
	struct scheduled_tick *t, *ptr;
	int flags;
	t = alloc_tick();
	if (!t)
		return -ENOMEM;
	t->when = jiffies + evt_jiffies;
	t->data = data;
	interrupt_disable(flags);
	if (list_empty(&scheduled_ticks)) {
		list_add(&t->list, &scheduled_ticks);
		interrupt_restore(flags);
		return 0;
	}
	list_for_each_entry(ptr, &scheduled_ticks, list) {
		if (time_before(t->when, ptr->when)) {
			ptr->when -= t->when;
			list_add_tail(&t->list, &ptr->list);
			interrupt_restore(flags);
			return 0;
		}
		t->when -= ptr->when;
	}
	list_add_tail(&t->list, &ptr->list);
	interrupt_restore(flags);
	return 0;
}

int sys_timer_get_next_tick(unsigned long *j, void **data)
{
	struct scheduled_tick *t;
	if (list_empty(&scheduled_ticks))
		return -1;
	t = list_entry(scheduled_ticks.next, struct scheduled_tick, list);
	*j = t->when;
	*data = t->data;
	return 0;
}

void on_hw_timer_tick(struct event_handler_data *d)
{
	struct scheduled_tick *next, *tmp;
	list_for_each_entry_safe(next, tmp, &scheduled_ticks, list) {
		if (time_after(next->when, jiffies))
			break;
		trigger_event(&event_name(sys_timer_tick), next->data,
			      EVT_PRIO_MAX);
		if (!list_is_last(&next->list, &scheduled_ticks)) {
			struct scheduled_tick *t;
			t = list_entry(next->list.next,
				       struct scheduled_tick, list);
			t->when += jiffies;
		}
		free_tick(next);
	}
}

declare_event_handler(hw_timer_tick, NULL, on_hw_timer_tick, NULL);
