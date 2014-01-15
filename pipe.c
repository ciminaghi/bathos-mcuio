#include <bathos/bathos.h>
#include <bathos/errno.h>
#include <bathos/pipe.h>
#include <bathos/string.h>

extern struct bathos_dev bathos_devices_start[], bathos_devices_end[];

declare_event(input_pipe_opened);
declare_event(output_pipe_opened);
declare_event(input_pipe_closed);
declare_event(output_pipe_closed);
declare_event(pipe_input_ready);
declare_event(pipe_output_ready);
declare_event(pipe_really_closed);

static struct bathos_pipe pipes[MAX_BATHOS_PIPES];

static void __do_free_pipe(struct bathos_pipe *p)
{
	p->dev = NULL;
}

static struct bathos_pipe *__find_free_pipe(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(pipes); i++)
		if (!pipes[i].dev)
			return &pipes[i];
	return NULL;
}

struct bathos_dev * __attribute__((weak)) bathos_find_dev(struct bathos_pipe *p)
{
	struct bathos_dev *d;
	for (d = bathos_devices_start; d != bathos_devices_end; d++)
		if (!strcmp(p->n, d->name))
			return d;
	return NULL;
}

struct bathos_pipe *pipe_open(const char *n, int mode, void *data)
{
	struct bathos_pipe *out = NULL;
	struct event *e;
	int stat;
	struct bathos_dev *dev;
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
	if (dev->ops->open) {
		stat = dev->ops->open(out);
		if (stat) {
			__do_free_pipe(out);
			bathos_errno = stat;
			return NULL;
		}
	}
	e = mode == BATHOS_MODE_INPUT ? &evt_input_pipe_opened :
	    &evt_output_pipe_opened;
	printf("Event = %p\n", e);
	trigger_event(e, out, EVT_PRIO_MAX);
	return out;
}

int pipe_close(struct bathos_pipe *pipe)
{
	struct event *e;
	printf("%s %d, pipe = %p\n", __func__, __LINE__, pipe);
	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}
	if (pipe->dev->ops->close)
		pipe->dev->ops->close(pipe);
	e = pipe->mode == BATHOS_MODE_INPUT ? &evt_input_pipe_closed :
		&evt_output_pipe_closed;
	trigger_event(e, pipe, EVT_PRIO_MAX);
	trigger_event(&event_name(pipe_really_closed), pipe, EVT_PRIO_MAX - 1);
	return 0;
}

int pipe_read(struct bathos_pipe *pipe, char *buf, int len)
{
	int stat;
	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}
	if (!(pipe->mode & BATHOS_MODE_INPUT) || !pipe->dev->ops->read) {
		bathos_errno = EPERM;
		return -1;
	}
	stat = pipe->dev->ops->read(pipe, buf, len);
	bathos_errno = stat < 0 ? -stat : 0;
	return stat;
}

int pipe_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	int stat;
	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}
	if (!(pipe->mode & BATHOS_MODE_OUTPUT) || !pipe->dev->ops->write) {
		bathos_errno = EPERM;
		return -1;
	}
	stat = pipe->dev->ops->write(pipe, buf, len);
	bathos_errno = stat < 0 ? -stat : 0;
	return stat;
}

int pipe_ioctl(struct bathos_pipe *pipe, struct bathos_ioctl_data *data)
{
	int stat;
	if (!pipe->dev) {
		bathos_errno = EBADF;
		return -1;
	}
	if (!pipe->dev->ops->ioctl) {
		bathos_errno = EPERM;
		return -1;
	}
	stat = pipe->dev->ops->ioctl(pipe, data);
	bathos_errno = stat < 0 ? -stat : 0;
	return stat;
}

static void do_free_pipe(struct event_handler_data *data)
{
	struct bathos_pipe *p = data->data;
	printf("%s %d, pipe =  %p\n", __func__, __LINE__, p);
	__do_free_pipe(p);
}

declare_event_handler(pipe_really_closed, NULL, do_free_pipe, NULL);
