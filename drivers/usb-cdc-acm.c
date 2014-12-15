/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 *
 * cdc-acm configuration tables come from:
 * USB Serial Example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_serial.html
 */

/* #define DEBUG 1 */

#include <bathos/usb.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <bathos/interrupt.h>
#include <bathos/dev_ops.h>

#define VENDOR_ID		0x2a03
#define PRODUCT_ID		0x0001

/* FIXME usb strings should depend on platform */
#define STR_MANUFACTURER	"D\0o\0g\0 \0H\0u\0n\0t\0e\0r\0"
#define STR_PRODUCT		"F\0r\0e\0e\0d\0o\0g\0"
#define STR_SERIAL_NUMBER	"0\00\00\00\0"

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

/* CDC (communication class device) */
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22

#define CDC_ACM_ENDPOINT	1
#define CDC_RX_ENDPOINT		2
#define CDC_TX_ENDPOINT		3
#define CDC_ACM_SIZE		16
#define CDC_RX_SIZE		64
#define CDC_TX_SIZE		64

/*  Only one instance at the moment */
struct cdc_data {
	volatile uint8_t usb_configuration;
	uint8_t line_coding[7];
	int open_mode;
	int dev_initialized;
	uint8_t *txbuf;
	int txlen;
};

static struct cdc_data __data = {
	.line_coding = {0x00, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x08},
};

struct bathos_dev __usb_uart_dev;

/* Descriptors are the data that your computer reads when it auto-detects
 * this USB device (called "enumeration" in USB lingo).  The most commonly
 * changed items are editable at the top of this file.  Changing things
 * in here should only be done by those who've read chapter 9 of the USB
 * spec and relevant portions of any USB class specifications!
 */
static const uint8_t PROGMEM device_descriptor[] = {
	18,		/* bLength */
	1,		/* bDescriptorType */
	0x00, 0x02, 	/* bcdUSB */
	2,		/* bDeviceClass */
	0,		/* bDeviceSubClass */
	0,		/* bDeviceProtocol */
	EP0_BUFSIZE,	/* bMaxPacketSize0 */
	LSB(VENDOR_ID), /* idVendor LSB */
	MSB(VENDOR_ID), /* idVendor MSB */
	LSB(PRODUCT_ID),/* idProduct LSB*/
	MSB(PRODUCT_ID),/* idProduct MSB*/
	0x00, 0x01,	/* bcdDevice */
	1,		/* iManufacturer */
	2,		/* iProduct */
	3,		/* iSerialNumber */
	1,		/* bNumConfigurations */
};

#define CONFIG1_DESC_SIZE (9+9+5+5+4+5+7+9+7+7)
static const uint8_t PROGMEM config1_descriptor[CONFIG1_DESC_SIZE] = {
	/* configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10 */
	9,		/* bLength */
	2,		/* bDescriptorType */
	LSB(CONFIG1_DESC_SIZE),
	MSB(CONFIG1_DESC_SIZE),/* wTotalLength */
	2,		/* bNumInterfaces */
	1,		/* bConfigurationValue */
	0,		/* iConfiguration */
	0xC0,		/* bmAttributes */
	50,		/* bMaxPower */
	/* interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12 */
	9,		/* bLength */
	4,		/* bDescriptorType */
	0,		/* bInterfaceNumber */
	0,		/* bAlternateSetting */
	1,		/* bNumEndpoints */
	0x02,		/* bInterfaceClass */
	0x02,		/* bInterfaceSubClass */
	0x01,		/* bInterfaceProtocol */
	0,		/* iInterface */
	/* CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26 */
	5,		/* bFunctionLength */
	0x24,		/* bDescriptorType */
	0x00,		/* bDescriptorSubtype */
	0x10, 0x01,	/* bcdCDC */
	/* Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27 */
	5,		/* bFunctionLength */
	0x24,		/* bDescriptorType */
	0x01,		/* bDescriptorSubtype */
	0x01,		/* bmCapabilities */
	1,		/* bDataInterface */
	/*
	 * Abstract Control Management Functional Descriptor,
	 * CDC Spec 5.2.3.3, Table 28
	 */
	4,		/* bFunctionLength */
	0x24,		/* bDescriptorType */
	0x02,		/* bDescriptorSubtype */
	0x06,		/* bmCapabilities */
	/* Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33 */
	5,		/* bFunctionLength */
	0x24,		/* bDescriptorType */
	0x06,		/* bDescriptorSubtype */
	0,		/* bMasterInterface */
	1,		/* bSlaveInterface0 */
	/* endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13 */
	7,		/* bLength */
	5,		/* bDescriptorType */
	CDC_ACM_ENDPOINT | 0x80,/* bEndpointAddress */
	0x03,		/* bmAttributes (0x03=intr) */
	CDC_ACM_SIZE, 0,/* wMaxPacketSize */
	64,		/* bInterval */
	/* interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12 */
	9,		/* bLength */
	4,		/* bDescriptorType */
	1,		/* bInterfaceNumber */
	0,		/* bAlternateSetting */
	2,		/* bNumEndpoints */
	0x0A,		/* bInterfaceClass */
	0x00,		/* bInterfaceSubClass */
	0x00,		/* bInterfaceProtocol */
	0,		/* iInterface */
	/* endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13 */
	7,		/* bLength */
	5,		/* bDescriptorType */
	CDC_RX_ENDPOINT,/* bEndpointAddress */
	0x02,		/* bmAttributes (0x02=bulk) */
	CDC_RX_SIZE, 0,	/* wMaxPacketSize */
	0,		/* bInterval */
	/* endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13 */
	7,		/* bLength */
	5,		/* bDescriptorType */
	CDC_TX_ENDPOINT | 0x80,/* bEndpointAddress */
	0x02,		/* bmAttributes (0x02=bulk) */
	CDC_TX_SIZE, 0,	/* wMaxPacketSize */
	0		/* bInterval */
};

struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	char wString[];
};
static const struct usb_string_descriptor_struct PROGMEM str0 = {
	4,
	3,
	"\x04\x09"
};
static const struct usb_string_descriptor_struct PROGMEM str1 = {
	sizeof(STR_MANUFACTURER) + 2,
	3,
	STR_MANUFACTURER
};
static const struct usb_string_descriptor_struct PROGMEM str2 = {
	sizeof(STR_PRODUCT) + 2,
	3,
	STR_PRODUCT
};
static const struct usb_string_descriptor_struct PROGMEM str3 = {
	sizeof(STR_SERIAL_NUMBER) + 2,
	3,
	STR_SERIAL_NUMBER
};

const struct usb_descriptor_list PROGMEM usb_descr_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
	{0x0300, 0x0000, (const uint8_t *)&str0, 4},
	{0x0301, 0x0409, (const uint8_t *)&str1, sizeof(STR_MANUFACTURER) + 2},
	{0x0302, 0x0409, (const uint8_t *)&str2, sizeof(STR_PRODUCT) + 2},
	{0x0303, 0x0409, (const uint8_t *)&str3, sizeof(STR_SERIAL_NUMBER) + 2},
	{.addr = NULL}
};

static void reset_cb()
{
	console_putc('R');
	__data.usb_configuration = 0;
	__data.txlen = 0;
}

static void sleep_cb()
{
	console_putc('S');
}

#ifdef CONFIG_MACH_KL25Z
/* KL25Z dma requires two buffers, for ping-pong even/odd handling */
static uint8_t cdc_rx_buf[CDC_RX_SIZE * 2] __attribute__ ((aligned(4)));
static uint8_t cdc_acm_buf[CDC_ACM_SIZE * 2] __attribute__ ((aligned(4)));
static uint8_t cdc_tx_buf[CDC_TX_SIZE * 2] __attribute__ ((aligned(4)));
#else
static uint8_t cdc_rx_buf[CDC_RX_SIZE];
static uint8_t cdc_acm_buf[CDC_ACM_SIZE];
static uint8_t cdc_tx_buf[CDC_TX_SIZE];
#endif

#define _to_u16(l, h) ((uint16_t)h << 8 | l)

static void __handle_token_setup(uint8_t *buf, int ep, int len)
{
	int i;
	struct usb_tok_setup *s = (struct usb_tok_setup *)buf;
	uint16_t s_wValue = _to_u16(s->wValueL, s->wValueH);
	uint16_t s_wLength = _to_u16(s->wLengthL, s->wLengthH);
	const struct usb_descriptor_list *d = NULL;

	if (s->bmRequestType & 0x1f) {
		/* Ignore any request but get line coding */
		if (s->bRequest == CDC_GET_LINE_CODING)
			usb_write(ep, __data.line_coding,
				  sizeof(__data.line_coding));
		else
			usb_write(ep, NULL, 0);
	}
	else if (s->bRequest == 0x6) { /* get_descriptor */
		for (i = 0; ; i++) {
			d = &usb_descr_list[i];
			if ((d->wValue == s_wValue) ||
				(!d->addr)) {
				break;
			}
		}
		if (!d->addr) {
			pr_debug("usb warning: unknown descr %02x\n", s_wValue);
			/* FIXME A Request error would be the correct behaviour
			 * here. See usb 2.0 specs, §9.2.7.
			 * Tipically, >=2.0 hosts require the device_qualifier
			 * descriptor (0x600) which should be replied with
			 * an error request when high speed is not supported.
			 */
			usb_write(ep, NULL, 0);
			return;
		}
		if (d->length)
			len = d->length;
		else
			len = d->addr[0];
		usb_write(ep, d->addr, min(len, s_wLength));
	}
	else if (s->bRequest == 0x5) { /* set_address */
		usb_write(ep, NULL, 0);
		usb_set_address(s->wValueL & 0x7f);
	}
	else if (s->bRequest == 0x9) { /* set_configuration */
		usb_request_rx_ep(CDC_ACM_ENDPOINT, cdc_acm_buf, CDC_ACM_SIZE);
		if (__data.open_mode & BATHOS_MODE_INPUT)
			usb_request_rx_ep(CDC_RX_ENDPOINT, cdc_rx_buf,
					  CDC_RX_SIZE);
		if (__data.open_mode & BATHOS_MODE_OUTPUT)
			usb_request_tx_ep(CDC_TX_ENDPOINT, NULL, CDC_TX_SIZE);
		usb_write(ep, NULL, 0);
		__data.usb_configuration = s_wValue;
		return;
	}
}

