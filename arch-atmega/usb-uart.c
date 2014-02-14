/* 
 * Code comes from:
 * USB Serial Example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_serial.html
 */

#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <arch/hw.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define MAX_ENDPOINT		4

/* Timeouts in milliseconds */
#define TRANSMIT_FLUSH_TIMEOUT	5
#define TRANSMIT_TIMEOUT	25
#define SUPPORT_ENDPOINT_HALT

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#define VENDOR_ID		0x16C0
#define PRODUCT_ID		0x047A

#define STR_MANUFACTURER	L"Dog Hunter"
#define STR_PRODUCT		L"YUN"
#define STR_SERIAL_NUMBER	L"00000"


/* standard control endpoint request types */
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11
/* HID (human interface device) */
#define HID_GET_REPORT			1
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11
/* CDC (communication class device) */
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22


#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN		0xC1
#define EP_TYPE_INTERRUPT_OUT		0xC0
#define EP_TYPE_ISOCHRONOUS_IN		0x41
#define EP_TYPE_ISOCHRONOUS_OUT		0x40
#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06
#define EP_SIZE(s)			((s) == 64 ? 0x30 :	\
					 ((s) == 32 ? 0x20 :	\
					  ((s) == 16 ? 0x10 :	\
					   0x00)))

#define ENDPOINT0_SIZE		16
#define CDC_ACM_ENDPOINT	2
#define CDC_RX_ENDPOINT		3
#define CDC_TX_ENDPOINT		4
#define CDC_ACM_SIZE		16
#define CDC_ACM_BUFFER		EP_SINGLE_BUFFER
#define CDC_RX_SIZE		64
#define CDC_RX_BUFFER 		EP_DOUBLE_BUFFER
#define CDC_TX_SIZE		64
#define CDC_TX_BUFFER		EP_DOUBLE_BUFFER



/*  Only one instance at the moment */
static struct usb_uart_data {
	volatile uint8_t usb_configuration;
	volatile int cdc_line_rtsdtr;
	uint8_t cdc_line_coding[7];

	volatile uint8_t transmit_flush_timer;
	uint8_t transmit_previous_timeout;
} __data = {
	.cdc_line_coding = {0x00, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x08},
};

static const uint8_t PROGMEM endpoint_config_table[] = {
	0,
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(CDC_ACM_SIZE) | CDC_ACM_BUFFER,
	1, EP_TYPE_BULK_OUT,      EP_SIZE(CDC_RX_SIZE) | CDC_RX_BUFFER,
	1, EP_TYPE_BULK_IN,       EP_SIZE(CDC_TX_SIZE) | CDC_TX_BUFFER
};

/* Descriptors are the data that your computer reads when it auto-detects
 * this USB device (called "enumeration" in USB lingo).  The most commonly
 * changed items are editable at the top of this file.  Changing things
 * in here should only be done by those who've read chapter 9 of the USB
 * spec and relevant portions of any USB class specifications!
 */
static const uint8_t PROGMEM device_descriptor[] = {
	/* bLength */
	18,
	/* bDescriptorType */
	1,
	/* bcdUSB */
	0x00, 0x02,
	/* bDeviceClass */
	2,
	/* bDeviceSubClass */
	0,
	/* bDeviceProtocol */
	0,
	/* bMaxPacketSize0 */
	ENDPOINT0_SIZE,
	/* idVendor */
	LSB(VENDOR_ID), MSB(VENDOR_ID),
	/* idProduct */
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),
	/* bcdDevice */
	0x00, 0x01,
	/* iManufacturer */
	1,
	/* iProduct */
	2,
	/* iSerialNumber */
	3,
	/* bNumConfigurations */
	1,
};

