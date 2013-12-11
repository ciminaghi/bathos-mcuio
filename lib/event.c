
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <bathos/event.h>

static struct list_head pending_events_1[NEVT_PRIOS];
static struct list_head pending_events_2[NEVT_PRIOS];
static struct list_head *pending_events = pending_events_1;
static struct list_head *next_pending_events = pending_events_2;

int events_init(void)
{
	int i;
	struct event *e;
	for (i = 0; i < NEVT_PRIOS; i++) {
		INIT_LIST_HEAD(&pending_events_1[i]);
		INIT_LIST_HEAD(&pending_events_2[i]);
	}
	for (e = events_start; e < events_end; e++) {
		struct event_handler_data *d;
		e->list.next = e->list.prev = &e->list;
		for (d = e->handlers_start; d != e->handlers_end; d++) {
			d->evt = e;
			if (d->ops->init)
				/* FIXME: do something in case of error */
				(void)d->ops->init(d);
		}
	}
	return 0;
}

int trigger_event(struct event *e, void *data, int evt_prio)
{
	if (evt_prio < EVT_PRIO_MIN || evt_prio > EVT_PRIO_MAX)
		return -EINVAL;
	if (!list_empty(&e->list)) {
		e->overrun = 1;
		return 0;
	}
	e->data = data;
	list_add_tail(&e->list, &next_pending_events[evt_prio]);
	return 0;
}

void handle_events(void)
{
	struct event *e, *tmp;
	struct event_handler_data *d;
	struct list_head *tmpl;
	int i;

	/* Swap events list */
	tmpl = next_pending_events;
	next_pending_events = pending_events;
	pending_events = tmpl;

	for (i = EVT_PRIO_MAX; i >= EVT_PRIO_MIN; i--) {
		list_for_each_entry_safe(e, tmp, &pending_events[i], list) {
			list_del_init(&e->list);
			for (d = e->handlers_start; d != e->handlers_end; d++) {
				d->data = e->data;
				if (d->ops->handle)
					d->ops->handle(d);
			};
		}
	}
}

