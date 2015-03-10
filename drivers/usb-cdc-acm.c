/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>,
 * original code by Aurelio Colosimo <aurelio@aureliocolosimo.it>
 *
 */

#include <bathos/usb-device.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/dev_ops.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <bathos/interrupt.h>
#include <bathos/string.h>
#include <bathos/usb-cdc-acm.h>

/* CDC (communication class device) */
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22

static char __line_coding[] = {0x00, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x08};

declare_event(control_event);
declare_event(data_event);

static int usb_uart_write(void *_priv, const char *buf, int len)
{
	struct usb_cdc_data *data = _priv;

	/* Directly send to tx endpoint */
	if (!data->eps[TX])
		return -ENODEV;
	return pipe_write(data->eps[TX], buf, len);
}

static int usb_uart_rx_enable(void *_priv)
{
	struct usb_cdc_data *data = _priv;
	struct bathos_dev *dev = data->dev;
	const struct usb_device_proto_platform_data *plat =
		dev->platform_data;

	if (data->eps[RX])
		return 0;
	data->eps[RX] = pipe_open(plat->eps[RX], BATHOS_MODE_INPUT, data);
	if (!data->eps[RX])
		return -EINVAL;
	return 0;
}

static int usb_uart_tx_enable(void *_priv)
{
	struct usb_cdc_data *data = _priv;
	struct bathos_dev *dev = data->dev;
	const struct usb_device_proto_platform_data *plat =
		dev->platform_data;

	if (data->eps[TX])
		return 0;
	data->eps[TX] = pipe_open(plat->eps[TX], BATHOS_MODE_OUTPUT, data);
	if (!data->eps[TX])
		return -EINVAL;
	return 0;
}

static int usb_uart_rx_disable(void *_priv)
{
	struct usb_cdc_data *data = _priv;

	if (data->eps[RX])
		pipe_close(data->eps[RX]);
	data->eps[RX] = NULL;
	return 0;
}

static int usb_uart_tx_disable(void *_priv)
{
	struct usb_cdc_data *data = _priv;

	if (data->eps[TX])
		pipe_close(data->eps[TX]);
	data->eps[TX] = NULL;
	return 0;
}

const struct bathos_ll_dev_ops usb_uart_ll_dev_ops = {
	.write = usb_uart_write,
	.rx_disable = usb_uart_rx_disable,
	.rx_enable = usb_uart_rx_enable,
	.tx_disable = usb_uart_tx_disable,
	.tx_enable = usb_uart_tx_enable,
};


static int usb_uart_open(struct bathos_pipe *pipe)
{
	struct bathos_dev *dev = pipe->dev;
	const struct usb_device_proto_platform_data *plat =
		dev->platform_data;
	struct usb_cdc_data *data = plat->proto_specific;
	struct bathos_pipe *p;

	if (!plat)
		return -EINVAL;
	if (plat->neps < NEPS)
		return -EINVAL;

	if (dev->priv)
		return 0;

	/* First open */
	memset(data, 0, sizeof(*data));
	data->dev_data = bathos_dev_init(&usb_uart_ll_dev_ops, data);
	if (!data->dev_data)
		return -1;

	data->dev = dev;
	dev->priv = data->dev_data;

	memset(data->eps, 0, sizeof(data->eps));

	p = pipe_open(plat->eps[CONTROL], BATHOS_MODE_INPUT|BATHOS_MODE_OUTPUT,
		      data);
	if (!p)
		return -ENODEV;
	data->eps[CONTROL] = p;
	return 0;
}

static int usb_uart_close(struct bathos_pipe *pipe)
{
	int i;
	struct usb_cdc_data *data = pipe->data;

	for (i = 0; i < NEPS; i++)
		pipe_close(data->eps[i]);

	return bathos_dev_close(pipe);
}

static void control_handle(struct event_handler_data *ed)
{
	/* Control message received */
	struct bathos_pipe *p = ed->data;
	struct usb_device_ep_buf ep_buf;
	struct bathos_ioctl_data iocd = {
		.code = EP_IOC_GET_RX_BUF,
		.data = &ep_buf,
	};
	struct usb_setup_token *t;
	int stat, len;

	stat = pipe_ioctl(p, &iocd);
	if (stat < 0)
		return;
	t = (struct usb_setup_token *)ep_buf.buf;
	len = ep_buf.len;
	if (len != sizeof(*t))
		return;
	if (!(t->bmRequestType & 0x1f))
		return;
	switch (t->bRequest) {
	case CDC_GET_LINE_CODING:
		(void)pipe_write(p, __line_coding, sizeof(__line_coding));
		break;
	default:
		pipe_write(p, NULL, 0);
	}
}


declare_event_handler(control_event, NULL, control_handle, NULL);

static void data_handle(struct event_handler_data *ed)
{
	/* Data received */
	struct bathos_pipe *p = ed->data;
	struct usb_device_ep_buf ep_buf;
	struct bathos_ioctl_data iocd = {
		.code = EP_IOC_GET_RX_BUF,
		.data = &ep_buf,
	};
	struct usb_cdc_data *data = p->data;
	struct bathos_dev *dev = data->dev;
	int stat;

	stat = pipe_ioctl(p, &iocd);
	if (stat < 0)
		return;
	(void)bathos_dev_push_chars(dev, ep_buf.buf, ep_buf.len);
}

declare_event_handler(data_event, NULL, data_handle, NULL);

const struct bathos_dev_ops PROGMEM uart_dev_ops = {
	.open = usb_uart_open,
	.read = bathos_dev_read,
	.write = bathos_dev_write,
	.close = usb_uart_close,
	/* ioctl not implemented */
};
