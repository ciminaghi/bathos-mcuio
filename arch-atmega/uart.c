/*
 * Uart driver for bathos
 */

#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/circ_buf.h>
#include <bathos/string.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <bathos/shell.h>
#include <bathos/stdlib.h>
#include <arch/hw.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define UART_BUF_SIZE 32

static struct uart_data {
	struct circ_buf cbuf;
	char buf[UART_BUF_SIZE];
	int overrun;
} uart_data;

struct bathos_dev __uart_dev;

static int uart_set_baudrate(uint32_t baud)
{
	UBRR1 = (THOS_QUARTZ / 16) / baud - 1;
	return 0;
}

static int uart_init(void)
{
	/* Target baud rate = 250000 */
	uint32_t baud = 250000;
	uart_set_baudrate(baud);
	uart_data.cbuf.head = uart_data.cbuf.tail = 0;
	UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);
	return 0;
}

#if defined CONFIG_CONSOLE_UART && CONFIG_EARLY_CONSOLE
int console_early_init(void)
{
	return uart_init();
}
#else
rom_initcall(uart_init);
#endif

static int uart_open(struct bathos_pipe *pipe)
{
	return 0;
}

static int uart_read(struct bathos_pipe *pipe, char *buf, int len)
{
	struct uart_data *data = &uart_data;
	int l;

	l = min(len, CIRC_CNT_TO_END(data->cbuf.head, data->cbuf.tail,
				     UART_BUF_SIZE));
	if (!l)
		return -EAGAIN;

	memcpy(buf, &data->buf[data->cbuf.tail], l);
	data->cbuf.tail = (data->cbuf.tail + l) & (UART_BUF_SIZE - 1);
	data->overrun = 0;

	if (CIRC_CNT(data->cbuf.head, data->cbuf.tail, UART_BUF_SIZE))
		pipe_dev_trigger_event(&__uart_dev, &evt_pipe_input_ready,
				       EVT_PRIO_MAX);

	return l;
}

static int __uart_putc(const char c)
{
	while (!(UCSR1A & (1 << UDRE1)));
	UDR1 = c;
	return 1;
}

static int uart_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
		__uart_putc(buf[i]);
	return len;
}

static void uart_close(struct bathos_pipe *pipe)
{
	/* Disable everything */
	UCSR1B &= ~((1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1));
}

ISR(USART1_RX_vect, __attribute__((section(".text.ISR"))))
{
	struct uart_data *data = &uart_data;
	int cnt_prev;
	uint8_t c;
	cnt_prev = CIRC_CNT(data->cbuf.head, data->cbuf.tail, UART_BUF_SIZE);
	if (!CIRC_SPACE(data->cbuf.head, data->cbuf.tail, UART_BUF_SIZE))
		data->overrun = 1;
	c = UDR1;
	if (!data->overrun) {
		data->buf[data->cbuf.head] = c;
		data->cbuf.head = (data->cbuf.head + 1) & (UART_BUF_SIZE - 1);
	}
	if (!cnt_prev)
		pipe_dev_trigger_event(&__uart_dev, &evt_pipe_input_ready,
				       EVT_PRIO_MAX);
}

const struct bathos_dev_ops PROGMEM uart_dev_ops = {
	.open = uart_open,
	.read = uart_read,
	.write = uart_write,
	.close = uart_close,
	/* ioctl not implemented */
};

struct bathos_dev __uart_dev __attribute__((section(".bathos_devices"),
					    aligned(2))) = {
	.name = "avr-uart",
	.ops = &uart_dev_ops,
};

/* baudrate command */
#define NBAUDRATES 2

struct baudrate {
	uint32_t val;
	char *descr; /* needed because printf of val fails for val > 32767 */
};

static const struct baudrate PROGMEM baud[NBAUDRATES] = {
	{125000, "125000"},
	{250000, "250000"}
};

static uint8_t baud_idx = 1;

static int baudrate_handler(int argc, char *argv[])
{
	int ret = 0, i;

	if (argc > 1) {
		i = argv[1][0] - '1';
		if ((i < 0) || (i > NBAUDRATES)) {
			printf("Invalid baudrate: %d\n", i);
			ret = -EINVAL;
		}
		else {
			baud_idx = i;
			uart_set_baudrate(baud[i].val);
		}
	}

	for (i = 0; i < NBAUDRATES; i++) {
		if (i == baud_idx)
			printf("*");
		else
			printf(" ");
		printf(" %d: %s\n", i + 1, baud[i].descr);
	}

	return ret;
}

static void baudrate_help(int argc, char *argv[])
{
	printf("show/set uart baudrate\n");
}

declare_shell_cmd(baudrate, baudrate_handler, baudrate_help);
