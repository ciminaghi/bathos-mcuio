/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

/*
 * This task opens a configurable pipe, reads from it and writes
 * data to the same pipe
 */
#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>
#include <bathos/jiffies.h>
#include <bathos/stdio.h>
#include <generated/autoconf.h>

#ifndef CONFIG_LOOPBACK_PIPE_NAME
#define CONFIG_LOOPBACK_PIPE_NAME "usb-serial-0"
#endif

declare_event(loopback);

#ifdef DEBUG
static void dump_buf(const char *buf, int len)
{
	int i, j;

	if (len < 0) {
		printf("error reading buffer\n");
		return;
	}
	printf("read: ");
	for (i = 0; i < len; ) {
		for (j = 0 ; j < 4 && i < len; j++, i++)
			printf("0x%02x\t", buf[i]);
		printf("\n");
	}
}
#else
static inline void dump_buf(const char *buf, int len)
{
}
#endif


static int loopback_pipe_init(void *arg)
{
	struct bathos_pipe *p;

	p = pipe_open(CONFIG_LOOPBACK_PIPE_NAME,
		      BATHOS_MODE_INPUT | BATHOS_MODE_OUTPUT,
		      NULL);
	if (!p) {
		printf("%s: error opening pipe (%d)\n", __func__, bathos_errno);
		return -bathos_errno;
	}
	pipe_remap_input_ready_event(p, &evt_loopback);

	return 0;
}

static void *dummy(void *arg)
{
	return arg;
}

static struct bathos_task __task t_mcuio = {
	.name = "loopback-pipe", .period = 60 * HZ,
	.job = dummy, .arg = NULL,
	.init = loopback_pipe_init,
	.release = 3,
};

static void data_ready_handle(struct event_handler_data *ed)
{
	int stat, sent, tot;
	char buf[80];
	struct bathos_pipe *p = ed->data;

	stat = pipe_read(p, buf, sizeof(buf));
	dump_buf(buf, stat);
	if (stat <= 0) {
		printf("loopback-pipe: pipe rd err (%d)\n", bathos_errno);
		if (!stat) {
			/*
			  The other end closed the pipe (or an error occurred),
			  reopen it
			*/
			pipe_close(p);
			p = pipe_open(CONFIG_LOOPBACK_PIPE_NAME,
				      BATHOS_MODE_INPUT|
				      BATHOS_MODE_OUTPUT,
				      NULL);
			pipe_remap_input_ready_event(p, &evt_loopback);
		}
		return;
	}
	for (sent = 0, tot = stat; sent < tot; ) {
		stat = pipe_write(p, &buf[sent], tot - sent);
		if (stat < 0) {
			printf("loopback-pipe: pipe wr err (%d)\n",
			       bathos_errno);
			continue;
		}
		sent += stat;
	}
}

declare_event_handler(loopback, NULL, data_ready_handle, NULL);
