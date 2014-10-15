/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/errno.h>
#include <bathos/string.h>

#include <bathos/init.h>
#include <bathos/event.h>
#include <bathos/interrupt.h>
#include <bathos/allocator.h>
#include <bathos/circ_buf.h>

#define PENDING_EVENTS_POOL_SIZE 32

/*
 * Pending events are stored in a circular buffer, no list of pending events,
 * saves memory.
 */
static int pe_head, pe_tail, pe_buffer_nevts;

#if !defined CONFIG_EVENTS_USE_ALLOCATOR
static struct pending_event pe_buffer[PENDING_EVENTS_POOL_SIZE];
#else
#define PENDING_EVENTS_START_POOL_SIZE 32
static struct pending_event *pe_buffer;
#endif

#if !defined CONFIG_EVENTS_USE_ALLOCATOR
static int __init_pending_events(void)
{
	pe_buffer_nevts = PENDING_EVENTS_POOL_SIZE;
	return 0;
}
#else
static int __init_pending_events(void)
{
	pe_buffer_nevts = PENDING_EVENTS_START_POOL_SIZE;
	pe_buffer = bathos_buddy_alloc_buffer(pe_buffer_nevts *
					      sizeof(struct pending_event));
	return !pe_buffer ? -ENOMEM : 0;
}
#endif

#if defined CONFIG_ARCH_ATMEGA

static inline struct event *__get_evt(struct event *dst,
				      const struct event *src)
{
	memcpy_P(dst, src, sizeof(*dst));
	return dst;
}

static inline struct event_handler_data *
__get_evt_handler_data(struct event_handler_data *dst,
		       const struct event_handler_data *src)
{
	memcpy_P(dst, src, sizeof(*dst));
	return dst;
}

static inline struct event_handler_ops *
__get_evt_handler_ops(struct event_handler_ops *dst,
		      const struct event_handler_data *d)
{
	memcpy_P(dst, d->ops, sizeof(*dst));
	return dst;
}

#else

static inline struct event *__get_evt(struct event *dst,
				      const struct event *src)
{
	*dst = *src;
	return dst;
}

static inline struct event_handler_data *
__get_evt_handler_data(struct event_handler_data *ehd,
		       const struct event_handler_data *src)
{
	*dst = *src;
	return dst;
}

static inline struct event_handler_ops *
__get_evt_handler_ops(struct event_handler_ops *dst,
		      const struct event_handler_data *d)
{
	*dst = *d->ops;
	return dst;
}

#endif

int events_init(void)
{
	const struct event *__e;
	for (__e = events_start; __e < events_end; __e++) {
		struct event_handler_data *d, *__d;
		static struct event evt;
		struct event *e;

		e = __get_evt(&evt, __e);
		for (__d = e->handlers_start; __d != e->handlers_end; __d++) {
			static struct event_handler_data ehd;
			static struct event_handler_ops eho;

			d = __get_evt_handler_data(&ehd, __d);
			d->ops = __get_evt_handler_ops(&eho, d);
			/* FIX THIS !!!! */
			d->evt = __e;
			if (d->ops->init)
				/* FIXME: do something in case of error */
				(void)d->ops->init(d);
		}
	}
	return __init_pending_events();
}

int trigger_event(const struct event *e, void *data, int evt_prio)
{
	struct pending_event *pe;
	int h;
	unsigned long flags;

	interrupt_disable(flags);
	if (!CIRC_SPACE(pe_head, pe_tail, pe_buffer_nevts)) {
		interrupt_restore(flags);
		return -ENOMEM;
	}
	h = pe_head;
	pe_head = (pe_head + 1) & (pe_buffer_nevts - 1);
	interrupt_restore(flags);
	pe = &pe_buffer[h];
	pe->data = data;
	pe->event = e;
	return 0;
}

void handle_events(void)
{
	struct event *e;
	int i, n = pending_events();

	for (i = 0; i < n; i++) {
		struct pending_event *pe = &pe_buffer[pe_tail];
		static struct event evt;
		struct event_handler_data *__d, *d;
		static struct event_handler_data ehd;
		static struct event_handler_ops eho;

		e = __get_evt(&evt, pe->event);
		for (__d = e->handlers_start;
		     __d != e->handlers_end; __d++) {
			d = __get_evt_handler_data(&ehd, __d);
			d->ops = __get_evt_handler_ops(&eho,
						       d);
			d->data = pe->data;
			if (d->ops->handle)
				d->ops->handle(d);
		}
		pe_tail = (pe_tail + 1) & (pe_buffer_nevts - 1);
	}
}

int pending_events(void)
{
	return CIRC_CNT(pe_head, pe_tail, pe_buffer_nevts);
}
