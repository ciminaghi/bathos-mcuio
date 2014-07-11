/*
 * Copyright 2011 Dog Hunter SA
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * GNU GPLv2 or later
 */
/* Common operations for bathos devices */
#include <bathos/dev_ops.h>
#include <bathos/stdio.h> /* NULL */
#include <bathos/errno.h>
#include <bathos/pipe.h>
#include <bathos/allocator.h>
#include <bathos/circ_buf.h>

#define UART_DEFAULT_BUF_SIZE 16

struct bathos_dev_data {
	enum bathos_dev_mode mode;
	union {
		struct {
			int head;
			int tail;
			int size;
			char *buf;
			char *ext_buf;
		} cb;
		struct {
			int bhead;
			int btail;
			int bnum;
			int bsize;
			int curr_index;
			char **bufs;
		} pk;
	} d;
	/* rx high watermark */
	int rx_hwm;
	const struct bathos_ll_dev_ops * PROGMEM ops;
	void *ll_priv;
};

#if defined ARCH_IS_HARVARD
static inline struct bathos_ll_dev_ops *__get_ops(struct bathos_dev_data *data,
						  struct bathos_ll_dev_ops *ops)
{
	memcpy_p(ops, data->ops, sizeof(*ops));
	return ops;
}

#else
static inline struct bathos_ll_dev_ops *__get_ops(struct bathos_dev_data *data,
						  struct bathos_ll_dev_ops *ops)
{
	return data->ops;
}
#endif

struct bathos_dev_data *
bathos_dev_init(const struct bathos_ll_dev_ops * PROGMEM ops, void *priv)
{
	struct bathos_dev_data *out;

	if (!ops || !ops->rx_enable || !ops->tx_enable || !ops->rx_disable ||
	    !ops->rx_disable)
		return NULL;
	out = bathos_alloc_buffer(sizeof(*out));
	if (!out)
		return out;

	memset(out, 0, sizeof(*out));
	out->mode = CIRC_BUF;
	out->ops = ops;
	out->ll_priv = priv;
	return out;
}

int bathos_dev_cb_push_chars(struct bathos_dev *dev, const char *buf, int len)
{
	int l, s, out = 0;
	struct bathos_dev_data *data = dev->priv;

	s = CIRC_SPACE(data->d.cb.head, data->d.cb.tail, data->d.cb.size);
	if (!s)
		return -ENOMEM;
	/* Copy to the end of the buffer */
	l = min(len, CIRC_SPACE_TO_END(data->d.cb.head, data->d.cb.tail,
				       data->d.cb.size));
	memcpy(&data->d.cb.buf[data->d.cb.head], buf, l);
	data->d.cb.head = (data->d.cb.head + l) & (data->d.cb.size - 1);
	len -= l;
	if (!l)
		goto end;
	out = l;
	/* And finally copy the rest */
	l = min(len, s);
	memcpy(&data->d.cb.buf[data->d.cb.head], buf, l);
	data->d.cb.head = (data->d.cb.head + l) & (data->d.cb.size - 1);
end:
	if (CIRC_CNT(data->d.cb.head, data->d.cb.tail, data->d.cb.size) >
	    data->rx_hwm)
		pipe_dev_trigger_event(dev, &evt_pipe_input_ready,
				       EVT_PRIO_MAX);
	return out + l;
}

int bathos_dev_pk_push_chars(struct bathos_dev *dev, const char *buf, int len)
{
	struct bathos_dev_data *data = dev->priv;
	int s = data->d.pk.bsize - data->d.pk.curr_index, l;
	char *dst;

	l = min(s, len);
	if (!l)
		return len ? -ENOMEM : l;
	dst = &(data->d.pk.bufs[data->d.pk.bhead])[data->d.pk.curr_index];
	memcpy(dst, buf, l);
	data->d.pk.curr_index += l;
	if (data->d.pk.curr_index < data->rx_hwm)
		/* Below watermark, just return */
		return l;
	pipe_dev_trigger_event(dev, &evt_pipe_input_ready, EVT_PRIO_MAX);
	if (data->d.pk.curr_index < (data->d.pk.bsize - 1))
		/* Still space in buffer, just return */
		return l;
	if (!CIRC_SPACE(data->d.pk.bhead, data->d.pk.btail, data->d.pk.bnum))
		return l;
	/* Still space in queue, take next input buffer and reset curr_index */
	data->d.pk.bhead = (data->d.pk.bhead + 1) & (data->d.pk.bnum - 1);
	data->d.pk.curr_index = 0;
	return l;
}


