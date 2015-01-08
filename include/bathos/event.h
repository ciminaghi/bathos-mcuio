/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include <linux/list.h>
#include <generated/autoconf.h>
/* cat, xcat, str, xstr */
#include <bathos/bathos.h>
/* PROGMEM */
#include <arch/bathos-arch.h>
/* ENOSYS */
#include <bathos/errno.h>

struct event;
struct event_handler_data;

/*
 * @init: initializes event handler. Returns pointer to private data struct
 *        to be passed on to handle
 * @handle_ll: interrupt events only: low level handler, invoked from interrupt
 *        context.
 * @handle: actually handles the event
 * @exit: invoked when no more events can be generated
 */
struct event_handler_ops {
	/* Initializes event handler. Returns pointer to priva */
	int (*init)(struct event_handler_data *);
#ifdef CONFIG_INTERRUPT_EVENTS
	void (*handle_ll)(struct event_handler_data *);
#endif
	/* Chiamata per gestire l'evento */
	void (*handle)(struct event_handler_data *);
	/* Chiamata alla fine (eventiale) */
	void (*exit)(struct event_handler_data *);
};

/*
 * @evt: pointer to relevant event
 * @ops: pointer to event handler ops
 * @data: event specific data
 * @priv: pointer to handler private data structure
 */
struct event_handler_data {
	const struct event * PROGMEM evt;
	const struct event_handler_ops * PROGMEM ops;
	void *data;
	void *priv;
};

/*
 * This structure represents an event
 *
 * @handlers_start: pointer to the first handler for this event
 * @handlers_end: pointer to the last handler for this event
 */
struct event {
	struct event_handler_data *handlers_start;
	struct event_handler_data *handlers_end;
};

/*
 * This structure represents a pending instance of an event
 *
 * @event: pointer to relevant event structure
 * @data: data related to this instance
 */
struct pending_event {
	const struct event *event;
	void *data;
};


extern int events_init(void);
extern int pending_events(void);

/*
 * Trigger a generic event
 */
extern int trigger_event(const struct event *, void *data);
/* As above, but handlers are executed immediately */
extern int trigger_event_immediate(const struct event *, void *data);

#ifdef CONFIG_INTERRUPT_EVENTS
/* Trigger an interrupt event */
extern int trigger_interrupt_event(int n);
#else
static inline int trigger_interrupt_event(int n)
{
	return -ENOSYS;
}
#endif /* CONFIG_INTERRUPT_EVENTS */

/*
 * Invoke this from main loop
 */
extern void handle_events(void);

extern const struct event PROGMEM events_start[], events_end[],
	interrupt_events_start[], interrupt_events_end[];

#define event_name(n) xcat(evt_,n)
#define event_handlers_start(n) xcat(event_name(n),_handlers_start)
#define event_handlers_end(n) xcat(event_name(n),_handlers_end)
#define event_handler_ops_struct(n) xcat(event_name(n),_handler_ops)
#define event_handler_struct(n) xcat(event_name(n), _handler_data)
#define event_handler_section_name(n) xcat(xcat(.evt_,n), _handlers))

#define declare_event(n)						\
	extern struct event_handler_data event_handlers_start(n)[],	\
		event_handlers_end(n)[];				\
				const struct event event_name(n)	\
				__attribute__((section(".events"))) = { \
		.handlers_start = event_handlers_start(n),		\
		.handlers_end = event_handlers_end(n),			\
	}

#define declare_extern_event(n)			\
	extern struct event_handler_data event_handlers_start(n)[],	\
	    event_handlers_end(n)[];					\
	    extern const struct event __attribute__((section(".events"))) event_name(n);

#define __declare_event_handler(n, i, h, e, p)				\
    static const struct event_handler_ops PROGMEM			\
    event_handler_ops_struct(n) = {					\
	    .init = i,							\
	    .handle = h,						\
	    .exit = e,							\
    };									\
    struct event_handler_data xcat(MODULE_NAME, event_handler_struct(n)) \
    __attribute__((section(xstr(event_handler_section_name(n)))) = {	\
	    .ops = &event_handler_ops_struct(n),			\
	    .priv = p,							\
    };

#define declare_event_handler(n, i, h, e)	\
    __declare_event_handler(n, i, h, e, NULL)

#define declare_event_handler_with_priv(n, i, h, e, p) \
    __declare_event_handler(n, i, h, e, p)

#define bathos_int_handler_name(intno) xcat(bathos_int_handler_,intno)
#define bathos_ll_int_handler_name(intno) xcat(bathos_ll_int_handler_,intno)
#define bathos_int_handler_priv(intno) xcat(bathos_int_data_,intno)

#endif /* __EVENT_H__ */
