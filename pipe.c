/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/bathos.h>
#include <bathos/errno.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/allocator.h>

extern struct bathos_dev bathos_devices_start[], bathos_devices_end[];

declare_event(input_pipe_opened);
declare_event(output_pipe_opened);
declare_event(input_pipe_closed);
declare_event(output_pipe_closed);
declare_event(pipe_input_ready);
declare_event(pipe_output_ready);
declare_event(pipe_really_closed);

#ifdef CONFIG_ARCH_ATMEGA
static inline struct bathos_dev_ops *__get_dev_ops(struct bathos_dev_ops *ops,
						   struct bathos_dev *dev)
{
	memcpy_P(ops, dev->ops, sizeof(*ops));
	return ops;
}
#else
static inline struct bathos_dev_ops *__get_dev_ops(struct bathos_dev_ops *ops,
						   struct bathos_dev *dev)
{
	*ops = *dev->ops;
	return ops;
}
#endif

static void __do_free_pipe(struct bathos_pipe *p)
{
	p->dev = NULL;
}

static struct bathos_pipe *__find_free_pipe(void)
{
	return bathos_alloc_buffer(sizeof(struct bathos_pipe));
}

struct bathos_dev * __attribute__((weak)) bathos_find_dev(struct bathos_pipe *p)
{
	struct bathos_dev *d;
	for (d = bathos_devices_start; d < bathos_devices_end; d++)
		if (!strcmp(p->n, d->name))
			return d;
	return NULL;
}

void pipe_dev_trigger_event(struct bathos_dev *dev, const struct event *evt,
			    int prio)
{
	struct bathos_pipe *p;
	if (!dev->pipes.next)
		/* List not even initialized */
		return;
	list_for_each_entry(p, &dev->pipes, list) {
		if (trigger_event(evt, p, prio) < 0) {
			pr_debug("WARN: missing pipe evt\n");
		}
	}
}

struct bathos_pipe *pipe_open(const char *n, int mode, void *data)
{
	struct bathos_pipe *out = NULL;
	const struct event *e;
	int stat;
	struct bathos_dev *dev;
	struct bathos_dev_ops __ops, *ops;

	if (!mode) {
		bathos_errno = EINVAL;
		return out;
	}
	out = __find_free_pipe();
	if (!out) {
		bathos_errno = ENFILE;
		return out;
	}
	out->n = n;
	out->data = data;
	out->mode = mode;
	dev  = bathos_find_dev(out);
	if (!dev) {
		__do_free_pipe(out);
		bathos_errno = ENXIO;
		return NULL;
	}
	out->dev = dev;
	if (!dev->pipes.next)
		INIT_LIST_HEAD(&dev->pipes);
	ops = __get_dev_ops(&__ops, dev);
	if (ops->open) {
		stat = ops->open(out);
		if (stat) {
			__do_free_pipe(out);
			bathos_errno = stat;
			return NULL;
		}
	}
	list_add(&out->list, &dev->pipes);
	e = mode == BATHOS_MODE_INPUT ? &evt_input_pipe_opened :
	    &evt_output_pipe_opened;
	trigger_event(e, out, EVT_PRIO_MAX);
	return out;
}

int pipe_close(struct bathos_pipe *pipe)
{
	const struct event *e;
	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}
	e = pipe->mode == BATHOS_MODE_INPUT ? &evt_input_pipe_closed :
		&evt_output_pipe_closed;
	trigger_event(e, pipe, EVT_PRIO_MAX);
	trigger_event(&event_name(pipe_really_closed), pipe, EVT_PRIO_MAX - 1);
	return 0;
}

int pipe_read(struct bathos_pipe *pipe, char *buf, int len)
{
	int stat;
	struct bathos_dev_ops __ops, *ops;

	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}

	ops = __get_dev_ops(&__ops, pipe->dev);
	if (!(pipe->mode & BATHOS_MODE_INPUT) || !ops->read) {
		bathos_errno = EPERM;
		return -1;
	}
	stat = ops->read(pipe, buf, len);
	bathos_errno = stat < 0 ? -stat : 0;
	return stat;
}

int pipe_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	int stat;
	struct bathos_dev_ops __ops, *ops;

	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}
	ops = __get_dev_ops(&__ops, pipe->dev);
	if (!(pipe->mode & BATHOS_MODE_OUTPUT) || !ops->write) {
		bathos_errno = EPERM;
		return -1;
	}
	stat = ops->write(pipe, buf, len);
	bathos_errno = stat < 0 ? -stat : 0;
	return stat;
}

int pipe_ioctl(struct bathos_pipe *pipe, struct bathos_ioctl_data *data)
{
	int stat;
	struct bathos_dev_ops __ops, *ops;

	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}
	ops = __get_dev_ops(&__ops, pipe->dev);
	if (!ops->ioctl) {
		bathos_errno = EPERM;
		return -1;
	}
	stat = ops->ioctl(pipe, data);
	bathos_errno = stat < 0 ? -stat : 0;
	return stat;
}

static void do_free_pipe(struct event_handler_data *data)
{
	struct bathos_pipe *p = data->data;
	list_del_init(&p->list);
	struct bathos_dev_ops __ops, *ops;

	/* Last pipe related to this device ? */
	if (list_empty(&p->dev->pipes)) {
		ops = __get_dev_ops(&__ops, p->dev);
		if (ops->close)
			ops->close(p);
	}
	__do_free_pipe(p);
}

declare_event_handler(pipe_really_closed, NULL, do_free_pipe, NULL);
