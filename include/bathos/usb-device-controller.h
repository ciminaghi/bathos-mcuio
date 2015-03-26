/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 */
#ifndef __USB_DEVICE_CONTROLLER_H__
#define __USB_DEVICE_CONTROLLER_H__

#include <stdint.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>

struct usb_device_endpoint;
struct usb_device_controller;

/*
 * Tx buffer
 *
 * @buf: pointer to data address
 * @len: number of bytes to send. If bit 31 is 1, buffer is guaranteed not to
 * change while transmitting
 * @flags: buffer flags
 */
struct usb_device_tx_buf {
	const uint8_t	*buf;
	uint16_t	len;
#define TX_BUF_CONSTANT BIT(15)
#define TX_BUF_BUSY	BIT(14)
#define TX_BUF_NEEDS_TZ BIT(13)
#define TX_BUF_STALL	BIT(12)
	uint16_t	flags;
};

#define USB_BUF_UNDETERMINED_LENGTH 0xffff

#define USB_DEVICE_TX_BUF_INITIALIZER(__b,__l,__c)		\
	{							\
		.buf = (__b),					\
		.len = (__l),					\
		.flags = (__c) ? TX_BUF_CONSTANT : 0,		\
	}

#define INIT_USB_DEVICE_TX_BUF(__b,__a,__l,__c)			\
	do {							\
		__b.buf = __a;					\
		__b.len = (__l);				\
		__b.flags = (__c) ? TX_BUF_CONSTANT : 0;	\
	} while(0)

#define INIT_USB_PROTOCOL_STALL_BUF(__b)			\
	do {							\
		__b.buf = NULL;					\
		__b.len = 0;					\
		__b.flags = TX_BUF_STALL | TX_BUF_CONSTANT;	\
	} while(0)
	

static inline int usb_device_tx_buf_is_constant(struct usb_device_tx_buf *b)
{
	return b->flags & TX_BUF_CONSTANT;
}

static inline int usb_device_tx_buf_is_free(struct usb_device_tx_buf *b)
{
	return !(b->len & TX_BUF_BUSY);
}

static inline int usb_device_tx_buf_needs_tz(struct usb_device_tx_buf *b)
{
	return b->flags & TX_BUF_NEEDS_TZ;
}

static inline int usb_device_tx_buf_len(struct usb_device_tx_buf *b)
{
	return b->len;
}

static inline void usb_device_tx_buf_free(struct usb_device_tx_buf *b)
{
	b->flags &= ~TX_BUF_BUSY;
}

static inline void usb_device_tx_buf_add_term_zero(struct usb_device_tx_buf *b)
{
	b->flags |= TX_BUF_NEEDS_TZ;
}

static inline int usb_device_tx_buf_is_stall(struct usb_device_tx_buf *b)
{
	return b->flags & TX_BUF_STALL;
}

/*
 * Copy a tx buffer and set the destination as busy (bit 30)
 */
static inline void usb_device_tx_buf_get(struct usb_device_tx_buf *dst,
					 struct usb_device_tx_buf *src)
{
	*dst = *src;
	dst->flags |= TX_BUF_BUSY;
}

static inline
const uint8_t *usb_device_tx_buf_data(struct usb_device_tx_buf *b)
{
	return b->buf;
}

static inline void usb_device_tx_buf_tz_sent(struct usb_device_tx_buf *b)
{
	b->flags &= ~TX_BUF_NEEDS_TZ;
}

static inline int usb_device_tx_buf_update(struct usb_device_tx_buf *b, int len)
{
	b->buf += len;
	b->len -= min(b->len, len);
	return b->len;
}

/*
 * Device controller operations
 *
 * @reset: reset
 * @init: initialize
 * @enable: guess what
 * @disable: as above * @set_address: set usb address
 * @set_configuration: set configuration request was received
 * @enable_ep: enable endpoint
 * @disable_ep: disable endpoint
 * @stall_ep: stall an endpoint
 * @protocol_stall: a wrong control request was received, send a stall
 * handshake
 * @notify_read: tell the controller that incoming data have been read
 * @submit_tx_buf: submit buffer for transmission
 */