int bathos_dev_push_chars(struct bathos_dev *dev, const char *buf, int len)
{
	struct bathos_dev_data *data = dev->priv;

	switch (data->mode) {
	case CIRC_BUF:
		return bathos_dev_cb_push_chars(dev, buf, len);
	case PACKET:
		return bathos_dev_pk_push_chars(dev, buf, len);
	default:
		return -EINVAL;
	}
	/* NEVER REACHED */
	return -EINVAL;
}


static int __allocate_internal_buf(struct bathos_dev_data *data)
{
	data->d.cb.size = UART_DEFAULT_BUF_SIZE;
	data->d.cb.buf = bathos_alloc_buffer(data->d.cb.size);
	return data->d.cb.buf ? 0 : -ENOMEM;
}

int bathos_dev_open(struct bathos_pipe *pipe)
{
	struct bathos_dev_data *data = pipe->dev->priv;
	struct bathos_ll_dev_ops *ops, __ops;
	int stat;

	/* Mode is circ buf by default on open */
	data->mode = CIRC_BUF;
	if (!data->d.cb.buf) {
		stat = __allocate_internal_buf(data);
		if (stat < 0)
			return stat;
	}
	data->d.cb.head = data->d.cb.tail = 0;
	ops = __get_ops(data, &__ops);
	if (pipe->mode & BATHOS_MODE_INPUT) {
		stat = ops->rx_enable(data->ll_priv);
		if (stat)
			return stat;
	}
	if (pipe->mode & BATHOS_MODE_OUTPUT) {
		stat = ops->tx_enable(data->ll_priv);
		if (stat)
			return stat;
	}
	return 0;
}

int bathos_dev_read(struct bathos_pipe *pipe, char *buf, int len)
{
	int l;
	struct bathos_dev_data *data = pipe->dev->priv;

	if (data->mode != CIRC_BUF)
		return -EINVAL;
	if (!data->d.cb.buf || !data->d.cb.size)
		return -EINVAL;

	l = min(len, CIRC_CNT_TO_END(data->d.cb.head, data->d.cb.tail,
				     data->d.cb.size));
	if (!l)
		return -EAGAIN;

	memcpy(buf, &data->d.cb.buf[data->d.cb.tail], l);
	data->d.cb.tail = (data->d.cb.tail + l) & (data->d.cb.size - 1);

	if (CIRC_CNT(data->d.cb.head, data->d.cb.tail, data->d.cb.size))
		pipe_dev_trigger_event(pipe->dev, &evt_pipe_input_ready,
				       EVT_PRIO_MAX);

	return l;
}

int bathos_dev_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	struct bathos_dev_data *data = pipe->dev->priv;
	struct bathos_ll_dev_ops *ops, __ops;
	int i, stat = 0;

	ops = __get_ops(data, &__ops);
	if (!ops || (!ops->putc && !ops->write))
		return -EPERM;
	if (ops->write)
		return ops->write(data->ll_priv, buf, len);
	for (i = 0; i < len && stat >= 0; i++)
		stat = ops->putc(data->ll_priv, buf[i]);
	return stat < 0 ? stat : len;
}

int bathos_dev_close(struct bathos_pipe *pipe)
{
	int stat;
	struct bathos_dev_data *data = pipe->dev->priv;

	struct bathos_ll_dev_ops *ops, __ops;
	ops = __get_ops(data, &__ops);
	if (pipe->mode & BATHOS_MODE_INPUT) {
		stat = ops->rx_disable(data->ll_priv);
		if (stat)
			return stat;
	}
	if (pipe->mode & BATHOS_MODE_OUTPUT) {
		stat = ops->tx_enable(data->ll_priv);
		if (stat)
			return stat;
	}
	return 0;
}