#define CONFIG1_DESC_SIZE (9+9+5+5+4+5+7+9+7+7)
static const uint8_t PROGMEM config1_descriptor[CONFIG1_DESC_SIZE] = {
	/* configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10 */
	/* bLength */
	9,
	/* bDescriptorType */
	2,
	/* wTotalLength */
	LSB(CONFIG1_DESC_SIZE),
	MSB(CONFIG1_DESC_SIZE),
	/* bNumInterfaces */
	2,
	/* bConfigurationValue */
	1,
	/* iConfiguration */
	0,
	/* bmAttributes */
	0xC0,
	/* bMaxPower */
	50,
	/* interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12 */
	/* bLength */
	9,
	/* bDescriptorType */
	4,
	/* bInterfaceNumber */
	0,
	/* bAlternateSetting */
	0,
	/* bNumEndpoints */
	1,
	/* bInterfaceClass */
	0x02,
	/* bInterfaceSubClass */
	0x02,
	/* bInterfaceProtocol */
	0x01,
	/* iInterface */
	0,
	/* CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26 */
	/* bFunctionLength */
	5,
	/* bDescriptorType */
	0x24,
	/* bDescriptorSubtype */
	0x00,
	/* bcdCDC */
	0x10, 0x01,
	/* Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27 */
	/* bFunctionLength */
	5,
	/* bDescriptorType */
	0x24,
	/* bDescriptorSubtype */
	0x01,
	/* bmCapabilities */
	0x01,
	/* bDataInterface */
	1,
	/*
	 * Abstract Control Management Functional Descriptor,
	 * CDC Spec 5.2.3.3, Table 28
	 */
	/* bFunctionLength */
	4,
	/* bDescriptorType */
	0x24,
	/* bDescriptorSubtype */
	0x02,
	/* bmCapabilities */
	0x06,
	/* Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33 */
	/* bFunctionLength */
	5,
	/* bDescriptorType */
	0x24,
	/* bDescriptorSubtype */
	0x06,
	/* bMasterInterface */
	0,
	/* bSlaveInterface0 */
	1,
	/* endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13 */
	/* bLength */
	7,
	/* bDescriptorType */
	5,
	/* bEndpointAddress */
	CDC_ACM_ENDPOINT | 0x80,
	/* bmAttributes (0x03=intr) */
	0x03,
	/* wMaxPacketSize */
	CDC_ACM_SIZE, 0,
	/* bInterval */
	64,
	/* interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12 */
	/* bLength */
	9,
	/* bDescriptorType */
	4,
	/* bInterfaceNumber */
	1,
	/* bAlternateSetting */
	0,
	/* bNumEndpoints */
	2,
	/* bInterfaceClass */
	0x0A,
	/* bInterfaceSubClass */
	0x00,
	/* bInterfaceProtocol */
	0x00,
	/* iInterface */
	0,
	/* endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13 */
	/* bLength */
	7,
	/* bDescriptorType */
	5,
	/* bEndpointAddress */
	CDC_RX_ENDPOINT,
	/* bmAttributes (0x02=bulk) */
	0x02,
	/* wMaxPacketSize */
	CDC_RX_SIZE, 0,
	/* bInterval */
	0,
	/* endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13 */
	/* bLength */
	7,
	/* bDescriptorType */
	5,
	/* bEndpointAddress */
	CDC_TX_ENDPOINT | 0x80,
	/* bmAttributes (0x02=bulk) */
	0x02,
	/* wMaxPacketSize */
	CDC_TX_SIZE, 0,
	/* bInterval */
	0
};


struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	int16_t wString[];
};
static const struct usb_string_descriptor_struct PROGMEM string0 = {
	4,
	3,
	{0x0409}
};
static const struct usb_string_descriptor_struct PROGMEM string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
static const struct usb_string_descriptor_struct PROGMEM string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};
static const struct usb_string_descriptor_struct PROGMEM string3 = {
	sizeof(STR_SERIAL_NUMBER),
	3,
	STR_SERIAL_NUMBER
};

static const struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} PROGMEM descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},
	{0x0303, 0x0409, (const uint8_t *)&string3, sizeof(STR_SERIAL_NUMBER)}
};
#define NUM_DESC_LIST ARRAY_SIZE(descriptor_list)

/*
 * ISR: FIXME: add interrupt support
 */
ISR(USB_GEN_vect, __attribute__((section(".text.ISR"))))
{
	uint8_t intbits, t;
	intbits = UDINT;
	UDINT = 0;
	if (intbits & (1<<EORSTI)) {
		UENUM = 0;
		UECONX = 1;
		UECFG0X = EP_TYPE_CONTROL;
		UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
		UEIENX = (1<<RXSTPE);
		__data.usb_configuration = 0;
		__data.cdc_line_rtsdtr = 0;
	}
	if (intbits & (1<<SOFI)) {
		if (__data.usb_configuration) {
			t = __data.transmit_flush_timer;
			if (t) {
				__data.transmit_flush_timer = --t;
				if (!t) {
					UENUM = CDC_TX_ENDPOINT;
					UEINTX = 0x3A;
				}
			}
		}
	}
}