static void token_cb(int pid, int ep, uint8_t *buf, int len)
{
	if (pid == USB_TOKEN_SETUP)
		__handle_token_setup(buf, ep, len);
	if (len && pid == USB_TOKEN_OUT && ep == CDC_RX_ENDPOINT &&
		__data.open_mode & BATHOS_MODE_INPUT)
		bathos_dev_push_chars(&__usb_uart_dev, (char*)buf, len);
}

static void sof_cb(void)
{
	/* Flushes tx buffer at each sof (~1ms), even if not empty */
	if (!__data.usb_configuration || !__data.txlen)
		return;
	usb_write(CDC_TX_ENDPOINT, __data.txbuf, __data.txlen);
	__data.txlen = 0;
#ifdef CONFIG_MACH_KL25Z
	if (__data.txbuf == cdc_tx_buf)
		__data.txbuf += CDC_TX_SIZE;
	else
		__data.txbuf -= CDC_TX_SIZE;
#endif
}
declare_usb_ops(reset_cb, sleep_cb, token_cb, sof_cb);

static int usb_uart_putc(void *priv, const char c)
{
	int flags;
	if (!__data.usb_configuration)
		return -EAGAIN;
	interrupt_disable(flags);
	__data.txbuf[__data.txlen++] = c;
	/* FIXME if (__data.txlen == CDC_TX_SIZE) flush();*/
	interrupt_restore(flags);
	return 0;
}

static int usb_uart_rx_enable(void *priv)
{
	if (__data.usb_configuration)
		usb_request_rx_ep(CDC_RX_ENDPOINT, cdc_rx_buf, CDC_RX_SIZE);
	return 0;
}

static int usb_uart_tx_enable(void *priv)
{
	if (__data.usb_configuration)
		usb_request_tx_ep(CDC_TX_ENDPOINT, NULL, CDC_TX_SIZE);
	return 0;
}

static int usb_uart_rx_disable(void *priv)
{
	if (__data.usb_configuration)
		usb_release_rx_ep(CDC_RX_ENDPOINT, CDC_RX_SIZE);
	return 0;
}

static int usb_uart_tx_disable(void *priv)
{
	if (__data.usb_configuration)
		usb_release_tx_ep(CDC_TX_ENDPOINT, CDC_TX_SIZE);
	return 0;
}

static const struct bathos_ll_dev_ops uart_ll_dev_ops = {
	.putc = usb_uart_putc,
	.rx_enable = usb_uart_rx_enable,
	.rx_disable = usb_uart_rx_disable,
	.tx_enable = usb_uart_tx_enable,
	.tx_disable = usb_uart_tx_disable,
};

int usb_uart_open(struct bathos_pipe *pipe)
{
	int stat;
	void *priv;
	if (!__data.dev_initialized) {
		priv = bathos_dev_init(&uart_ll_dev_ops, NULL);
		if (!priv)
			return -ENODEV;
		__usb_uart_dev.priv = priv;
		__data.dev_initialized = 1;
		__data.txbuf = cdc_tx_buf;
		stat = bathos_dev_open(pipe);
		if (stat) {
			return stat;
		}
	}
	__data.open_mode |= pipe->mode;
	return 0;
}

void usb_uart_close(struct bathos_pipe *pipe)
{
	bathos_dev_close(pipe);
	__data.open_mode = 0;
}

static const struct bathos_dev_ops PROGMEM uart_dev_ops = {
	.open = usb_uart_open,
	.read = bathos_dev_read,
	.write = bathos_dev_write,
	.close = usb_uart_close,
	/* ioctl not implemented */
};

struct bathos_dev __usb_uart_dev __attribute__((section(".bathos_devices"))) = {
	.name = "usb-uart",
	.ops = &uart_dev_ops,
};
