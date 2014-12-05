/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

/* Generic driver for usb device */

#include <mach/hw.h>
#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <bathos/bathos.h>
#include <bathos/usb.h>
#include <bathos/init.h>
#include <bathos/jiffies.h>
#include <bathos/errno.h>
#include <bathos/delay.h>
#include <bathos/stdio.h>
#include <bathos/string.h>
#include <bathos/allocator.h>
#include <bathos/io.h>

extern struct usb_ops_t __usb_ops;

volatile uint8_t *regs8 = (void*)regs;
static uint8_t ep0_rx_buf[2 * EP0_BUFSIZE];

struct bdt_t {
	uint8_t flags;
	uint8_t rsvd1;
	uint16_t bc;
	uint32_t address;
};

#define OWN_MASK	0x80
#define DATA01_MASK	0x40
#define KEEP_MASK	0x20
#define NINC_MASK	0x10
#define DTS_MASK	0x08
#define STALL_MASK	0x04

#define TOK_PID_SHIFT	2
#define TOK_PID_MASK	0x3c

struct usb_device_t {
	int nep;
	int bdt_inited;
	struct bdt_t *bdt;
};

static struct usb_device_t __usb;
static struct usb_device_t *usb = &__usb;

int usb_set_bdt(void *bdt, unsigned int nep)
{
	if (usb->bdt_inited) {
		printf("Warning: set bdt once at startup\n");
		return -EPERM;
	}

	if (nep > 16)
		return -EINVAL;

	usb->bdt = (struct bdt_t*)bdt;
	usb->nep = nep;
	regs8[REG_USB0_BDTPAGE3] = (int)usb->bdt >> 24;
	regs8[REG_USB0_BDTPAGE2] = ((int)usb->bdt >> 16) & 0xff;
	regs8[REG_USB0_BDTPAGE1] = ((int)usb->bdt >> 8)  & 0xff;
	usb->bdt_inited = 1;

	return 0;
}

/* Minimal bdt usb configuration valid for a usb serial */
#ifndef USB_EP_NUM
#define USB_EP_NUM 4
#endif
static uint8_t bdt[USB_EP_NUM * 8 * 2 * 2] __attribute__((__aligned__(512)));

static int __usb_init(void)
{
	regs[REG_SOPT2] |= SIM_SOPT2_USBSRC_MASK;
	regs[REG_SCGC4] |= SIM_SCGC4_USBOTG_MASK;
	usb_set_bdt(bdt, USB_EP_NUM);
	return 0;
}

core_initcall(__usb_init);

static int usb_request_ep(uint8_t ep, int tx, uint8_t *buf1, uint8_t *buf2,
			 int bufsize)
{
	uint8_t idx;
	struct bdt_t *bd;

	idx = ep << 2 | (tx << 1);

	bd = &usb->bdt[idx];

	if (bd->address)
		goto done;

	bd[0].address = (uint32_t)buf1;
	bd[0].bc = bufsize;

	bd[0].address = (uint32_t)buf2;
	bd[0].bc = bufsize;

	if (!tx) {
		bd[0].flags = OWN_MASK;
		bd[1].flags = OWN_MASK;
	}

	/* Enable endpoint with handshake bit set to 1 */
	regs8[REG_USB0_ENDPT(ep)] |= (tx ? 0x05 : 0x09);

done:
	return 0;
}

int usb_request_tx_ep(uint8_t ep)
{
	return usb_request_ep(ep, 1, NULL, NULL, 0);
}

int usb_request_rx_ep(uint8_t ep, void *buf, int bufsize)
{
	return usb_request_ep(ep, 0, buf, buf + bufsize, 0);
}

int usb_release_ep(uint8_t ep, int bufsize, int tx)
{
	regs8[REG_USB0_ENDPT(ep)] &= ~(tx ? 0x05 : 0x09);
	return 0;
}

#define _to_u16(l, h) ((uint16_t)h << 8 | l)

/* FIXME One for each EP */
static uint8_t *_tx;
static int _len;
static uint8_t data0 = 0;
static uint8_t odd = 0;

static uint8_t _usb_device_addr = 0;
int usb_set_address(uint8_t addr)
{
	_usb_device_addr = addr;
	return 0;
}