/* Misc functions to wait for ready and send/receive packets */
static inline void usb_wait_in_ready(void)
{
	while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in(void)
{
	UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out(void)
{
	while (!(UEINTX & (1<<RXOUTI))) ;
}
static inline void usb_ack_out(void)
{
	UEINTX = ~(1<<RXOUTI);
}

int usb_configured(void)
{
	return __data.usb_configuration;
}

uint8_t usb_serial_get_control(void)
{
	return __data.cdc_line_rtsdtr;
}

/*
 * USB Endpoint Interrupt - endpoint 0 is handled here.
 * The other endpoints are manipulated by the user-callable
 * functions, and the start-of-frame interrupt.
 */
ISR(USB_COM_vect, __attribute__((section(".text.ISR"))))
{
	uint8_t intbits;
	const uint8_t *list;
	const uint8_t *cfg;
	uint8_t i, n, len, en;
	uint8_t *p;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t	desc_length;

	UENUM = 0;
	intbits = UEINTX;
	if (intbits & (1<<RXSTPI)) {
		bmRequestType = UEDATX;
		bRequest = UEDATX;
		wValue = UEDATX;
		wValue |= (UEDATX << 8);
		wIndex = UEDATX;
		wIndex |= (UEDATX << 8);
		wLength = UEDATX;
		wLength |= (UEDATX << 8);
		UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));
		if (bRequest == GET_DESCRIPTOR) {
			list = (const uint8_t *)descriptor_list;
			for (i=0; ; i++) {
				if (i >= NUM_DESC_LIST) {
					/* stall */
					UECONX = (1<<STALLRQ)|(1<<EPEN);
					return;
				}
				desc_val = pgm_read_word(list);
				if (desc_val != wValue) {
					list += sizeof(descriptor_list[0]);
					continue;
				}
				list += 2;
				desc_val = pgm_read_word(list);
				if (desc_val != wIndex) {
					list += sizeof(descriptor_list[0]) - 2;
					continue;
				}
				list += 2;
				desc_addr =
					(const uint8_t *)pgm_read_word(list);
				list += 2;
				desc_length = pgm_read_byte(list);
				break;
			}
			len = (wLength < 256) ? wLength : 255;
			if (len > desc_length)
				len = desc_length;
			do {
				/* wait for host ready for IN packet */
				do {
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI))
					/* abort */
					return;
				/* send IN packet */
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--) {
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);
			return;
		}
		if (bRequest == SET_ADDRESS) {
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = wValue | (1<<ADDEN);
			return;
		}
		if (bRequest == SET_CONFIGURATION && bmRequestType == 0) {
			__data.usb_configuration = wValue;
			__data.cdc_line_rtsdtr = 0;
			__data.transmit_flush_timer = 0;
			usb_send_in();
			cfg = endpoint_config_table;
			for (i=1; i<5; i++) {
				UENUM = i;
				en = pgm_read_byte(cfg++);
				UECONX = en;
				if (en) {
					UECFG0X = pgm_read_byte(cfg++);
					UECFG1X = pgm_read_byte(cfg++);
				}
			}
			UERST = 0x1E;
			UERST = 0;
			return;
		}
		if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80) {
			usb_wait_in_ready();
			UEDATX = __data.usb_configuration;
			usb_send_in();
			return;
		}
		if (bRequest == CDC_GET_LINE_CODING && bmRequestType == 0xA1) {
			usb_wait_in_ready();
			p = __data.cdc_line_coding;
			for (i=0; i<7; i++) {
				UEDATX = *p++;
			}
			usb_send_in();
			return;
		}
		if (bRequest == CDC_SET_LINE_CODING && bmRequestType == 0x21) {
			usb_wait_receive_out();
			p = __data.cdc_line_coding;
			for (i=0; i<7; i++) {
				*p++ = UEDATX;
			}
			usb_ack_out();
			usb_send_in();
			return;
		}
		if (bRequest == CDC_SET_CONTROL_LINE_STATE &&
		    bmRequestType == 0x21) {
			__data.cdc_line_rtsdtr = wValue;
			usb_wait_in_ready();
			usb_send_in();
			return;
		}
		if (bRequest == GET_STATUS) {
			usb_wait_in_ready();
			i = 0;
#ifdef SUPPORT_ENDPOINT_HALT
			if (bmRequestType == 0x82) {
				UENUM = wIndex;
				if (UECONX & (1<<STALLRQ)) i = 1;
				UENUM = 0;
			}
#endif
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return;
		}
