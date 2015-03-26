/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * Generic usb device controller code, control endpoint operations
 */
#include <bathos/usb-device.h>
#include <bathos/usb-device-controller.h>
#include <bathos/pipe.h>

/*
 * From the point of view of high level protocols, a control endpoint is
 * a pipe which is opened once for each interface supporting control
 * transactions. Device names are:
 *
 * usb-ep%d-if%d
 *
 * where the first %d stands for an endpoint number and the second %d
 * stands for an interface number (0, 1, ...)
 */

/* High level operations for endpoints */

static int __ep_out(struct usb_device_endpoint *ep,
		    int index,
		    const uint8_t *data, int len)
{
	ep->data->rx_buf = data;
	ep->data->rx_cnt = len;
	pipe_dev_trigger_event_immediate((struct bathos_dev *)ep->dev[index],
					 &evt_pipe_input_ready);
	return 0;
}

static int usb_device_ep_out(struct usb_device_endpoint *ep,
			     const uint8_t *data, int len)
{
	return __ep_out(ep, 0, data, len);
}

static int usb_device_ctrl_ep_setup(struct usb_device_endpoint *ep,
				    const struct usb_setup_data *d)
{
	int index = d->wIndexL;

	return __ep_out(ep, index, (const uint8_t *)d, sizeof(*d));
}

static int usb_device_ep_sof(struct usb_device_endpoint *ep)
{
	pipe_dev_trigger_event(ep->dev[0], &evt_pipe_output_ready);
	return 0;
}

const struct usb_device_endpoint_hl_ops PROGMEM usb_device_control_ep_ops = {
	.setup = usb_device_ctrl_ep_setup,
};

const struct usb_device_endpoint_hl_ops PROGMEM usb_device_bulk_ep_ops = {
	.out = usb_device_ep_out,
};

const struct usb_device_endpoint_hl_ops PROGMEM usb_device_interrupt_ep_ops = {
	.sof = usb_device_ep_sof,
};

const struct usb_device_endpoint_hl_ops PROGMEM usb_device_iso_ep_ops = {
	.out = usb_device_ep_out,
	.sof = usb_device_ep_sof,
};

/* Pipe operations for endpoints */

static int usb_ep_open(struct bathos_pipe *pipe)
{
	struct bathos_dev *dev = pipe->dev;
	/*
	 * dev->priv must already have been initialized !
	 */
	struct usb_device_endpoint *ep = dev->priv;

	ep->data->status = 0;
	ep->data->synch_frame = 0;
	ep->data->users = 0;
	usb_device_ep_get(ep);

	return 0;
}

static int usb_ep_read(struct bathos_pipe *pipe, char *buf, int len)
{
	/*
	 * Another way for reading from an endpoint, which should be more
	 * convenient for high level protocols, is using
	 * pipe_ioctl(EP_IOC_GET_RX_BUF) to read the address of the endpoint
	 * buffer, copying data directly from such buffer and then telling
	 * the controller that the buffer has been read via
	 * pipe_ioctl(EP_IOC_NOTIFY_READ).
	 */
	struct bathos_dev *dev = pipe->dev;
	struct usb_device_endpoint *ep = dev->priv;
	struct usb_device_controller *c = ep->controller;
	int l = min(len, ep->data->rx_cnt);

	if (len < 0)
		return -EINVAL;
	if (!len)
		return 0;
	if (l <= 0)
		return -EAGAIN;
	memcpy(buf, ep->data->rx_buf, l);
	c->plat->ops->notify_read(ep->controller, ep_number(ep), l);
	return l;
}

static int usb_ep_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	/* See comments above for a possibly more efficient way of writing */
	struct bathos_dev *dev = pipe->dev;
	struct usb_device_endpoint *ep = dev->priv;
	struct usb_device_controller *c = ep->controller;
	struct usb_device_tx_buf b =
	    USB_DEVICE_TX_BUF_INITIALIZER((const uint8_t *)buf, len, 0);

	return usb_device_controller_submit_tx_buf(c, ep_number(ep),
						   USB_BUF_UNDETERMINED_LENGTH,
						   &b);
}

static int usb_ep_close(struct bathos_pipe *pipe)
{
	struct bathos_dev *dev = pipe->dev;
	struct usb_device_endpoint *ep = dev->priv;

	usb_device_ep_put(ep);
	dev->priv = NULL;
	return 0;
}


static
int usb_ep_ioctl(struct bathos_pipe *pipe, struct bathos_ioctl_data *iocdata)
{
	struct bathos_dev *dev = pipe->dev;
	struct usb_device_endpoint *ep = dev->priv;
	struct usb_device_controller *c = ep->controller;

	switch(iocdata->code) {
	case EP_IOC_GET_RX_BUF:
	{
		struct usb_device_ep_rx_buf *epbuf = iocdata->data;

		epbuf->buf = ep->data->rx_buf;
		epbuf->len = ep->data->rx_cnt;
		return 0;
	}
	case EP_IOC_NOTIFY_READ:
	{
		int l = *(int *)iocdata->data;

		c->plat->ops->notify_read(c, ep_number(ep), l);
		return l;
	}
	default:
		return -EINVAL;
	}
	/* NEVER REACHED */
	return -EINVAL;
}

const struct bathos_dev_ops PROGMEM usb_ep_dev_ops = {
	.open = usb_ep_open,
	.read = usb_ep_read,
	.write = usb_ep_write,
	.close = usb_ep_close,
	.ioctl = usb_ep_ioctl,
};
