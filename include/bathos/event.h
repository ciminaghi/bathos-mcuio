
#ifndef __EVENT_H__
#define __EVENT_H__

#include <linux/list.h>

/* Definitions of event priorities (8 levels by default) */
#ifndef EVT_PRIO_MIN
#define EVT_PRIO_MIN 0
#endif
#ifndef EVT_PRIO_MAX
#define EVT_PRIO_MAX 7
#endif
#define NEVT_PRIOS   (EVT_PRIO_MAX - EVT_PRIO_MIN + 1)

struct event;
struct event_handler_data;

/*
 * @init: initializes event handler. Returns pointer to private data struct
 *        to be passed on to handle
 * @handle: actually handles the event
 * @exit: invoked when no more events can be generated
 */
struct event_handler_ops {
	/* Initializes event handler. Returns pointer to priva */
	int (*init)(struct event_handler_data *);
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
	struct event *evt;
	struct event_handler_ops *ops;
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
 * @list: this is used to link this instance to the list of pending events
 * @data: data related to this instance
 */
struct pending_event {
	struct event *event;
	struct list_head list;
	void *data;
};


extern int events_init(void);

/*
 * Trigger a generic event
 */
extern int trigger_event(struct event *, void *data, int evt_prio);
/*
 * Invoke this from main loop
 */
extern void handle_events(void);

extern struct event events_start[], events_end[];

#define cat(a,b) a##b
#define xcat(a,b) cat(a,b)
#define str(a) #a
#define xstr(a) str(a)
#define event_name(n) xcat(evt_,n)
#define event_handlers_start(n) xcat(event_name(n),_handlers_start)
#define event_handlers_end(n) xcat(event_name(n),_handlers_end)
#define event_handler_ops_struct(n) xcat(event_name(n),_handler_ops)
#define event_handler_struct(n) xcat(event_name(n), _handler_data)
#define event_handler_section_name(n) xcat(xcat(.evt_,n), _handlers))

#define declare_event(n)						\
	extern struct event_handler_data event_handlers_start(n)[],	\
		event_handlers_end(n)[];				\
	struct event event_name(n) __attribute__((section(".events"))) = { \
		.handlers_start = event_handlers_start(n),		\
		.handlers_end = event_handlers_end(n),			\
	}

#define declare_extern_event(n)			\
	extern struct event_handler_data event_handlers_start(n)[],	\
	    event_handlers_end(n)[];					\
	    extern struct event event_name(n);

#define __declare_event_handler(n, i, h, e, p)				\
	static struct event_handler_ops event_handler_ops_struct(n) = {	\
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

#endif /* __EVENT_H__ */