#ifdef SUPPORT_ENDPOINT_HALT
		if ((bRequest == CLEAR_FEATURE || bRequest == SET_FEATURE)
		    && bmRequestType == 0x02 && wValue == 0) {
			i = wIndex & 0x7F;
			if (i >= 1 && i <= MAX_ENDPOINT) {
				usb_send_in();
				UENUM = i;
				if (bRequest == SET_FEATURE) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);
				} else {
					UECONX = (1<<STALLRQC)|(1<<RSTDT)|
						(1<<EPEN);
					UERST = (1 << i);
					UERST = 0;
				}
				return;
			}
		}
#endif
	}
	/* stall */
	UECONX = (1<<STALLRQ) | (1<<EPEN);
}

#define HW_CONFIG() (UHWCON = 0x01)
#define PLL_CONFIG() (PLLCSR = 0x12)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))

static int usb_uart_init(void)
{
	HW_CONFIG();
	USB_FREEZE();
	PLL_CONFIG();
	while (!(PLLCSR & (1<<PLOCK))) ;
	USB_CONFIG();
	UDCON = 0;
	__data.usb_configuration = 0;
	__data.cdc_line_rtsdtr = 0;
	UDIEN = (1<<EORSTE)|(1<<SOFE);
	while (!(UDINT & EORSTI));
	return 0;
}
rom_initcall(usb_uart_init);

static int usb_uart_open(struct bathos_pipe *pipe)
{
	return 0;
}

static int usb_uart_read(struct bathos_pipe *pipe, char *buf, int len)
{
	return -1;
}

static int8_t __usb_uart_putchar(uint8_t c)
{
	uint8_t timeout, intr_state;

	if (!__data.usb_configuration) return -1;
	intr_state = SREG;
	cli();
	UENUM = CDC_TX_ENDPOINT;
	if (__data.transmit_previous_timeout) {
		if (!(UEINTX & (1<<RWAL))) {
			SREG = intr_state;
			return -1;
		}
		__data.transmit_previous_timeout = 0;
	}
	/* wait for the FIFO to be ready to accept data */
	timeout = UDFNUML + TRANSMIT_TIMEOUT;
	while (1) {
		/* are we ready to transmit? */
		if (UEINTX & (1<<RWAL)) break;
		SREG = intr_state;
		/* 
		 * have we waited too long?  This happens if the user
		 * is not running an application that is listening
		 */
		if (UDFNUML == timeout) {
			__data.transmit_previous_timeout = 1;
			return -1;
		}
		/* has the USB gone offline? */
		if (!__data.usb_configuration)
			return -1;
		/* get ready to try checking again */
		intr_state = SREG;
		cli();
		UENUM = CDC_TX_ENDPOINT;
	}
	/* actually write the byte into the FIFO */
	UEDATX = c;
	/* if this completed a packet, transmit it now! */
	if (!(UEINTX & (1<<RWAL))) UEINTX = 0x3A;
	__data.transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
	SREG = intr_state;
	return 0;
}