static void __release_cbuf(struct bathos_dev_data *data)
{
	/* Forget about any external buffer */
	data->d.cb.ext_buf = NULL;
	/* Free current buffer first */
	if (data->d.cb.buf)
		bathos_free_buffer(data->d.cb.buf, data->d.cb.size);
}

static int __switch_to_cbuf(struct bathos_dev_data *data, int bufsize)
{
	struct bathos_ll_dev_ops *ops, __ops;
	int stat;

	ops = __get_ops(data, &__ops);
	/* Disable rx first */
	stat = ops->rx_disable(data->ll_priv);
	if (stat)
		return stat;
	data->mode = CIRC_BUF;
	/* Release circular buffer */
	__release_cbuf(data);
	/* and get another one */
	data->d.cb.size = bufsize;
	data->d.cb.buf = bathos_alloc_buffer(data->d.cb.size);
	if (!data->d.cb.buf)
		return -ENOMEM;
	/* reset head and tail */
	data->d.cb.head = data->d.cb.tail = 0;
	return stat;
}

static int __switch_to_pckt(struct bathos_dev_data *data, void *iocdata)
{
	struct bathos_ll_dev_ops *ops, __ops;
	struct dev_ioc_set_bqueue_data *bqdata = iocdata;
	int stat;

	if (!bqdata->bufs)
		return -EINVAL;
	ops = __get_ops(data, &__ops);
	/* Disable rx first */
	stat = ops->rx_disable(data->ll_priv);
	if (stat)
		return stat;
	data->mode = PACKET;
	/* Release circular buffer */
	__release_cbuf(data);
	data->d.pk.bhead = data->d.pk.btail = 0;
	data->d.pk.bsize = bqdata->bufsize;
	data->d.pk.bnum = bqdata->nbufs;
	data->d.pk.bufs = bqdata->bufs;
	data->d.pk.curr_index = 0;
	/* Send an interrupt when a buffer is full */
	data->rx_hwm = bqdata->bufsize - 1;
	return stat;
}

static int __peek_buf(struct bathos_dev_data *data, char **out)
{
	*out = NULL;
	if (!CIRC_CNT(data->d.pk.bhead, data->d.pk.btail, data->d.pk.bnum))
		return -EAGAIN;
	*out = data->d.pk.bufs[data->d.pk.btail];
	return 0;
}

static int __dequeue_buf(struct bathos_dev_data *data, char **out)
{
	int ret = __peek_buf(data, out);

	if (ret)
		return ret;
	data->d.pk.btail = (data->d.pk.btail + 1) & (data->d.pk.bnum - 1);
	return 0;
}

int bathos_dev_ioctl(struct bathos_pipe *pipe,
		     struct bathos_ioctl_data *iocdata)
{
	struct bathos_dev_data *data = pipe->dev->priv;
	
	switch (iocdata->code) {
	case DEV_IOC_RX_SET_HIGH_WATERMARK:
		if (!iocdata->data)
			return -EINVAL;
		data->rx_hwm = *(int *)iocdata->data;
		return 0;
	case DEV_IOC_RX_SET_CBUF_MODE:
		if (data->mode == CIRC_BUF)
			return 0;
		if (!iocdata->data)
			return -EINVAL;
		return __switch_to_cbuf(data, *(char *)iocdata->data);
	case DEV_IOC_RX_SET_BQUEUE_MODE:
		if (data->mode == PACKET)
			return 0;
		if (!iocdata->data)
			return -EINVAL;
		return __switch_to_pckt(data, iocdata->data);
	case DEV_IOC_RX_DEQUEUE_BUFFER:
	case DEV_IOC_RX_PEEK_BUFFER:
		if (data->mode == CIRC_BUF)
			return -EINVAL;
		if (!iocdata->data)
			return -EINVAL;
		return iocdata->code == DEV_IOC_RX_DEQUEUE_BUFFER ?
			__dequeue_buf(data, (char **)iocdata->data) :
			__peek_buf(data, (char **)iocdata->data);
	default:
		return -EINVAL;
	}
	/* NEVER REACHED */
	return -EINVAL;
}
