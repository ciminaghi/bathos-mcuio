/*
 * Uart driver for bathos
 */

#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/circ_buf.h>
#include <bathos/string.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <bathos/allocator.h>
#include <bathos/dev_ops.h>
#include <arch/hw.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


static uint8_t initialized;
static struct bathos_dev __uart_dev;

static int atmega_uart_rx_enable(void *priv)
{
	UCSR1B |= (1 << RXEN1) | (1 << RXCIE1);
	return 0;
}

static int atmega_uart_tx_enable(void *priv)
{
	UCSR1B |= (1 << TXEN1);
	return 0;
}

static int atmega_uart_rx_disable(void *priv)
{
	UCSR1B &= ~((1 << RXEN1) | (1 << RXCIE1));
	return 0;
}

static int atmega_uart_tx_disable(void *priv)
{
	UCSR1B &= ~(1 << TXEN1);
	return 0;
}

static int atmega_uart_putc(void *priv, const char c)
{
	while (!(UCSR1A & (1 << UDRE1)));
	UDR1 = c;
	return 1;
}

static struct bathos_ll_dev_ops PROGMEM atmega_uart_ops = {
	.putc = atmega_uart_putc,
	.rx_disable = atmega_uart_rx_disable,
	.rx_enable = atmega_uart_rx_enable,
	.tx_disable = atmega_uart_tx_disable,
	.tx_enable = atmega_uart_tx_enable,
};

static int atmega_uart_init(void)
{
	void *udata;

	if (initialized)
		return 0;
	/* Target baud rate = 125000 */
	UBRR1 = THOS_QUARTZ/(16*125000) - 1;
	udata = bathos_dev_init(&atmega_uart_ops, NULL);
	if (!udata)
		return -ENODEV;
	__uart_dev.priv = udata;
	initialized = 1;
	return 0;
}

#if defined CONFIG_CONSOLE_UART && CONFIG_EARLY_CONSOLE
int console_early_init(void)
{
	return atmega_uart_init();
}
#else
rom_initcall(atmega_uart_init);
#endif


ISR(USART1_RX_vect, __attribute__((section(".text.ISR"))))
{
	char c = UDR1;

	(void)bathos_dev_push_chars(&__uart_dev, &c, 1);
}

static int atmega_uart_open(struct bathos_pipe *pipe)
{
	int stat = 0;

	if (!initialized)
		stat = atmega_uart_init();
	if (stat)
		return stat;
	return bathos_dev_open(pipe);
}

const struct bathos_dev_ops PROGMEM uart_dev_ops = {
	.open = atmega_uart_open,
	.read = bathos_dev_read,
	.write = bathos_dev_write,
	.close = bathos_dev_close,
	.ioctl = bathos_dev_ioctl,
};

static struct bathos_dev __uart_dev __attribute__((section(".bathos_devices"),
					    aligned(2))) = {
	.name = "avr-uart",
	.ops = &uart_dev_ops,
};

