/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

/* Generic driver for usb device */

/* #define DEBUG 1 */
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
static uint8_t ep0_rx_buf[2 * EP0_BUFSIZE] __attribute__ ((aligned(4)));

static int __usb_flush(uint8_t ep);

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

struct endpoint_t {
	int rx_bufsize;
	int tx_pktlen;
	const uint8_t *tx_pend;
	int tx_pend_len;
	uint8_t tx_data0;
	uint8_t tx_odd;
	uint8_t tx_valid:1;
};

static struct usb_device_t __usb;
static struct usb_device_t *usb = &__usb;

/* Minimal bdt usb configuration valid for a usb serial
 * Warning: increase this if you need more endpoints in your driver */
#ifndef USB_EP_NUM
#define USB_EP_NUM 4
#endif
static uint8_t bdt[USB_EP_NUM * 8 * 2 * 2] __attribute__((__aligned__(512)));
static struct endpoint_t eps[USB_EP_NUM];

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

static int __usb_init(void)
{
	regs[REG_SOPT2] |= SIM_SOPT2_USBSRC_MASK;
	regs[REG_SCGC4] |= SIM_SCGC4_USBOTG_MASK;
	usb_set_bdt(bdt, USB_EP_NUM);
	return 0;
}

core_initcall(__usb_init);

static inline int usb_request_ep(uint8_t ep, int tx, uint8_t *buf1,
				uint8_t *buf2, int bufsize)
{
	uint8_t idx;
	struct bdt_t *bd;

	idx = ep << 2 | (tx << 1);

	bd = &usb->bdt[idx];

	bd[0].address = (uint32_t)buf1;
	bd[1].address = (uint32_t)buf2;

	bd[0].bc = bufsize;
	bd[1].bc = bufsize;

	bd[0].flags = tx ? 0: OWN_MASK;
	bd[1].flags = bd[0].flags;

	/* Enable endpoint with handshake bit set to 1 */
	regs8[REG_USB0_ENDPT(ep)] |= (tx ? 0x05 : 0x09);

done:
	return 0;
}

int usb_request_tx_ep(uint8_t ep, void *buf, int bufsize)
{
	/* no internal buffer for tx: address will be set on usb_write */
	eps[ep].tx_pktlen = bufsize;
	eps[ep].tx_valid = 0;
	eps[ep].tx_odd = 0;
	eps[ep].tx_data0 = 0;
	return usb_request_ep(ep, 1, NULL, NULL, bufsize);
}

int usb_request_rx_ep(uint8_t ep, void *buf, int bufsize)
{
	eps[ep].rx_bufsize = bufsize;
	return usb_request_ep(ep, 0, buf, buf + bufsize, bufsize);
}

int usb_release_tx_ep(uint8_t ep, int bufsize)
{
	regs8[REG_USB0_ENDPT(ep)] &= ~0x05;
	return 0;
}

int usb_release_rx_ep(uint8_t ep, int bufsize)
{
	regs8[REG_USB0_ENDPT(ep)] &= ~0x09;
	eps[ep].rx_bufsize = 0;
	return 0;
}

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
	uint8_t ep;

	mask = regs8[REG_USB0_ISTAT];
	stat = regs8[REG_USB0_STAT];
	bd =  &usb->bdt[stat >> 2];
	ep = stat >> 4;

	if (mask & USB_ISTAT_USBRST_MASK) {
		regs8[REG_USB0_CTL] |= USB_CTL_ODDRST_MASK;
		usb_request_rx_ep(0, ep0_rx_buf, EP0_BUFSIZE);
		usb_request_tx_ep(0, NULL, EP0_BUFSIZE);
		regs8[REG_USB0_ADDR] = 0x00;
		regs8[REG_USB0_ISTAT] = 0xff;
		if (__usb_ops.reset)
			__usb_ops.reset();
		return;
	}

	if (mask & USB_ISTAT_TOKDNE_MASK) {
		uint8_t pid = (bd->flags & TOK_PID_MASK) >> TOK_PID_SHIFT;

		switch (pid) {
		case USB_TOKEN_SETUP:
			eps[ep].tx_data0 = DATA01_MASK;
			break;
		case USB_TOKEN_IN:
			if (_usb_device_addr) {
				regs8[REG_USB0_ADDR] = _usb_device_addr;
				_usb_device_addr = 0;
			}
			__usb_flush(ep);
			break;
		default:
			break;
		}

		if (__usb_ops.token)
			__usb_ops.token(pid, ep, (uint8_t*)bd->address, bd->bc);
		regs8[REG_USB0_CTL] = USB_CTL_USBENSOFEN_MASK;
		regs8[REG_USB0_ISTAT] = USB_ISTAT_TOKDNE_MASK;
	}

	if (mask & USB_ISTAT_SLEEP_MASK) {
		if (__usb_ops.sleep)
			__usb_ops.sleep();
		regs8[REG_USB0_ISTAT] |= USB_ISTAT_SLEEP_MASK;
	}

	if (mask & USB_ISTAT_ERROR_MASK) {
		regs8[REG_USB0_ERRSTAT] = 0xff;
		regs8[REG_USB0_ISTAT] |= USB_ISTAT_ERROR_MASK;
	}

	if (mask & USB_ISTAT_SOFTOK_MASK) {
		if (__usb_ops.sof)
			__usb_ops.sof();
		regs8[REG_USB0_ISTAT] |= USB_ISTAT_SOFTOK_MASK;
	}

	if (!(stat & 0x8)) { /* !tx */
		bd->bc = eps[ep].rx_bufsize;
		bd->flags = OWN_MASK;
	}

}

int usb_enable()
{
	nvic_set_handler(NVIC_IRQ_USBOTG, __usb_irq_handler);
	regs8[REG_USB0_INTEN] = USB_ISTAT_USBRST_MASK | USB_ISTAT_TOKDNE_MASK |
		USB_ISTAT_SOFTOK_MASK |
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

static int __usb_flush(uint8_t ep)
{
	struct endpoint_t *e = &eps[ep];
	int idx = ep << 2 | 0x2 | e->tx_odd;
	struct bdt_t *bd = &usb->bdt[idx];
	int l = e->tx_pend_len < e->tx_pktlen ? e->tx_pend_len : e->tx_pktlen;

	if (!e->tx_valid)
		return 0;

	if (bd->flags & OWN_MASK)
		return -EAGAIN;

	/* Send packet */
	bd->address = (uint32_t)e->tx_pend;
	bd->bc = l;
	bd->flags = OWN_MASK | e->tx_data0;

	/* Prepare for next flush, if any */
	e->tx_odd ^= 1;
	e->tx_pend_len -= l;
	e->tx_data0 ^= DATA01_MASK;
	if (e->tx_pend_len == 0)
		e->tx_valid = 0;
	else
		e->tx_pend += l;

	return l;
}

int usb_write(uint8_t ep, const uint8_t *buf, int len)
{
	struct endpoint_t *e;
	if (ep > USB_EP_NUM)
		return -EINVAL;
	e = &eps[ep];

	if (e->tx_valid)
		return -EAGAIN;
	e->tx_pend = buf;
	e->tx_pend_len = len;
	e->tx_valid = 1;
	__usb_flush(ep);
	return len;
}
