/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

/*
 * radio-bridge-master: master side of radio bridge.
 * Typical use case:
 *
 *   input pipe                         output pipe
 *  +------+      +--------------+      +-------+
 *  | uart |<---->| radio-bridge |<---->| radio |<---> ... AIR
 *  +------+      +--------------+      +-------+
 *                        ^
 *                        | local pipe
 *                        V
 *
 * Periodically sends "beacon" messages to slave (0 bytes long messages)
 * The slave can respond to beacon messages with its spontaneous messages.
 * Messages coming from input pipe are copied to output pipe in place of
 * beacon messages (every time a beacon period expires, either a beacon message
 * or a packet coming from input pipe is written to the output pipe).
 *
 * Platform data include a pointer to a filter function returning !0 in case
 * incoming data has to be directed to local pipe.
 *
 * Output events: pipe_input_ready (on local pipe)
 */

#include <bathos/bathos.h>
#include <bathos/errno.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/allocator.h>
#include <bathos/radio-bridge-master.h>

struct rb_master_private_data {
	struct bathos_pipe *input_pipe;
	struct bathos_pipe *output_pipe;
	uint8_t packet_in_ipipe_buffer;
	uint8_t filter_out_packet;
	uint8_t input_bytes;
	char *ipipe_buffer;
	char *opipe_buffer;
};

declare_event(rb_ipipe_input_ready);
declare_event(rb_opipe_input_ready);

/*
  Beacon function, if packet is in input buffer send it, otherwise send
  an empty packet (beacon)
 */
void rb_master_do_beacon(struct bathos_dev *dev)
{
	struct rb_master_private_data *priv = dev->priv;
	const struct rb_master_platform_data *plat = dev->platform_data;
	int l = 0;
	char *buf = NULL;

	if (!priv)
		return;
	if (priv->packet_in_ipipe_buffer && !priv->filter_out_packet) {
		l = plat->packet_size;
		buf = priv->ipipe_buffer;
		priv->packet_in_ipipe_buffer = 0;
	}

	(void)pipe_write(priv->output_pipe, buf, l);
}


/* Pipe operations */

static int rb_master_open(struct bathos_pipe *pipe)
{
	const struct rb_master_platform_data *plat = pipe->dev->platform_data;
	struct rb_master_private_data *priv;
	int ret = 0;

	if (!plat)
		return -EINVAL;
	priv = bathos_alloc_buffer(sizeof(*priv));
	if (!priv)
		return -ENOMEM;

	priv->input_pipe = pipe_open(plat->input_dev,
				     BATHOS_MODE_INPUT_OUTPUT,
				     priv);
	if (!priv->input_pipe) {
		ret = -ENODEV;
		goto error0;
	}
	/* Remap event so that only our handler is called */
	pipe_remap_input_ready_event(priv->input_pipe,
				     &evt_rb_ipipe_input_ready);

	priv->output_pipe = pipe_open(plat->output_dev,
				      BATHOS_MODE_INPUT_OUTPUT,
				      priv);
	if (!priv->output_pipe) {
		ret = -ENODEV;
		goto error1;
	}
	priv->ipipe_buffer = bathos_alloc_buffer(plat->packet_size);
	if (!priv->ipipe_buffer) {
		ret = -ENOMEM;
		goto error2;
	}
	pipe_remap_input_ready_event(priv->output_pipe,
				     &evt_rb_opipe_input_ready);
	priv->opipe_buffer = bathos_alloc_buffer(plat->packet_size);
	if (!priv->opipe_buffer) {
		ret = -ENOMEM;
		goto error3;
	}
	pipe->dev->priv = priv;
	return 0;

error3:
	bathos_free_buffer(priv->ipipe_buffer, plat->packet_size);
error2:
	pipe_close(priv->output_pipe);
error1:
	pipe_close(priv->input_pipe);
error0:
	bathos_free_buffer(priv, sizeof(*priv));
	return ret;
}

static int rb_master_read(struct bathos_pipe *pipe, char *buf, int len)
{
	struct rb_master_private_data *priv = pipe->dev->priv;
	const struct rb_master_platform_data *plat = pipe->dev->platform_data;
	int l = min(len, plat->packet_size);

	if (!(priv->packet_in_ipipe_buffer && priv->filter_out_packet))
		return -EAGAIN;
	if (l <= 0)
		return -EINVAL;
	priv->filter_out_packet = 0;
	priv->packet_in_ipipe_buffer = 0;
	memcpy(buf, priv, l);
	return l;
}

static int rb_master_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	struct rb_master_private_data *priv = pipe->dev->priv;

	return pipe_write(priv->input_pipe, buf, len);
}

static int rb_master_close(struct bathos_pipe *pipe)
{
	struct rb_master_private_data *priv = pipe->dev->priv;
	const struct rb_master_platform_data *plat = pipe->dev->platform_data;

	pipe_close(priv->input_pipe);
	pipe_close(priv->output_pipe);
	bathos_free_buffer(priv->ipipe_buffer, plat->packet_size);
	bathos_free_buffer(priv, sizeof(*priv));
	return 0;
}

static void input_pipe_ready_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe = ed->data;
	struct rb_master_private_data *priv = pipe->dev->priv;
	const struct rb_master_platform_data *plat = pipe->dev->platform_data;
	int stat, l = plat->packet_size - priv->input_bytes;

	if (priv->packet_in_ipipe_buffer && !priv->filter_out_packet)
		pr_debug("%s: input overrun\n", __func__);
	stat = pipe_read(pipe, priv->ipipe_buffer, l);
	if (stat < 0) {
		printf("%s: error in pipe_read\n", __func__);
		return;
	}
	priv->input_bytes += l;
	if (priv->input_bytes < plat->packet_size)
		return;
	if (plat->filter(priv->ipipe_buffer, plat->packet_size,
			 plat->filter_data))
		priv->filter_out_packet = 1;
	priv->packet_in_ipipe_buffer = 1;
	priv->input_bytes = 0;
}

declare_event_handler(rb_ipipe_input_ready, NULL, input_pipe_ready_handle,
		      NULL);

static void output_pipe_ready_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe = ed->data;
	struct rb_master_private_data *priv = pipe->dev->priv;
	const struct rb_master_platform_data *plat = pipe->dev->platform_data;
	int stat;

	/*
	 * Output pipe is the radio, which always receives whole packets,
	 * so a single read per packet should suffice
	*/
	stat = pipe_read(pipe, priv->opipe_buffer, sizeof(plat->packet_size));
	if (stat < 0) {
		printf("%s: error in pipe_read\n", __func__);
		return;
	}
	if (stat < plat->packet_size)
		printf("%s: WARNING: bad packet size %d\n", __func__, stat);
	stat = pipe_write(priv->input_pipe, priv->opipe_buffer, stat);
	if (stat < 0)
		printf("%s: error in pipe_write\n", __func__);
}

declare_event_handler(rb_opipe_input_ready, NULL, output_pipe_ready_handle,
		      NULL);

const struct bathos_dev_ops master_radio_dev_ops = {
	.open = rb_master_open,
	.read = rb_master_read,
	.write = rb_master_write,
	.close = rb_master_close,
};
