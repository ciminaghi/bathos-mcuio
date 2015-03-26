/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * Generic usb device controller code, functions to be invoked by low level
 * device controller drivers.
 */
#include <bathos/usb-device.h>
#include <bathos/usb-device-controller.h>

#define USB_REQUEST_TYPE_RCP_MASK	0x1f
#define USB_DEVICE_REQUEST		0x00
#define USB_INTERFACE_REQUEST		0x01
#define USB_ENDPOINT_REQUEST		0x02

#define USB_REQUEST_MASK		0x03
#define USB_REQUEST_SHIFT		5
#define USB_STANDARD_REQUEST		0x00
#define USB_CLASS_REQUEST		0x01
#define USB_VENDOR_REQUEST		0x02

#define USB_DESCR_DEVICE		0x01
#define USB_DESCR_CONFIG		0x02
#define USB_DESCR_STRING		0x03

static inline uint16_t _to_u16(uint8_t lo, uint16_t hi)
{
	return (hi << 8) | lo;
}

static inline int is_standard_request(uint8_t rt)
{
	/* bits 5 .. 6 == 0 */
	return !(rt & (USB_REQUEST_MASK << USB_REQUEST_SHIFT));
}

static inline uint8_t descr_type(uint16_t v)
{
	return v >> 8;
}

static inline uint8_t descr_index(uint16_t v)
{
	return v & 0xff;
}

static int control_ack(struct usb_device_controller *c)
{
	return usb_device_controller_submit_null_tx_buf(c, 0);
}

static int stall_handshake(struct usb_device_controller *c, uint8_t e)
{
	struct usb_device_tx_buf b;

	INIT_USB_PROTOCOL_STALL_BUF(b);
	return usb_device_controller_submit_tx_buf(c, e, 0, &b);
}

static int handle_get_descriptor(struct usb_device_controller *c,
				 uint8_t type, uint8_t index, uint16_t length)
{
	const struct usb_descriptor *d;
	struct usb_device_tx_buf b;
	int lim;

	switch (type) {
	case USB_DESCR_DEVICE:
		d = device_descriptors_start[index];
		lim = device_descriptors_end - &device_descriptors_start[0];
		break;
	case USB_DESCR_CONFIG:
		d = config_descriptors_start[index];
		lim = config_descriptors_end - &config_descriptors_start[0];
		break;
	case USB_DESCR_STRING:
		d = string_descriptors_start[index];
		lim = string_descriptors_end - &string_descriptors_start[0];
		break;
	default:
		return stall_handshake(c, 0);
	}
	if (index >= lim)
		/* Wrong index ? */
		return stall_handshake(c, 0);
	INIT_USB_DEVICE_TX_BUF(b, d->addr, min(length, d->len), 1);
	return usb_device_controller_submit_tx_buf(c, 0, length, &b);
}

static int handle_device_request(struct usb_device_controller *c,
				 uint8_t rt,
				 uint8_t r,
				 uint16_t value,
				 uint16_t length,
				 uint16_t index)
{
	if (!is_standard_request(rt)) {
		/* Non standard rquests are unsupported, just ack */
		return control_ack(c);
	}
	switch (r) {
	case USB_GET_STATUS:
	{
		/* Hardwired: no remote wakeup, not self-powered */
		static const uint8_t s[] = { 0, 0 };
		struct usb_device_tx_buf b =
		    USB_DEVICE_TX_BUF_INITIALIZER(s, sizeof(s), 1);

		return usb_device_controller_submit_tx_buf(c, 0, length, &b);
	}
	case USB_CLEAR_FEATURE:
		/* Actually unsupported, but we can only ack */
		return control_ack(c);
	case USB_SET_FEATURE:
		/* Actually unsupported, but we can only ack */
		return control_ack(c);
	case USB_SET_ADDRESS:
		c->next_addr = (uint8_t)value;
		return control_ack(c);
	case USB_GET_DESCRIPTOR:
	{
		uint8_t type = descr_type(value);
		uint8_t index = descr_index(value);
		return handle_get_descriptor(c, type, index, length);
	}
	case USB_GET_CONFIGURATION:
	{
		struct usb_device_tx_buf b =
		    USB_DEVICE_TX_BUF_INITIALIZER(&c->configuration, 1, 1);

		return usb_device_controller_submit_tx_buf(c, 0, length, &b);
	}
	case USB_SET_CONFIGURATION:
		usb_device_controller_set_configuration(c, (uint8_t)value);
		return control_ack(c);
	default:
		/* Unsupported requests */
		return stall_handshake(c, 0);
	}
	/* NEVER REACHED */
	return stall_handshake(c, 0);
}

