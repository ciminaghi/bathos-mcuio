/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * Low level usb device controller driver for freescale microcontrollers
 * Original code by Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */
#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/irq.h>
#include <bathos/usb-device.h>
#include <bathos/usb-device-controller.h>
#include <bathos/usb-device-freescale.h>
#include <mach/hw.h>

#ifndef USB_DEVICE_CONTROLLER_BASE
/* Default controller address, fits both the kl25 and the kl46 */
#define BASE 0x40072000UL
#endif

#define REG_USB0_PERID(b)		((b) / 1UL)
#define REG_USB0_IDCOMP(b)		((b + 4UL) / 1)
#define REG_USB0_REV(b)			((b + 8UL) / 1)
#define REG_USB0_ADDINFO(b)		((b + 0xcUL) / 1)
#define REG_USB0_OTGISTAT(b)		((b + 0x10UL) / 1)
#define REG_USB0_OTGICR(b)		((b + 0x14UL) / 1)
#define REG_USB0_OTGSTAT(b)		((b + 0x18UL) / 1)
#define REG_USB0_OTGCTL(b)		((b + 0x1cUL) / 1)

#define REG_USB0_ISTAT(b)		((b + 0x80) / 1)
/* USB_ISTAT and USB_INTEN mask */
#define USB_ISTAT_STALL_MASK		BIT(7)
#define USB_ISTAT_ATTACH_MASK		BIT(6)
#define USB_ISTAT_RESUME_MASK		BIT(5)
#define USB_ISTAT_SLEEP_MASK		BIT(4)
#define USB_ISTAT_TOKDNE_MASK		BIT(3)
#define USB_ISTAT_SOFTOK_MASK		BIT(2)
#define USB_ISTAT_ERROR_MASK		BIT(1)
#define USB_ISTAT_USBRST_MASK		BIT(0)

#define REG_USB0_INTEN(b)		((b + 0x84UL) / 1)
#define REG_USB0_ERRSTAT(b)		((b + 0x88UL) / 1)
#define REG_USB0_ERREN(b)		((b + 0x8cUL) / 1)
#define REG_USB0_STAT(b)		((b + 0x90UL) / 1)
#define USB_STAT_ODD_MASK		BIT(2)
#define USB_STAT_TX_MASK		BIT(3)
#define USB_STAT_EP_MASK		0xf
#define USB_STAT_EP_SHIFT		4

#define REG_USB0_CTL(b)			((b + 0x94UL) / 1)
/* USB_CTL mask */
#define USB_CTL_JSTATE_MASK		BIT(7)
#define USB_CTL_SE0_MASK		BIT(6)
#define USB_CTL_TXSUSPENDTOKENB_MASK	BIT(5)
#define USB_CTL_RESET_MASK		BIT(4)
#define USB_CTL_HOSTMODEEN_MASK		BIT(3)
#define USB_CTL_RESUME_MASK		BIT(2)
#define USB_CTL_ODDRST_MASK		BIT(1)
#define USB_CTL_USBENSOFEN_MASK		BIT(0)

#define REG_USB0_ADDR(b)		((b + 0x98UL) / 1)
#define REG_USB0_BDTPAGE1(b)		((b + 0x9cUL) / 1)
#define REG_USB0_FRMNUML(b)		((b + 0xa0UL) / 1)
#define REG_USB0_FRMNUMH(b)		((b + 0xa4UL) / 1)
#define REG_USB0_TOKEN(b)		((b + 0xa8UL) / 1)
#define REG_USB0_SOFTHLD(b)		((b + 0xacUL) / 1)
#define REG_USB0_BDTPAGE2(b)		((b + 0xb0UL) / 1)
#define REG_USB0_BDTPAGE3(b)		((b + 0xb4UL) / 1)
#define REG_USB0_ENDPT(b,n)		((((b + 0xc0UL) + ((unsigned long)(n))*0x4UL)) / 1)
#define EPHSHK				BIT(0)
#define EPSTALL				BIT(1)
#define EPTXEN				BIT(2)
#define EPRXEN				BIT(3)
#define EPCTLDIS			BIT(4)

#define REG_USB0_USBCTRL(b)		((b + 0x100UL) / 1)
#define REG_USB0_OBSERVE(b)		((b + 0x104UL) / 1)

#define REG_USB0_CONTROL(b)		((b + 0x108UL) / 1)
#define USB_CONTROL_DPPULLOPNONOTG_MASK	0x10

#define REG_USB0_USBTRC0(b)		((b + 0x10cUL) / 1)
#define REG_USB0_USBFRMADJUST(b)	((b + 0x114UL) / 1)

