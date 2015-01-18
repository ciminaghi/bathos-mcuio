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
#include <bathos/bitops.h>
#include <bathos/ffs.h>

#ifdef CONFIG_ARCH_ATMEGA
#define PENDING_EVENTS_POOL_SIZE 64
#elif defined (CONFIG_ARCH_ARM)
#define PENDING_EVENTS_POOL_SIZE 256
#endif

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

#ifdef CONFIG_INTERRUPT_EVENTS
#define PENDING_FLAGS_SZ BITS_TO_LONGS(CONFIG_NR_INTERRUPTS)

static unsigned long ie_pending_flags[PENDING_FLAGS_SZ];
#endif /* CONFIG_INTERRUPT_EVENTS */


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
	*ehd = *src;
	return ehd;
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
	/* Interrupt events are __not__ initialized */
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
core_initcall(events_init);

int trigger_event(const struct event *e, void *data)
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

#ifdef CONFIG_INTERRUPT_EVENTS
static inline int __handle_ll_evt(struct event_handler_data *d)
{
	d->ops->handle_ll(d);
	return 1;
}
#else
static inline int __handle_ll_evt(struct event_handler_data *d)
{
	return 0;
}
#endif /* CONFIG_INTERRUPT_EVENTS */

static void __handle_event(const struct event *__e, void *data, int ll)
{
	struct event_handler_data *__d, *d;
	struct event_handler_data ehd;
	struct event_handler_ops eho;
	struct event evt;
	struct event *e;

	e = __get_evt(&evt, __e);
	for (__d = e->handlers_start;
	     __d != e->handlers_end; __d++) {
		d = __get_evt_handler_data(&ehd, __d);
		d->ops = __get_evt_handler_ops(&eho,
					       d);
		d->data = data;
		if (ll && __handle_ll_evt(d))
			continue;
		if (d->ops->handle)
			d->ops->handle(d);
	}
}

int trigger_event_immediate(const struct event *e, void *data)
{
	__handle_event(e, data, 0);
	return 0;
}

#ifdef CONFIG_INTERRUPT_EVENTS
int trigger_interrupt_event(int evno)
{
	const struct event *e;

	if (evno >= CONFIG_NR_INTERRUPTS)
		return -EINVAL;

	/* Start low level handler */
	e = &interrupt_events_start[evno];
	__handle_event(e, NULL, 1);

	/* And get ready for executing high level handler */
	set_bit(evno, ie_pending_flags);
	return 0;
}

static void handle_interrupt_events(void)
{
	int w;
	static unsigned long tmp[PENDING_FLAGS_SZ];

	memcpy(tmp, ie_pending_flags, sizeof(tmp));

	for (w = 0; w < CONFIG_NR_INTERRUPTS; w+=BITS_PER_LONG) {
		int k = BITS_TO_LONGS(w);
		unsigned long *l = &tmp;
		const struct event *e = &interrupt_events_start[k];

		do {
			int i = ffs(*l);
			if (!i)
				break;
			i--;
			__handle_event(e + i, NULL, 0);
			*l &= ~BIT_MASK(i);
		} while(1);
	}
}

static int pending_interrupt_events(void)
{
	int w;

	for (w = 0; w < BITS_TO_LONGS(CONFIG_NR_INTERRUPTS); w++)
		if (ie_pending_flags[w])
			return 1;
	return 0;
}
#else /* !CONFIG_INTERRUPT_EVENTS */
static void handle_interrupt_events(void)
{
}

static int pending_interrupt_events(void)
{
	return 0;
}
#endif /* CONFIG_INTERRUPT_EVENTS */

static inline int pending_regular_events(void)
{
	return CIRC_CNT(pe_head, pe_tail, pe_buffer_nevts);
}

static void handle_regular_events(void)
{
	struct event *e;
	int i, n = pending_regular_events();

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

void handle_events(void)
{
	handle_interrupt_events();
	handle_regular_events();
}

int pending_events(void)
{
	if (pending_interrupt_events())
		return 1;
	return pending_regular_events();
}