struct usb_device_controller_ops {
	int (*init)(struct usb_device_controller *);
	void (*reset)(struct usb_device_controller *);
	int (*enable)(struct usb_device_controller *);
	int (*disable)(struct usb_device_controller *);
	int (*suspend)(struct usb_device_controller *);
	int (*set_address)(struct usb_device_controller *);
	int (*set_configuration)(struct usb_device_controller *, uint8_t);
	int (*enable_ep)(struct usb_device_controller *, uint8_t ep);
	int (*disable_ep)(struct usb_device_controller *, uint8_t ep);
	int (*stall_ep)(struct usb_device_controller *, uint8_t ep, int stall);
	int (*protocol_stall)(struct usb_device_controller *, uint8_t ep);
	void (*notify_read)(struct usb_device_controller *, uint8_t ep,
			    int len);
	int (*submit_tx_buf)(struct usb_device_controller *, uint8_t ep,
			     struct usb_device_tx_buf *);
	int (*get_tx_buf_queue_depth)(struct usb_device_controller *,
				      uint8_t ep);
};

/*
 * High level operations for an endpoint: we have one set for each ep type:
 * control, interrupt, bulk and isochronous
 *
 * @in: invoked on reception of in token
 * @out: invoked on reception of out token (and relevant data)
 * @setup: invoked on reception of setup token (and relevant data when
 * applicable)
 * @sof: invoked on start of frame reception
 */
struct usb_device_endpoint_hl_ops {
	int (*in)(struct usb_device_endpoint *, uint8_t *data, int *len);
	int (*out)(struct usb_device_endpoint *, const uint8_t *data,
		   int len);
	int (*setup)(struct usb_device_endpoint *,
		     const struct usb_setup_data *);
	int (*sof)(struct usb_device_endpoint *);
};

struct usb_device_endpoint_data {
	int				users;
	const uint8_t			*rx_buf;
	int				rx_cnt;
	int				transmitting;
	struct usb_device_tx_buf	*curr_tx_buf;
	uint16_t			synch_frame;
#define EP_STALLED 1
	uint8_t				status;
	void				*ll_data;
};

struct usb_device_endpoint {
	struct		usb_device_endpoint_data *data;
	/*
	 * EP0 can correspond to multiple devices
	 * For EP0, dev is an array of one pointer per interface
	 * For the other endpoints, dev is an array of just one entry.
	 */
	struct bathos_dev			**dev;
	const struct usb_device_endpoint_hl_ops	*hl_ops;
	struct usb_device_controller		*controller;
	struct usb_device_tx_buf		*tx_bufs;
	uint8_t					ep_address;
	uint8_t					iso;
	int					size;
};

static inline uint8_t ep_number(struct usb_device_endpoint *ep)
{
	return ep->ep_address & 0x7f;
}

static inline int ep_is_tx(struct usb_device_endpoint *ep)
{
	return ep->ep_address & 0x80 ? 1 : 0;
}

static inline int ep_is_rx(struct usb_device_endpoint *ep)
{
	return ep_is_tx(ep) ? 0 : 1;
}

extern struct usb_device_endpoint usb_endpoints[];

struct usb_device_controller_platform_data {
	const struct usb_device_controller_ops *ops;
	const void *device_specific;
};

struct usb_device_controller {
	const struct usb_device_controller_platform_data * PROGMEM plat;
	uint8_t next_addr;
	uint8_t configuration;
	int users;
};

static inline int usb_device_controller_init(struct usb_device_controller *c)
{
	c->configuration = 0;
	c->users = 0;
	if (c->plat->ops->init)
		c->plat->ops->init(c);
	return 0;
}

static inline int
usb_device_controller_set_address(struct usb_device_controller *c)
{
	if (c->plat->ops->set_address)
		c->plat->ops->set_address(c);
	return 0;
}

