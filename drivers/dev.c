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
	struct bathos_dev_data *out = bathos_alloc_buffer(sizeof(*out));
	if (!out)
		return out;

	memset(out, 0, sizeof(*out));
	out->mode = CIRC_BUF;
	out->ops = ops;
	out->ll_priv = priv;
	return out;
}

int bathos_dev_push_chars(struct bathos_dev *dev, const char *buf, int len)
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
	if (pipe->mode & BATHOS_MODE_INPUT && ops && ops->rx_enable) {
		stat = ops->rx_enable(data->ll_priv);
		if (stat)
			return stat;
	}
	if (pipe->mode & BATHOS_MODE_OUTPUT && ops && ops->tx_enable) {
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
	if (pipe->mode & BATHOS_MODE_INPUT && ops && ops->rx_disable) {
		stat = ops->rx_disable(data->ll_priv);
		if (stat)
			return stat;
	}
	if (pipe->mode & BATHOS_MODE_OUTPUT && ops && ops->tx_disable) {
		stat = ops->tx_enable(data->ll_priv);
		if (stat)
			return stat;
	}
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
		
	default:
		return -EINVAL;
	}
	/* NEVER REACHED */
	return -EINVAL;
}
