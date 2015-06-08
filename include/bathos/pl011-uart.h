#ifndef __PL011_UART_H__
#define __PL011_UART_H__

#include <stdint.h>
#include <bathos/dev_ops.h>

struct pl011_uart_priv;

struct pl011_uart_platform_data {
	int index;
	uint8_t tx_pin;
	uint8_t rx_pin;
	int irq;
	int afnum;
	int clock;
	uint32_t base;
	/* Support for pl011 uart variants ? Unused at present */
	int flags;
	int (*power)(const struct pl011_uart_platform_data *, int on);
};

extern const struct bathos_dev_ops PROGMEM pl011_uart_dev_ops;
extern void pl011_uart_irq_handler(struct bathos_dev *);
extern int pl011_uart_console_init(const struct
				   pl011_uart_platform_data *);
extern void pl011_uart_console_putc(uint32_t base, const char c);

#endif /* __FREESCALE_UART_H__ */