static int
usb_device_freescale_stall_ep(struct usb_device_controller *c, uint8_t e,
			      int stall);

static inline uint8_t __bd_get_pid(struct bdt *b)
{
	return (b->flags & TOK_PID_MASK) >> TOK_PID_SHIFT;
}

static inline uint8_t __get_bdt_index(uint8_t e, uint8_t tx, uint8_t odd)
{
	return ((e << 2) | (tx << 1)) + odd;
}

static inline struct bdt *__get_bdt(struct bdt *bdt, uint8_t e, uint8_t tx,
				    uint8_t odd)
{
	return &bdt[__get_bdt_index(e, tx, odd)];
}

static inline int __usb_stat_to_bd_index(uint8_t usb_stat)
{
	return usb_stat >> 2;
}

static inline int __find_free_tx_slot(struct usb_device_controller *c,
				      uint8_t e)
{
	int i, n = c->plat->ops->get_tx_buf_queue_depth(c, e);
	struct usb_device_endpoint *ep = &usb_endpoints[e];

	if (n < 0)
		return n;

	for (i = 0; i < n; i++)
		if (usb_device_tx_buf_is_free(&ep->tx_bufs[i]))
			return i;

	return -1;
}

static struct bdt *__prepare_bd(struct usb_device_controller *c, uint8_t e,
				int slot_index)
{
	int l;
	const struct usb_device_freescale_platform_data *plat
		= c->plat->device_specific;
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	struct usb_device_tx_buf *b = &ep->tx_bufs[slot_index];
	struct usb_device_freescale_ep_ll_data *lld = ep->data->ll_data;
	struct bdt *bd, *out = NULL;

	bd = __get_bdt(plat->bdt, e, 1, lld->odd);
	if (bd->address.l) {
		printf("%s: bd is busy\n", __func__);
		return out;
	}
	out = bd;
	bd->address.wp = b->buf;
	l = min(usb_device_tx_buf_len(b), ep->size);
	bd->bc = l;
	bd->flags = lld->tx_data0 | OWN_MASK;
	lld->tx_data0 ^= DATA01_MASK;
	lld->odd ^= 1;
	usb_device_tx_buf_update(b, l);
	if (!ep->data->curr_tx_buf)
		ep->data->curr_tx_buf = b;
	return out;
}

static void __free_tx_buf(struct usb_device_controller *c, uint8_t e,
			  struct usb_device_tx_buf *b)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	const struct usb_device_freescale_platform_data *plat
		= c->plat->device_specific;
	struct usb_device_freescale_buf *fb = &plat->bufs[e];

	if ((b->buf >= fb->buf1) && (b->buf < fb->buf1 + ep->size))
		fb->flags &= ~BUSY1;
	if ((b->buf >= fb->buf2) && (b->buf < fb->buf2 + ep->size))
		fb->flags &= ~BUSY2;
	usb_device_tx_buf_free(b);
}

static void __update_tx_buf(struct usb_device_controller *c, uint8_t e)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	struct usb_device_tx_buf *b = ep->data->curr_tx_buf;
	struct usb_device_tx_buf *new_b;
	struct bdt *next_bd;
	int tx_buf_index, new_tx_buf_index, needs_tz;
	uint16_t remaining;

	if (!ep->data->curr_tx_buf) {
		printf("WARNING %s with NULL curr_tx_buf\n", __func__);
		return;
	}
	tx_buf_index = ep->data->curr_tx_buf - ep->tx_bufs;
	new_tx_buf_index = tx_buf_index;
	remaining = usb_device_tx_buf_len(b);
	needs_tz = usb_device_tx_buf_needs_tz(b);
	if (!remaining && !needs_tz) {
		/*
		 * Current tx buffer done, see if we can
		 * get ready for the next one
		 */
		new_tx_buf_index = tx_buf_index ? 0 : 1;
		new_b = &ep->tx_bufs[new_tx_buf_index];
		__free_tx_buf(c, e, b);
		ep->data->curr_tx_buf = NULL;
		if (usb_device_tx_buf_is_free(new_b)) {
			/* Nothing to do */
			return;
		}
		ep->data->curr_tx_buf = new_b;
	}
	if (!remaining && needs_tz)
		usb_device_tx_buf_tz_sent(b);
	next_bd = __prepare_bd(c, e, new_tx_buf_index);
	if (!next_bd)
		printf("WARNING: %s, no next bd\n", __func__);
}