int __usb_uart_write(const uint8_t *buffer, uint16_t size)
{
	uint8_t timeout, intr_state, write_size;

	/* if we're not online (enumerated and configured), error */
	if (!__data.usb_configuration)
		return -EAGAIN;
	intr_state = SREG;
	cli();
	UENUM = CDC_TX_ENDPOINT;
	/* if we gave up due to timeout before, don't wait again */
	if (__data.transmit_previous_timeout) {
		if (!(UEINTX & (1<<RWAL))) {
			SREG = intr_state;
			return -EIO;
		}
		__data.transmit_previous_timeout = 0;
	}
	/* each iteration of this loop transmits a packet */
	while (size) {
		/* wait for the FIFO to be ready to accept data */
		timeout = UDFNUML + TRANSMIT_TIMEOUT;
		while (1) {
			/* are we ready to transmit? */
			if (UEINTX & (1<<RWAL))
				break;
			SREG = intr_state;
			/*
			  have we waited too long?  This happens if the user
			  is not running an application that is listening
			*/
			if (UDFNUML == timeout) {
				__data.transmit_previous_timeout = 1;
				return -EIO;
			}
			/* has the USB gone offline? */
			if (!__data.usb_configuration)
				return -EAGAIN;
			/* get ready to try checking again */
			intr_state = SREG;
			cli();
			UENUM = CDC_TX_ENDPOINT;
		}

		/* compute how many bytes will fit into the next packet */
		write_size = CDC_TX_SIZE - UEBCLX;
		if (write_size > size)
			write_size = size;
		size -= write_size;

		/* FIXME: THIS IS UGLY ! */
		/* write the packet */
		switch (write_size) {
#if (CDC_TX_SIZE == 64)
		case 64: UEDATX = *buffer++;
		case 63: UEDATX = *buffer++;
		case 62: UEDATX = *buffer++;
		case 61: UEDATX = *buffer++;
		case 60: UEDATX = *buffer++;
		case 59: UEDATX = *buffer++;
		case 58: UEDATX = *buffer++;
		case 57: UEDATX = *buffer++;
		case 56: UEDATX = *buffer++;
		case 55: UEDATX = *buffer++;
		case 54: UEDATX = *buffer++;
		case 53: UEDATX = *buffer++;
		case 52: UEDATX = *buffer++;
		case 51: UEDATX = *buffer++;
		case 50: UEDATX = *buffer++;
		case 49: UEDATX = *buffer++;
		case 48: UEDATX = *buffer++;
		case 47: UEDATX = *buffer++;
		case 46: UEDATX = *buffer++;
		case 45: UEDATX = *buffer++;
		case 44: UEDATX = *buffer++;
		case 43: UEDATX = *buffer++;
		case 42: UEDATX = *buffer++;
		case 41: UEDATX = *buffer++;
		case 40: UEDATX = *buffer++;
		case 39: UEDATX = *buffer++;
		case 38: UEDATX = *buffer++;
		case 37: UEDATX = *buffer++;
		case 36: UEDATX = *buffer++;
		case 35: UEDATX = *buffer++;
		case 34: UEDATX = *buffer++;
		case 33: UEDATX = *buffer++;
#endif
#if (CDC_TX_SIZE >= 32)
		case 32: UEDATX = *buffer++;
		case 31: UEDATX = *buffer++;
		case 30: UEDATX = *buffer++;
		case 29: UEDATX = *buffer++;
		case 28: UEDATX = *buffer++;
		case 27: UEDATX = *buffer++;
		case 26: UEDATX = *buffer++;
		case 25: UEDATX = *buffer++;
		case 24: UEDATX = *buffer++;
		case 23: UEDATX = *buffer++;
		case 22: UEDATX = *buffer++;
		case 21: UEDATX = *buffer++;
		case 20: UEDATX = *buffer++;
		case 19: UEDATX = *buffer++;
		case 18: UEDATX = *buffer++;
		case 17: UEDATX = *buffer++;
#endif
#if (CDC_TX_SIZE >= 16)
		case 16: UEDATX = *buffer++;
		case 15: UEDATX = *buffer++;
		case 14: UEDATX = *buffer++;
		case 13: UEDATX = *buffer++;
		case 12: UEDATX = *buffer++;
		case 11: UEDATX = *buffer++;
		case 10: UEDATX = *buffer++;
		case  9: UEDATX = *buffer++;
#endif
		case  8: UEDATX = *buffer++;
		case  7: UEDATX = *buffer++;
		case  6: UEDATX = *buffer++;
		case  5: UEDATX = *buffer++;
		case  4: UEDATX = *buffer++;
		case  3: UEDATX = *buffer++;
		case  2: UEDATX = *buffer++;
		default:
		case  1: UEDATX = *buffer++;
		case  0: break;
		}
		/* if this completed a packet, transmit it now! */
		if (!(UEINTX & (1<<RWAL)))
			UEINTX = 0x3A;
		__data.transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
		SREG = intr_state;
	}
	return 0;
}

static int usb_uart_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	if (!len) {
		/* Just check if dev can be written */
		if (usb_configured() &&
		    (usb_serial_get_control() & USB_SERIAL_DTR)) {
			return len;
		}
		return -EAGAIN;
	}
	return !__usb_uart_write((const uint8_t *)buf, len) ? len : -1;
}

static void usb_uart_close(struct bathos_pipe *pipe)
{
	UDIEN &= ~(1<<EORSTE)|(1<<SOFE);
}

static struct bathos_dev_ops uart_dev_ops = {
	.open = usb_uart_open,
	.read = usb_uart_read,
	.write = usb_uart_write,
	.close = usb_uart_close,
	/* ioctl not implemented */
};

struct bathos_dev __usb_uart_dev __attribute__((section(".bathos_devices"))) = {
	.name = "usb-uart",
	.ops = &uart_dev_ops,	
};