/*
 * Returns 0 if request handled locally, 1 otherwise
 */
static int handle_interface_request(struct usb_device_controller *c,
				    uint8_t rt,
				    uint8_t r,
				    uint16_t value,
				    uint16_t length,
				    uint16_t index)
{
	if (!is_standard_request(rt))
		return 1;

	switch (r) {		
	case USB_GET_STATUS:
	{
		static const uint8_t s[2] = { 0, 0, };
		struct usb_device_tx_buf b =
		    USB_DEVICE_TX_BUF_INITIALIZER(s, sizeof(s), 1);

		return usb_device_controller_submit_tx_buf(c, 0, length, &b);
	}
	case USB_CLEAR_FEATURE:
	case USB_SET_FEATURE:
		return control_ack(c);
	case USB_GET_INTERFACE:
	case USB_SET_INTERFACE:
		/* Must be handled by high level protocol */
		return 1;
	default:
		return stall_handshake(c, 0);
	}
}

static int handle_endpoint_request(struct usb_device_controller *c,
				   uint8_t rt,
				   uint8_t r,
				   uint16_t value,
				   uint16_t length,
				   uint16_t index)
{
	struct usb_device_endpoint *ep = &usb_endpoints[index];
	if (!is_standard_request(rt))
		return 1;

	switch (r) {
	case USB_GET_STATUS:
	{
		struct usb_device_tx_buf b =
		    USB_DEVICE_TX_BUF_INITIALIZER(&ep->data->status,
						  sizeof(ep->data->status),
						  1);

		return usb_device_controller_submit_tx_buf(c, 0, length, &b);
	}
	case USB_SET_FEATURE:
	case USB_CLEAR_FEATURE:
	{
		int stall = r == USB_SET_FEATURE;

		if (!index)
			/* Unimplemented on ep 0 */
			return control_ack(c);
		if (value != 0)
			return control_ack(c);
		if (!c->plat->ops->stall_ep(c, index, stall))
			ep->data->status = stall ? EP_STALLED : 0;
		return control_ack(c);
	}
	case USB_SYNCH_FRAME:
	{
		uint8_t v[2] = {
			ep->data->synch_frame & 0xff,
			ep->data->synch_frame >> 8,
		};
		struct usb_device_tx_buf b =
		    USB_DEVICE_TX_BUF_INITIALIZER(v, sizeof(v), 1);

		return usb_device_controller_submit_tx_buf(c, 0, length, &b);
	}
	default:
		return stall_handshake(c, 0);
	}
}

int usb_device_controller_setup(struct usb_device_controller *c,
				const void *buf)
{
	const struct usb_setup_data *s = buf;
	uint16_t v = _to_u16(s->wValueL, s->wValueH);
	uint16_t l = _to_u16(s->wLengthL, s->wLengthH);
	uint16_t i = _to_u16(s->wIndexL, s->wIndexH);

	switch (s->bmRequestType & USB_REQUEST_TYPE_RCP_MASK) {
	case USB_DEVICE_REQUEST:
		return handle_device_request(c, s->bmRequestType, s->bRequest,
					     v, l, i);
	case USB_INTERFACE_REQUEST:
	{
		int ret;
		struct usb_device_endpoint *ep = &usb_endpoints[0];

		ret = handle_interface_request(c, s->bmRequestType, s->bRequest,
					       v, l, i);
		if (!ret)
			return ret;
		return ep->hl_ops->setup(ep, s);
	}
	case USB_ENDPOINT_REQUEST:
		return handle_endpoint_request(c, s->bmRequestType,
					       s->bRequest, v, l, i);
	}
	return stall_handshake(c, 0);
}

int usb_device_controller_in(struct usb_device_controller *c,
			     uint8_t e)
{
	if (c->next_addr) {
		c->plat->ops->set_address(c);
		c->next_addr = 0;
	}
	return 0;
}

int usb_device_controller_out(struct usb_device_controller *c,
			      uint8_t e,
			      const uint8_t *buf,
			      int len)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];

	return ep->hl_ops->out ? ep->hl_ops->out(ep, buf, len) : -ENOSYS;
}

int usb_device_controller_sof(struct usb_device_controller *c, uint8_t e)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];

	return ep->hl_ops->sof ? ep->hl_ops->sof(ep) : 0;
}