void usb_device_freescale_irq_handler(struct usb_device_controller *c)
{
	const struct usb_device_freescale_platform_data *plat
		= c->plat->device_specific;
	struct bdt *bd, *curr_bd, __bd;
	uint8_t bd_index;
	uint8_t mask;
	uint8_t stat;
	uint8_t e;
	struct usb_device_endpoint *ep;
	uint8_t *regs8 = (void *)regs;
	struct usb_device_freescale_ep_ll_data *lld;

	mask = readb(REG_USB0_ISTAT(BASE));
	stat = readb(REG_USB0_STAT(BASE));
	bd_index = __usb_stat_to_bd_index(stat);
	bd =  &plat->bdt[bd_index];
	e = stat >> USB_STAT_EP_SHIFT;
	ep = &usb_endpoints[e];
	curr_bd = bd;
	lld = ep->data->ll_data;

	if (!(stat & 0x8) && !e) { /* rx && control transfers */
		__bd = *bd;
		bd->bc = ep->size;
		bd->flags = OWN_MASK;
		curr_bd = &__bd;
	}

	if (mask & USB_ISTAT_USBRST_MASK) {
		/* Reset and enable ep 0 */
		usb_device_controller_reset(c);
		regs8[REG_USB0_ISTAT(BASE)] = 0xff;
		return;
	}

	if (mask & USB_ISTAT_STALL_MASK) {
		regs8[REG_USB0_ISTAT(BASE)] = USB_ISTAT_STALL_MASK;
		/* Unstall */
		usb_device_freescale_stall_ep(c, 0, 0);
		return;
	}

	if (mask & USB_ISTAT_TOKDNE_MASK) {
		uint8_t pid = __bd_get_pid(curr_bd);

		regs8[REG_USB0_ISTAT(BASE)] = USB_ISTAT_TOKDNE_MASK;

		switch (pid) {
		case USB_TOKEN_SETUP:
			lld->tx_data0 = DATA01_MASK;
			usb_device_controller_setup(c, curr_bd->address.rp);
			break;
		case USB_TOKEN_IN:
		{
			ep->data->transmitting = 0;

			/* Mark current bd as free */
			curr_bd->address.wp = NULL;

			usb_device_controller_in(c, e);

			__update_tx_buf(c, e);

			break;
		}
		case USB_TOKEN_OUT:
		{
			usb_device_controller_out(c, e, curr_bd->address.rp,
						  curr_bd->bc);
			curr_bd->bc = ep->size;
			curr_bd->flags = OWN_MASK;
			break;
		}
		default:
			printf("pid? 0x%02x\n", pid);
			/* Unexpected pid ? */
			break;
		}
		regs8[REG_USB0_CTL(BASE)] = USB_CTL_USBENSOFEN_MASK;
	}

	if (mask & USB_ISTAT_SLEEP_MASK) {
		regs8[REG_USB0_ISTAT(BASE)] = USB_ISTAT_SLEEP_MASK;
		
		/* Bus idle for >= 3ms */
		usb_device_controller_suspend(c);
	}

	if (mask & USB_ISTAT_ERROR_MASK) {
		regs8[REG_USB0_ISTAT(BASE)] = USB_ISTAT_ERROR_MASK;

		/* FIXME: WHAT SHOULD WE DO HERE BESIDES CLEARING THE ERROR ? */
		printf("%s: err 0x%02x\n", __func__,
		       regs8[REG_USB0_ERRSTAT(BASE)]);
		regs8[REG_USB0_ERRSTAT(BASE)] = 0xff;
	}

	if (mask & USB_ISTAT_SOFTOK_MASK) {
		regs8[REG_USB0_ISTAT(BASE)] = USB_ISTAT_SOFTOK_MASK;
		usb_device_controller_sof(c, e);
	}
}

static int usb_device_freescale_init(struct usb_device_controller *c)
{
	return 0;
}

static int
usb_device_freescale_enable_ep(struct usb_device_controller *c, uint8_t e);

static void __reset_ep(struct usb_device_controller *c, uint8_t e)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	const struct usb_device_freescale_platform_data *plat
		= c->plat->device_specific;
	int i;
	struct usb_device_freescale_ep_ll_data *lld = ep->data->ll_data;

	lld->odd = 0;
	for (i = 0; i < 2; i++)
		usb_device_tx_buf_free(&ep->tx_bufs[i]);
	for (i = 0; i < 2; i++) {
		struct bdt *bd = __get_bdt(plat->bdt, e, 1, i);
		bd->address.l = 0;
	}
	ep->data->curr_tx_buf = 0;
}