static void __usb_irq_handler(void)
{
	uint8_t mask;
	uint8_t stat;
	struct bdt_t *bd;

	mask = regs8[REG_USB0_ISTAT];
	stat = regs8[REG_USB0_STAT];
	bd =  &usb->bdt[stat >> 2];

	if (mask & USB_ISTAT_USBRST_MASK) {
		regs8[REG_USB0_CTL] |= USB_CTL_ODDRST_MASK;
		usb_request_rx_ep(0, ep0_rx_buf, EP0_BUFSIZE);
		usb_request_tx_ep(0);
		regs8[REG_USB0_ADDR] = 0x00;
		regs8[REG_USB0_ISTAT] = 0xff;
		if (__usb_ops.reset)
			__usb_ops.reset();
		return;
	}

	if (mask & USB_ISTAT_TOKDNE_MASK) {
		uint8_t pid = (bd->flags & TOK_PID_MASK) >> TOK_PID_SHIFT;

		if (pid == USB_TOKEN_SETUP) {
			int len;
			int i;
			struct usb_tok_setup *s =
				(struct usb_tok_setup *)bd->address;
			uint16_t s_wValue = _to_u16(s->wValueL, s->wValueH);
			uint16_t s_wLength = _to_u16(s->wLengthL, s->wLengthH);
			const struct usb_descriptor_list *d = NULL;

			data0 = DATA01_MASK;

			if (s->bRequest == 0x6) { /* get_descriptor */
				for (i = 0; ; i++) {
					d = &usb_descr_list[i];
					if ((d->wValue == s_wValue) ||
					    (!usb_descr_list[i].addr))
						break;
				}
				if (!d->addr) {
					pr_debug(
					  "usb warning: unknown descr %02x\n",
					  s_wValue);
					usb_write(0, NULL, 0);
					goto token_done;
				}
				if (d->length)
					len = d->length;
				else
					len = d->addr[0];
				usb_write(0, d->addr, min(len, s_wLength));
			}
			else if (s->bRequest == 0x5) { /* set_configuration */
				usb_write(0, NULL, 0);
				usb_set_address(s->wValueL & 0x7f);
			}
			else if (s->bRequest == 0x9) { /* set_address */
				usb_write(0, NULL, 0);
				if (__usb_ops.config)
					__usb_ops.config(s_wValue);
				goto token_done;
			}
		}
		else if (pid == USB_TOKEN_IN) {
			if (_usb_device_addr) {
				regs8[REG_USB0_ADDR] = _usb_device_addr;
				_usb_device_addr = 0;
			}
			if (_len)
				usb_write(0, _tx, _len);
		}

		if (__usb_ops.token)
			__usb_ops.token(pid, (uint8_t*)bd->address, bd->bc);
token_done:
		regs8[REG_USB0_CTL] = USB_CTL_USBENSOFEN_MASK;
		regs8[REG_USB0_ISTAT] = USB_ISTAT_TOKDNE_MASK;
	}

	if (mask & USB_ISTAT_SLEEP_MASK) {
		if (__usb_ops.sleep)
			__usb_ops.sleep();
		regs8[REG_USB0_ISTAT] |= USB_ISTAT_SLEEP_MASK;
	}

	if (mask & USB_ISTAT_ERROR_MASK) {
		printf("usb error %02x\n", regs8[REG_USB0_ERRSTAT]);
		regs8[REG_USB0_ERRSTAT] = 0xff;
		regs8[REG_USB0_ISTAT] |= USB_ISTAT_ERROR_MASK;
	}

	if (!(stat & 0x8)) { /* !tx */
		bd->bc = EP0_BUFSIZE;
		bd->flags = OWN_MASK;
	}

}

int usb_enable()
{
	nvic_set_handler(NVIC_IRQ_USBOTG, __usb_irq_handler);
	regs8[REG_USB0_INTEN] = USB_ISTAT_USBRST_MASK | USB_ISTAT_TOKDNE_MASK |
		USB_ISTAT_SLEEP_MASK | USB_ISTAT_ERROR_MASK;
	regs8[REG_USB0_ERREN] = 0xff;
	regs8[REG_USB0_USBCTRL] = 0;
	nvic_enable(NVIC_IRQ_USBOTG);
	regs8[REG_USB0_USBTRC0] |= 0x40;
	regs8[REG_USB0_CONTROL] |= USB_CONTROL_DPPULLOPNONOTG_MASK;
	regs8[REG_USB0_CTL] |= USB_CTL_USBENSOFEN_MASK;
	return 0;
}
device_initcall(usb_enable);

int usb_disable()
{
	nvic_disable(NVIC_IRQ_USBOTG);
	regs8[REG_USB0_CONTROL] &= ~USB_CONTROL_DPPULLOPNONOTG_MASK;
	regs8[REG_USB0_ENDPT(0)] = 0;
	regs8[REG_USB0_CTL] &= ~USB_CTL_USBENSOFEN_MASK;
	regs8[REG_USB0_INTEN] = 0;
	return 0;
}

int usb_write(uint8_t ep, const uint8_t *buf, int len)
{
	int idx = ep << 2 | 0x2 | odd;
	struct bdt_t *bd = &usb->bdt[idx];
	int l = len < EP0_BUFSIZE ? len : EP0_BUFSIZE;
	if (bd->flags & OWN_MASK)
		return -EAGAIN;
	bd->address = (int32_t)buf;
	bd->bc = l;
	if (len > EP0_BUFSIZE) {
		_tx = (uint8_t*)(bd->address + EP0_BUFSIZE);
		_len = len - EP0_BUFSIZE;
	}
	else {
		_tx = NULL;
		_len = 0;
	}
	bd->flags = OWN_MASK | data0;

	data0 ^= DATA01_MASK;
	odd ^= 1;
	return l;
}
