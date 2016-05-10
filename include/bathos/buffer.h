#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <bathos/event.h>
#include <linux/list.h>

/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* Generic buffer [for async operations] */
/*
 * The following events can be emitted by a buffer:
 *
 * release: this is emitted when the buffer's users counter drops to zero
 */

struct bathos_bqueue;

struct bathos_sglist_el {
	void				*data;
	int				len;
	struct list_head		list;
};

struct bathos_bdescr {
	/*
	 * Either we have data != NULL && sglist empty or data == NULL
	 * and sglist !empty
	 */
	void				*data;
	unsigned int			data_size;
	struct list_head		sglist;
	int				error;
	int				users;
	const struct event		*release_event;
	struct bathos_bqueue		*queue;
	struct list_head		list;
};

static inline int bdescr_error(struct bathos_bdescr *b)
{
	return b->error;
}

static inline void bdescr_set_error(struct bathos_bdescr *b, int e)
{
	b->error = e;
}

static inline void bdescr_get(struct bathos_bdescr *b)
{
	unsigned long flags;

	interrupt_disable(flags);
	b->users++;
	interrupt_restore(flags);
}

static inline void bdescr_put(struct bathos_bdescr *b)
{
	unsigned long flags;

	interrupt_disable(flags);
	if (--b->users) {
		interrupt_restore(flags);
		return;
	}
	interrupt_restore(flags);
	if (b->release_event)
		trigger_event(b->release_event, b);
}

static inline struct bathos_bdescr *
bdescr_shallow_copy(struct bathos_bdescr *src)
{
	bdescr_get(src);
	return src;
}

extern struct bathos_bdescr *bdescr_copy(const struct bathos_bdescr *src);


static inline void *bdescr_data(struct bathos_bdescr *b,
				unsigned int *data_size)
{
	*data_size = b->data_size;
	return b->data;
}

static inline void bdescr_remap_release_event(struct bathos_bdescr *b,
					      const struct event *e)
{
	b->release_event = e;
}


#endif /* __BUFFER_H__ */