static void usb_device_freescale_reset(struct usb_device_controller *c)
{
	volatile uint8_t *regs8 = (void*)regs;

	regs8[REG_USB0_CTL(BASE)] |= USB_CTL_ODDRST_MASK;
	regs8[REG_USB0_ADDR(BASE)] = 0x00;
	__reset_ep(c, 0);
	usb_device_freescale_enable_ep(c, 0);
}

static int usb_device_freescale_enable(struct usb_device_controller *c)
{
	volatile uint8_t *regs8 = (void*)regs;
	const struct usb_device_freescale_platform_data *plat
		= c->plat->device_specific;

	regs[REG_SOPT2] |= SIM_SOPT2_USBSRC_MASK;
	regs[REG_SCGC4] |= SIM_SCGC4_USBOTG_MASK;
	writeb((uint32_t)plat->bdt >> 24, REG_USB0_BDTPAGE3(BASE));
	writeb(((uint32_t)plat->bdt >> 16) & 0xff, REG_USB0_BDTPAGE2(BASE));
	writeb(((uint32_t)plat->bdt >> 8) & 0xff, REG_USB0_BDTPAGE1(BASE));

	writeb(USB_ISTAT_USBRST_MASK | USB_ISTAT_TOKDNE_MASK |
	       USB_ISTAT_SLEEP_MASK | USB_ISTAT_ERROR_MASK |
	       USB_ISTAT_STALL_MASK, REG_USB0_INTEN(BASE));
	writeb(0xff, REG_USB0_ERREN(BASE));
	writeb(0, REG_USB0_USBCTRL(BASE));
	writeb(0x40, REG_USB0_USBTRC0(BASE));
	regs8[REG_USB0_CONTROL(BASE)] |= USB_CONTROL_DPPULLOPNONOTG_MASK;
	regs8[REG_USB0_CTL(BASE)] |= USB_CTL_USBENSOFEN_MASK;

	return bathos_enable_irq(IRQ_USBOTG);
}

static int usb_device_freescale_disable(struct usb_device_controller *c)
{
	volatile uint8_t *regs8 = (void*)regs;
	int ret;

	ret = bathos_disable_irq(IRQ_USBOTG);
	if (ret < 0)
		return ret;
	regs8[REG_USB0_CONTROL(BASE)] &= ~USB_CONTROL_DPPULLOPNONOTG_MASK;
	writeb(0, REG_USB0_ENDPT(BASE, 0));
	regs8[REG_USB0_CTL(BASE)] &= ~USB_CTL_USBENSOFEN_MASK;
	writeb(0, REG_USB0_INTEN(BASE));

	regs[REG_SOPT2] &= ~SIM_SOPT2_USBSRC_MASK;
	regs[REG_SCGC4] &= ~SIM_SCGC4_USBOTG_MASK;
	return ret;
}

static int usb_device_freescale_set_address(struct usb_device_controller *c)
{
	writeb(c->next_addr, REG_USB0_ADDR(BASE));
	return 0;
}

static int
usb_device_freescale_set_configuration(struct usb_device_controller *c,
				       uint8_t cfg)
{
	return cfg ? -EINVAL : 0;
}

static int
__do_enable_ep(struct usb_device_controller *c, uint8_t e, int tx)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	const struct usb_device_freescale_platform_data *plat
		= c->plat->device_specific;
	uint8_t idx = e << 2 | (tx << 1);
	struct bdt *bd = &plat->bdt[idx];
	uint8_t mask = 0;
	volatile uint8_t *regs8 = (void*)regs;

	if (!tx) {
		bd[0].address.rp = plat->bufs[e].buf1;
		bd[1].address.rp = plat->bufs[e].buf2;

		bd[0].bc = ep->size;
		bd[1].bc = ep->size;

		bd[0].flags = OWN_MASK;
		bd[1].flags = OWN_MASK;
	}

	mask = (tx ? EPTXEN : EPRXEN);
	if (!ep->iso)
		mask |= EPHSHK;

	/* Enable endpoint with handshake bit set to 1 */
	regs8[REG_USB0_ENDPT(BASE, e)] |= mask;
	return 0;
}

static int
usb_device_freescale_enable_ep(struct usb_device_controller *c, uint8_t e)
{
	int ret = 0;
	struct usb_device_endpoint *ep = &usb_endpoints[e];

	ret = __do_enable_ep(c, e, ep_is_tx(ep));
	if (ret < 0 || e)
		return ret;
	/* Ep 0 is bidirectional !! */
	return __do_enable_ep(c, e, 1);
}

