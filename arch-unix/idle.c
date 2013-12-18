#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/idle.h>
#include <bathos/sys_timer.h>
#include <bathos/pipe.h>
#include <arch/hw.h>
#include <arch/pipe.h>

#include <bathos/errno.h>
#include <sys/select.h>

static fd_set rd_fdset;
static fd_set wr_fdset;
static int nfds;

/*
 * idle implementation for unix, select on stdin with a timeout for detecting
 * events
 */
void idle(void)
{
	int stat;
	struct timeval next_timeout;
	fd_set __rdfds = rd_fdset;

	unsigned long now, next;
	void *data;

	now = jiffies;
	if (sys_timer_get_next_tick(&next, &data) < 0)
		/* No next tick */
		return;
	if (time_before(next, now))
		goto end;
	next_timeout.tv_usec = ((1000000 * (next - now)) / HZ) % 1000000;
	next_timeout.tv_sec = ((1000000 * (next - now)) / HZ) / 1000000;
	stat = select(nfds, &__rdfds, NULL, NULL, &next_timeout);
	if (stat < 0 && bathos_errno == EINTR)
		return ;
	if (stat < 0) {
		sleep(1);
		return;
	}
	if (stat > 0) {
		/* TRIGGER INPUT EVENTS HERE */
	}
end:
	trigger_event(&event_name(sys_timer_tick), NULL, EVT_PRIO_MAX);
}

static int pipe_opened_init(struct event_handler_data *data)
{
	FD_ZERO(&rd_fdset);
	FD_ZERO(&wr_fdset);
	return 0;
}

static void pipe_opened_handle(struct event_handler_data *data)
{
	struct bathos_pipe *p = data->data;
	struct bathos_dev *dev;
	struct arch_unix_pipe_data *adata;
	if (!p)
		return;
	dev = p->dev;
	if (!dev)
		return;
	adata = dev->priv;
	if (!adata)
		return;
	if (p->mode & BATHOS_MODE_OUTPUT)
		FD_SET(adata->fd, &wr_fdset);
	if (p->mode & BATHOS_MODE_INPUT)
		FD_SET(adata->fd, &rd_fdset);
	nfds = max(nfds, adata->fd + 1);
}

declare_event_handler(input_pipe_opened, pipe_opened_init,
		      pipe_opened_handle, NULL);
declare_event_handler(output_pipe_opened, pipe_opened_init,
		      pipe_opened_handle, NULL);