static inline int
usb_device_controller_set_configuration(struct usb_device_controller *c,
					uint8_t config)
{
	/* Only one config supported */
	if (config > 0)
		return -EINVAL;
	c->configuration = config;
	if (c->plat->ops->set_configuration)
		c->plat->ops->set_configuration(c, config);
	return 0;
}

static inline int
usb_device_controller_submit_tx_buf(struct usb_device_controller *c,
				    uint8_t e,
				    uint16_t requested_length,
				    struct usb_device_tx_buf *b)
{
	if ((requested_length == USB_BUF_UNDETERMINED_LENGTH ||
	     (requested_length > usb_device_tx_buf_len(b))) &&
	    usb_device_tx_buf_len(b) &&
	    !(usb_device_tx_buf_len(b) % usb_endpoints[e].size)) {
		/*
		 * Length is unknown or a buffer longer than the one we're
		 * sending was requested, and the size of the submitted buffer
		 * is an integer multiple of the endpoint size -> let's tell
		 * the controller that a zero length packet must be sent
		 * at the end of the buffer to tell the host we're done with
		 * the transfer.
		 */
		usb_device_tx_buf_add_term_zero(b);
	}
	return c->plat->ops->submit_tx_buf(c, e, b);
}

static inline int
usb_device_controller_submit_null_tx_buf(struct usb_device_controller *c,
					 uint8_t e)
{
	struct usb_device_tx_buf b = USB_DEVICE_TX_BUF_INITIALIZER(NULL, 0, 1);

	return usb_device_controller_submit_tx_buf(c, e, 0, &b);
}

extern int usb_device_controller_setup(struct usb_device_controller *c,
				       const void *buf);

extern int usb_device_controller_in(struct usb_device_controller *c, uint8_t e);

extern int usb_device_controller_out(struct usb_device_controller *c,
				     uint8_t ep,
				     const uint8_t *buf,
				     int len);

extern int usb_device_controller_sof(struct usb_device_controller *c,
				     uint8_t);

static inline void usb_device_controller_reset(struct usb_device_controller *c)
{
	if (c->plat->ops->reset)
		c->plat->ops->reset(c);
}

static inline int
usb_device_controller_suspend(struct usb_device_controller *c)
{
	if (c->plat->ops->suspend)
		return c->plat->ops->suspend(c);
	return -ENOSYS;
}

static inline int usb_device_controller_get(struct usb_device_controller *c)
{
	if (!c->users++)
		c->plat->ops->enable(c);
	return c->users;
}

static inline int usb_device_controller_put(struct usb_device_controller *c)
{
	if (!c->users)
		return 0;
	c->users--;
	if (!c->users)
		c->plat->ops->disable(c);
	return c->users;
}

static inline int usb_device_ep_get(struct usb_device_endpoint *ep)
{
	struct usb_device_controller *c = ep->controller;
	int e = ep - usb_endpoints;
	
	usb_device_controller_get(c);
	/* Ep0 is enabled on reception of usb reset */
	if (!ep->data->users++ && e)
		c->plat->ops->enable_ep(c, e);
	return ep->data->users;
}

static inline int usb_device_ep_put(struct usb_device_endpoint *ep)
{
	if (!ep->data->users)
		return 0;
	ep->data->users--;
	if (!ep->data->users) {
		struct usb_device_controller *c = ep->controller;

		c->plat->ops->disable_ep(c, ep_number(ep));
		usb_device_controller_put(c);
	}
	return ep->data->users;
}

extern const struct bathos_dev_ops usb_ep_dev_ops;
extern const struct usb_device_endpoint_hl_ops usb_device_control_ep_ops,
    usb_device_bulk_ep_ops, usb_device_iso_ep_ops, usb_device_interrupt_ep_ops;

#define USB_DEVICE_CONTROLLER_NAME(i) \
    xcat(xcat(MACH, _slave_controller_),i)

#endif /* __USB_DEVICE_CONTROLLER_H__ */
