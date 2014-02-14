
#include <bathos/errno.h>
#include <bathos/string.h>

#include <bathos/init.h>
#include <bathos/event.h>
#include <bathos/interrupt.h>

#define PENDING_EVENTS_POOL_SIZE 32

static struct list_head pending_events_1[NEVT_PRIOS];
static struct list_head pending_events_2[NEVT_PRIOS];
static struct list_head *pending_events = pending_events_1;
static struct list_head *next_pending_events = pending_events_2;
static struct list_head free_pending_events;

static struct pending_event pe_array[PENDING_EVENTS_POOL_SIZE];

struct pending_event *alloc_pending_event(void)
{
	struct pending_event *out;
	int flags;
	interrupt_disable(flags);
	if (list_empty(&free_pending_events)) {
		interrupt_restore(flags);
		return NULL;
	}
	out = list_entry(free_pending_events.next, struct pending_event, list);
	list_del_init(&out->list);
	interrupt_restore(flags);
	return out;
}

int events_init(void)
{
	int i;
	struct event *e;
	INIT_LIST_HEAD(&free_pending_events);
	for (i = 0; i < ARRAY_SIZE(pe_array); i++)
		list_add_tail(&pe_array[i].list, &free_pending_events);
	for (i = 0; i < NEVT_PRIOS; i++) {
		INIT_LIST_HEAD(&pending_events_1[i]);
		INIT_LIST_HEAD(&pending_events_2[i]);
	}
	for (e = events_start; e < events_end; e++) {
		struct event_handler_data *d;
		for (d = e->handlers_start; d != e->handlers_end; d++) {
			d->evt = e;
			if (d->ops->init)
				/* FIXME: do something in case of error */
				(void)d->ops->init(d);
		}
	}
	return 0;
}

rom_initcall(events_init);

int trigger_event(struct event *e, void *data, int evt_prio)
{
	struct pending_event *pe;
	int flags;
	if (evt_prio < EVT_PRIO_MIN || evt_prio > EVT_PRIO_MAX)
		return -EINVAL;
	pe = alloc_pending_event();
	if (!pe)
		return -ENOMEM;
	pe->data = data;
	pe->event = e;
	interrupt_disable(flags);
	list_add_tail(&pe->list, &next_pending_events[evt_prio]);
	interrupt_restore(flags);
	return 0;
}

void handle_events(void)
{
	struct pending_event *pe, *tmp;
	struct event *e;
	struct event_handler_data *d;
	struct list_head *tmpl;
	int i, empty;

	do {
		uint8_t flags;
		interrupt_disable(flags);
		/* Swap events list */
		tmpl = next_pending_events;
		next_pending_events = pending_events;
		pending_events = tmpl;
		interrupt_restore(flags);

		for (i = EVT_PRIO_MAX, empty = NEVT_PRIOS;
		     i >= EVT_PRIO_MIN; i--) {
			list_for_each_entry_safe(pe, tmp,
						 &pending_events[i], list) {
				e = pe->event;
				empty--;
				for (d = e->handlers_start;
				     d != e->handlers_end; d++) {
					d->data = pe->data;
					if (d->ops->handle)
						d->ops->handle(d);
				};
				interrupt_disable(flags);
				list_move(&pe->list, &free_pending_events);
				interrupt_restore(flags);
			}
		}
		
	} while(empty < NEVT_PRIOS);
}

