/* 
 * On the arduino yun board, the atmega32u4 UART port is connected to
 * the first UART of the ath331 MIPS CPU, used both as Linux console port
 * and as an mcuio bus. This device lets the user switch between "console"
 * mode and "mcuio" mode. In console mode, characters coming from the atheros
 * UART are redirected to bathos_stdout (a pipe attached to the usb-uart
 * device). On the other hand, in mcuio mode, data coming from the atheros can
 * be read from the device via pipe_read() (task-mcuio uses this method to
 * receive input data).
 * When working in console mode, characters coming from bathos_stdin (an
 * usb-uart based pipe again) are redirected to the mips console.
 */

#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/circ_buf.h>
#include <bathos/string.h>
#include <bathos/errno.h>
#include <arch/hw.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

enum mux_mode {
	MIPS_CONSOLE = 0,
	MCUIO = 1
};

/* ^], like telnet */
#define MODE_SWITCH 0x1d

static struct uart_mux_yun_data {
	enum mux_mode mode;
	struct bathos_pipe *secpipe;
} uart_data;

declare_event(mcuio_data_ready);

static int uart_mux_yun_open(struct bathos_pipe *pipe)
{
	uart_data.secpipe = pipe_open("avr-uart", BATHOS_MODE_INPUT_OUTPUT,
				      NULL);
	if (!uart_data.secpipe)
		return -ENODEV;
	uart_data.mode = MIPS_CONSOLE;
	return 0;
}

static int uart_mux_yun_read(struct bathos_pipe *pipe, char *buf, int len)
{
	return pipe_read(uart_data.secpipe, buf, len);
}

static int uart_mux_yun_write(struct bathos_pipe *pipe, const char *buf,
			      int len)
{
	return pipe_write(uart_data.secpipe, buf, len);
}

static void uart_mux_yun_close(struct bathos_pipe *pipe)
{
	pipe_close(uart_data.secpipe);
}

static struct bathos_dev_ops uart_mux_yun_dev_ops = {
	.open = uart_mux_yun_open,
	.read = uart_mux_yun_read,
	.write = uart_mux_yun_write,
	.close = uart_mux_yun_close,
};

struct bathos_dev __uart_mux_dev __attribute__((section(".bathos_devices"),
						aligned(2))) = {
	.name = "uart-mux-yun",
	.ops = &uart_mux_yun_dev_ops,
};


static void __pipe_input_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe = ed->data;
	char buf[20];
	int l = 0;

	if (list_empty(&pipe->dev->pipes))
		/* Not opened */
		return;

	if (pipe != uart_data.secpipe && pipe != bathos_stdin)
		return;

	if (uart_data.mode == MIPS_CONSOLE || pipe == bathos_stdin) {
		l = pipe_read(pipe, buf, sizeof(buf));
		if (l <= 0)
			return;
	}
	if (pipe == bathos_stdin) {
		int i;
		for (i = 0; i < l && uart_data.secpipe; i++) {
			if (buf[i] == MODE_SWITCH) {
				uart_data.mode = uart_data.mode == MCUIO ?
					MIPS_CONSOLE : MCUIO;
				printf("\r\n%s: switched to mode %s\r\n",
				       uart_data.secpipe->dev->name,
				       uart_data.mode == MCUIO ?
				       "mcuio" : "mips-console");
				break;
			}
		}
		if (uart_data.mode == MCUIO)
			return;
		/* Console mode, send input to mips */
		if (uart_data.secpipe)
			pipe_write(uart_data.secpipe, buf, l);
		return;
	}
	/* Data coming from mips (avr-uart) */
	if (uart_data.mode == MIPS_CONSOLE) {
		pipe_write(bathos_stdout, buf, l);
		return;
	}
	/* MCUIO mode, trigger event for mcuio */
	trigger_event(&evt_mcuio_data_ready, NULL, EVT_PRIO_MAX);
}

declare_event_handler_with_priv(pipe_input_ready, NULL, __pipe_input_handle,
				NULL, &uart_data);