static int
usb_device_freescale_disable_ep(struct usb_device_controller *c, uint8_t e)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	int tx = ep_is_tx(ep);
	volatile uint8_t *regs8 = (void*)regs;
	uint8_t mask = 0;

	mask = (tx ? EPTXEN : EPRXEN);
	if (!ep->iso)
		mask |= EPHSHK;

	regs8[REG_USB0_ENDPT(BASE, e)] &= ~mask;
	return 0;
}

static int
usb_device_freescale_stall_ep(struct usb_device_controller *c, uint8_t e,
			      int stall)
{
	volatile uint8_t *regs8 = (void*)regs;

	if (!!(regs8[REG_USB0_ENDPT(BASE, e)] & EPSTALL) == !!stall)
		return 0;
	
	regs8[REG_USB0_ENDPT(BASE, e)] &= ~EPSTALL;
	if (stall)
		regs8[REG_USB0_ENDPT(BASE, e)] |= EPSTALL;
	return 0;
}

static int
usb_device_freescale_protocol_stall(struct usb_device_controller *c, uint8_t ep)
{
	/* Is this correct ? */
	return usb_device_freescale_stall_ep(c, 0, 1);
}

static void
usb_device_freescale_notify_read(struct usb_device_controller *c, uint8_t e,
				 int len)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];

	if (len <= 0)
		return;
	ep->data->rx_cnt -= len;
	if (ep->data->rx_cnt < 0)
		ep->data->rx_cnt = 0; /* in case rx_cnt gets < 0 */
}

static int __clone_tx_buf(struct usb_device_controller *c, uint8_t e,
			  struct usb_device_tx_buf *b)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	const struct usb_device_freescale_platform_data *plat
		= c->plat->device_specific;
	struct usb_device_freescale_buf *fb = &plat->bufs[e];
	int l = min(b->len, ep->size);
	uint8_t mask, *dst;

	if (l <= 0)
		return l;
	for (mask = BUSY1; mask <= BUSY2; mask <<= 1)
		if (!(fb->flags & mask)) {
			fb->flags |= mask;
			dst = mask == BUSY1 ? fb->buf1 : fb->buf2;
			break;
		}
	if (mask > BUSY2)
		return -EAGAIN;
	memcpy(dst, b->buf, l);
	b->buf = dst;
	b->len = l;
	return l;
}

static int
usb_device_freescale_submit_tx_buf(struct usb_device_controller *c, uint8_t e,
				   struct usb_device_tx_buf *b)
{
	struct usb_device_endpoint *ep = &usb_endpoints[e];
	int free_tx_slot;
	struct bdt *bd;

	if (!b)
		return -EINVAL;

	free_tx_slot = __find_free_tx_slot(c, e);
	if (free_tx_slot < 0) {
		printf("%s: no free slots\n", __func__);
		return -EAGAIN;
	}

	if (!usb_device_tx_buf_is_constant(b)) {
		/* Buffer is not constant, need to clone it */
		/*
		 * WARNING: b->len may be modified because we might not be
		 * able to clone the whole input buffer
		 */
		if (__clone_tx_buf(c, e, b) < 0)
			return -EAGAIN;
	}

	/* Copy submitted buffer and mark it busy */
	usb_device_tx_buf_get(&ep->tx_bufs[free_tx_slot], b);

	if (ep->data->transmitting)
		goto end;

	if (usb_device_tx_buf_is_stall(b)) {
		usb_device_freescale_stall_ep(c, 0, 1);
		return 0;
	}

	/* Prepare buffer descriptor */
	bd = __prepare_bd(c, e, free_tx_slot);
	if (!bd) {
		printf("%s: WARNING: __prepare_bd returns error\n", __func__);
		return -ENOMEM;
	}
end:
	return usb_device_tx_buf_len(b);
}

static int
usb_device_freescale_get_tx_buf_queue_depth(struct usb_device_controller *c,
					    uint8_t ep)
{
	return 2;
}

struct usb_device_controller_ops PROGMEM freescale_usb_device_controller_ops = {
	.init = usb_device_freescale_init,
	.reset = usb_device_freescale_reset,
	.enable = usb_device_freescale_enable,
	.disable = usb_device_freescale_disable,
	.set_address = usb_device_freescale_set_address,
	.set_configuration = usb_device_freescale_set_configuration,
	.enable_ep = usb_device_freescale_enable_ep,
	.disable_ep = usb_device_freescale_disable_ep,
	.stall_ep = usb_device_freescale_stall_ep,
	.protocol_stall = usb_device_freescale_protocol_stall,
	.notify_read = usb_device_freescale_notify_read,
	.submit_tx_buf = usb_device_freescale_submit_tx_buf,
	.get_tx_buf_queue_depth = usb_device_freescale_get_tx_buf_queue_depth,
};
