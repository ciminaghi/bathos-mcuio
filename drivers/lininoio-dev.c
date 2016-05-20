/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* [char] device over lininoio */

#include <bathos/bathos.h>
#include <bathos/errno.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/allocator.h>
#include <bathos/lininoio-internal.h>
#include <bathos/lininoio-dev.h>
#include <bathos/dev_ops.h>

struct lininoio_dev_priv {
	int connected;
	int ninstances;
	struct lininoio_channel_data *chan_data;
};

#if ARCH_IS_HARVARD

const struct lininoio_channel *
__get_chan(const struct lininoio_channel * PROGMEM chan,
	   struct lininoio_channel *_chan)
{
	/* copy pointer, not structure */
	memcpy_P(_chan, chan, sizeof(chan));
	return _chan;
}

#else

const struct lininoio_channel *
__get_chan(const struct lininoio_channel * PROGMEM chan,
	   struct lininoio_channel *_chan)
{
	return chan;
}

#endif

static int lininoio_dev_open(struct bathos_pipe *pipe)
{
	struct lininoio_dev_priv *priv;
	const struct lininoio_channel * PROGMEM chan = pipe->dev->platform_data;
	struct lininoio_channel __ch;
	const struct lininoio_channel *ch = __get_chan(chan, &__ch);

	if (!chan) {
		printf("%s: ERROR, no channel\n", __func__);
		return -ENODEV;
	}
	priv = pipe->dev->priv;
	if (priv)
		/* Not first open */
		return 0;

	priv = bathos_alloc_buffer(sizeof(*priv));
	if (!priv)
		return -ENOMEM;
	priv->ninstances = 1;
	priv->chan_data = ch->data;
	pipe->dev->priv = priv;
	return 0;
}

static int lininoio_dev_write(struct bathos_pipe *pipe,
			      const char *buf, int len)
{
	struct lininoio_dev_priv *priv = pipe->dev->priv;
	struct lininoio_channel_data *chan_data;
	struct bathos_bdescr *b;
	int l;

	if (len < 0) {
		printf("ERR: %s: invalid length %d\n", __func__, len);
		return -EINVAL;
	}
	if (!priv) {
		printf("ERR: %s: no private data\n", __func__);
		return -EINVAL;
	}
	chan_data = priv->chan_data;
	b = pipe_async_get_buf(chan_data->io_pipe);
	if (!b)
		return -EAGAIN;
	if (!b->data) {
		printf("ERR: %s: buffer data is NULL\n", __func__);
		return -EINVAL;
	}
	l = min(len, b->data_size);
	memcpy(b->data, buf, l);
	if (l < len)
		printf("WARN: %s: lost %u bytes\n", __func__,
		       len - b->data_size);
	return l;
}

static int lininoio_dev_close(struct bathos_pipe *pipe)
{
	struct lininoio_dev_priv *priv = pipe->dev->priv;

	if (priv->ninstances-- > 1)
		return 0;

	/* Closing last instance */
	bathos_free_buffer(priv, sizeof(*priv));
	return 0;
}

const struct bathos_dev_ops lininoio_dev_ops = {
	.open = lininoio_dev_open,
	.write = lininoio_dev_write,
	.close = lininoio_dev_close,
};


int lininoio_dev_input(const struct lininoio_channel *ch,
		       void *buf, uint16_t len)
{
	struct bathos_dev *dev = ch->data->priv;

	if (!dev) {
		printf("WARN: %s, dev is NULL\n", __func__);
		return 0;
	}
	return bathos_dev_push_chars(dev, buf, len);
}
