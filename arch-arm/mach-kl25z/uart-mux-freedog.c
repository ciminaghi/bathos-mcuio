/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Adapted from arch-atmega/usb-uart-yun.c
 */

/* FIXME: documentation about uart mux on freedog board */

#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/circ_buf.h>
#include <bathos/string.h>
#include <bathos/errno.h>
#include <bathos/types.h>
#include <bathos/shell.h>
#include <arch/hw.h>

/* UART mux */

enum uart_mux_mode {
	MIPS_CONSOLE = 0,
	MCUIO = 1
};

enum usb_uart_mux_mode {
	SHELL = 1,
	UART = 2,
};

/* ^], like telnet */
#define USB_UART_MODE_SWITCH 0x1d

/* Switch command from mips: 0xaa */
#define UART_MODE_SWITCH_MIPS 0xaa

/* Buffer length for dynamic allocation */
#define PIPE_BUF_SIZE 64

static struct uart_mux_yun_data {
	enum uart_mux_mode uart_mode;
	enum usb_uart_mux_mode usb_uart_mode;
	struct bathos_pipe *uartpipe;
} uart_data;

declare_event(mcuio_data_ready);

static int uart_mux_yun_open(struct bathos_pipe *pipe)
{
	uart_data.uartpipe = pipe_open("uart2",
			BATHOS_MODE_INPUT_OUTPUT,
			NULL);
	if (!uart_data.uartpipe)
		return -ENODEV;

	uart_data.uart_mode = MIPS_CONSOLE;
	uart_data.usb_uart_mode = UART;

	return 0;
}

static int uart_mux_yun_read(struct bathos_pipe *pipe, char *buf, int len)
{
	return pipe_read(uart_data.uartpipe, buf, len);
}

static int uart_mux_yun_write(struct bathos_pipe *pipe, const char *buf,
			      int len)
{
	return pipe_write(uart_data.uartpipe, buf, len);
}

static void uart_mux_yun_close(struct bathos_pipe *pipe)
{
	pipe_close(uart_data.uartpipe);
}

static const struct bathos_dev_ops PROGMEM uart_mux_yun_dev_ops = {
	.open = uart_mux_yun_open,
	.read = uart_mux_yun_read,
	.write = uart_mux_yun_write,
	.close = uart_mux_yun_close,
};

struct bathos_dev __uart_mux_dev __attribute__((section(".bathos_devices"),
						aligned(2))) = {
	.name = "uart-mux-freedog",
	.ops = &uart_mux_yun_dev_ops,
};

static void __do_switch_uart(void)
{
	uart_data.uart_mode = uart_data.uart_mode == MCUIO ?
		MIPS_CONSOLE : MCUIO;
	printf("\r\n%s: switched to uart mode: %s\r\n",
	       uart_data.uartpipe->dev->name,
	       uart_data.uart_mode == MCUIO ?
	       "mcuio" : "mips-console");
}

static void __do_switch_usb_uart(void)
{
	const char *m;

	switch (uart_data.usb_uart_mode) {
	case SHELL:
		uart_data.usb_uart_mode = UART;
		m = "uart";
		break;
	default:
		if (trigger_event(&evt_shell_start, NULL) < 0) {
			printf("error switching to shell mode\n");
			return;
		}
		uart_data.usb_uart_mode = SHELL;
		m = "shell";
		break;
	}
	printf("\r\nswitched to usb_uart mode: %s\r\n", m);
}

static void __pipe_input_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe = ed->data;
	char buf[PIPE_BUF_SIZE];
	int l = 0;

	if (list_empty(&pipe->dev->pipes))
		/* Not opened */
		goto done;
	
	if (pipe != uart_data.uartpipe && pipe != bathos_stdin)
		goto done;
	
	if ((uart_data.uart_mode == MIPS_CONSOLE || pipe == bathos_stdin) &&
	    uart_data.usb_uart_mode != SHELL) {
		l = pipe_read(pipe, buf, PIPE_BUF_SIZE);
		if (l <= 0)
		    goto done;
	}
	if (pipe == bathos_stdin) {
		int i;
		for (i = 0; i < l && uart_data.uartpipe; i++) {
			if (buf[i] == USB_UART_MODE_SWITCH) {
				__do_switch_usb_uart();
				break;
			}
		}

		if (uart_data.usb_uart_mode == SHELL) {
			/* Let the shell read the chars */
			trigger_event(&evt_shell_input_ready, NULL);
			goto done;
		}

		if (uart_data.uart_mode == MCUIO)
			goto done;
		/* Console uart_mode, send input to mips */
		if (uart_data.uartpipe)
			pipe_write(uart_data.uartpipe, buf, l);
		goto done;
	}
	/* Data coming from mips (avr-uart) */
	if (uart_data.uart_mode == MIPS_CONSOLE) {
		int i;
		for (i = 0; i < l; i++) {
			if (buf[i] == UART_MODE_SWITCH_MIPS) {
				__do_switch_uart();
				break;
			}
		}
		if (uart_data.uart_mode == MCUIO)
			goto done;
		pipe_write(bathos_stdout, buf, l);
		goto done;
	}
	/* MCUIO uart_mode, trigger event for mcuio */
	trigger_event(&evt_mcuio_data_ready, NULL);

done:
	return;
}

declare_event_handler(pipe_input_ready, NULL, __pipe_input_handle, NULL);

static void __shell_termination_handle(struct event_handler_data *ed)
{
	__do_switch_usb_uart();
}

declare_event_handler(shell_termination, NULL, __shell_termination_handle,
		      NULL);
