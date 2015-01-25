/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

/*
 * radio-bridge-slave: slave side of radio bridge.
 * Typical use case:
 *
 *   input pipe
 *  +-------+      +--------------+
 *  | radio |<---->| radio-bridge |
 *  +-------+      +--------------+
 *                         ^
 *                         | local pipe
 *                         V
 *
 * Waits for beacon messages or actual packets from master side.
 *
 * Output events: pipe_input_ready (on local pipe)
 */

#include <bathos/bathos.h>
#include <bathos/errno.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/allocator.h>
#include <bathos/radio-bridge-slave.h>

#define MAX_PACKETS_IN_OUTPUT_BUFFER 2

struct rb_slave_private_data {
	struct bathos_pipe *input_pipe;
	uint8_t packet_in_input_buffer;
	char *input_buffer;
	int input_bytes;
	uint8_t packets_in_output_buffer;
	char *output_buffer;
	int ninstances;
};

declare_event(rb_ipipe_input_ready);

/* Pipe operations */

static int rb_slave_open(struct bathos_pipe *pipe)
{
	const struct rb_slave_platform_data *plat = pipe->dev->platform_data;
	struct rb_slave_private_data *priv;
	int ret = 0;

	if (!plat)
		return -EINVAL;
	priv = pipe->dev->priv;
	if (priv)
		/* Not first open */
		return 0;
		
	priv = bathos_alloc_buffer(sizeof(*priv));
	if (!priv)
		return -ENOMEM;
	memset(priv, 0, sizeof(*priv));

	priv->input_pipe = pipe_open(plat->input_dev,
				     BATHOS_MODE_INPUT_OUTPUT,
				     pipe);
	if (!priv->input_pipe) {
		ret = -ENODEV;
		goto error0;
	}
	/* Remap event so that only our handler is called */
	pipe_remap_input_ready_event(priv->input_pipe,
				     &evt_rb_ipipe_input_ready);

	priv->input_buffer = bathos_alloc_buffer(plat->packet_size);
	if (!priv->input_buffer) {
		ret = -ENOMEM;
		goto error1;
	}
	/* Reserve space for a couple of output packets */
	priv->output_buffer =
		bathos_alloc_buffer(MAX_PACKETS_IN_OUTPUT_BUFFER *
				    plat->packet_size);
	if (!priv->output_buffer) {
		ret = -ENOMEM;
		goto error2;
	}
	priv->ninstances = 1;
	pipe->dev->priv = priv;
	return 0;

error2:
	bathos_free_buffer(priv->input_buffer, plat->packet_size);
error1:
	pipe_close(priv->input_pipe);
error0:
	bathos_free_buffer(priv, sizeof(*priv));
	return ret;
}

static int rb_slave_read(struct bathos_pipe *pipe, char *buf, int len)
{
	struct rb_slave_private_data *priv = pipe->dev->priv;
	const struct rb_slave_platform_data *plat = pipe->dev->platform_data;
	int l = min(len, plat->packet_size);

	if (!priv->packet_in_input_buffer)
		return -EAGAIN;
	if (l <= 0)
		return -EINVAL;
	priv->packet_in_input_buffer--;
	memcpy(buf, priv->input_buffer, l);
	return l;
}

static int rb_slave_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	struct rb_slave_private_data *priv = pipe->dev->priv;
	const struct rb_slave_platform_data *plat = pipe->dev->platform_data;
	int l = min(len, plat->packet_size);
	int offs = priv->packets_in_output_buffer * plat->packet_size;

	if (priv->packets_in_output_buffer >= MAX_PACKETS_IN_OUTPUT_BUFFER)
		return -EAGAIN;
	memcpy(&priv->output_buffer[offs], buf, l);
	priv->packets_in_output_buffer++;
	return l;
}

static int rb_slave_close(struct bathos_pipe *pipe)
{
	struct rb_slave_private_data *priv = pipe->dev->priv;
	const struct rb_slave_platform_data *plat = pipe->dev->platform_data;

	if (priv->ninstances-- > 1)
		return 0;

	/* Closing last instance */
	pipe_close(priv->input_pipe);
	bathos_free_buffer(priv->input_buffer, plat->packet_size);
	bathos_free_buffer(priv->output_buffer, plat->packet_size *
			   MAX_PACKETS_IN_OUTPUT_BUFFER);
	bathos_free_buffer(priv, sizeof(*priv));
	pipe->data = NULL;
	return 0;
}

/* An empty packet is a beacon coming from the other side of the bridge */
static void beacon_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe = ed->data;
	struct bathos_pipe *my_pipe = pipe->data;
	struct rb_slave_private_data *priv = my_pipe->dev->priv;
	const struct rb_slave_platform_data *plat = my_pipe->dev->platform_data;
	int stat;

	if (!priv->packets_in_output_buffer)
		return;

	while (priv->packets_in_output_buffer) {
		priv->packets_in_output_buffer--;
		stat = pipe_write(pipe, priv->output_buffer,
				  plat->packet_size);
		if (stat < 0)
			printf("WARN: %s, error in pipe_write()\n",
			       __func__);
	}
}

declare_event_handler(radio_empty_packet_received, NULL,
		      beacon_handle, NULL);

static void input_pipe_ready_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe = ed->data;
	struct bathos_pipe *my_pipe = pipe->data;
	struct rb_slave_private_data *priv = my_pipe->dev->priv;
	const struct rb_slave_platform_data *plat = my_pipe->dev->platform_data;
	int stat;

	if (priv->packet_in_input_buffer)
		pr_debug("%s: input overrun\n", __func__);
	stat = pipe_read(pipe, priv->input_buffer, plat->packet_size);
	if (stat < 0) {
		printf("%s: error in pipe_read\n", __func__);
		return;
	}
	priv->input_bytes += stat;
	if (priv->input_bytes < plat->packet_size)
		return;
	priv->packet_in_input_buffer++;
	priv->input_bytes = 0;
	pipe_dev_trigger_event(my_pipe->dev, &evt_pipe_input_ready);
}

declare_event_handler(rb_ipipe_input_ready, NULL, input_pipe_ready_handle,
		      NULL);

const struct bathos_dev_ops slave_radio_dev_ops = {
	.open = rb_slave_open,
	.read = rb_slave_read,
	.write = rb_slave_write,
	.close = rb_slave_close,
};
