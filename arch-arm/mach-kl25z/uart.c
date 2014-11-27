/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <arch/hw.h>
#include <mach/hw.h>
#include <arch/gpio.h>
#include <arch/bathos-arch.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/errno.h>
#include <bathos/delay.h>
#include <bathos/stdio.h>
#include <bathos/circ_buf.h>
#include <bathos/string.h>
#include <bathos/io.h>
#include <cpu-cortex-m0/nvic.h>

#define UART_BUF_SIZE 32
#define UART_EN_MASK (UART_C2_RIE_MASK | UART_C2_RE_MASK | UART_C2_TE_MASK)

static struct uart_data {
	struct circ_buf cbuf;
	char buf[UART_BUF_SIZE];
	int overrun;
} uart_data;

struct bathos_dev __uart_dev;

static void __uart_irq_handler(void);

static int uart_set_baudrate(uint32_t baud)
{
	/* set BDH and BDL */
	uint32_t bd = BUS_FREQ / 16 / baud;
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_BDH(2)] &= ~(0x1f);
	regs8[REG_UART_BDH(2)] |= (bd >> 8) & 0x1f;
	regs8[REG_UART_BDL(2)] = bd & 0xff;
	return 0;
}

static int uart_init(void)
{
	volatile uint8_t *regs8 = (void*)regs;
	uart_data.cbuf.head = uart_data.cbuf.tail = 0;

	/* Init UART2, target baud rate = 250000 */
	regs[REG_SCGC4] |= SIM_SCGC4_UART2_MASK;
	regs8[REG_UART_C2(2)] &= ~UART_EN_MASK;
	uart_set_baudrate(250000);
	gpio_dir_af(GPIO_NR(PORTE, 22), 1, 1, 4);
	gpio_dir_af(GPIO_NR(PORTE, 23), 0, 1, 4);
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
	volatile uint8_t *regs8 = (void*)regs;
	nvic_set_handler(NVIC_IRQ_UART2, __uart_irq_handler);
	nvic_enable(NVIC_IRQ_UART2);
	regs8[REG_UART_C2(2)] |= UART_EN_MASK;
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
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_D(2)] = c;
	while(!(regs8[REG_UART_S1(2)] & UART_S1_TDRE_MASK));
	return 1;
}

static int uart_write(struct bathos_pipe *pipe, const char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
		__uart_putc(buf[i]);
	return len;
}

int uart_write_public(struct bathos_pipe *pipe, const char *buf, int len)
{
	return uart_write(NULL, buf, len);
}

static void uart_close(struct bathos_pipe *pipe)
{
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_C2(2)] &= ~UART_EN_MASK;
	nvic_disable(NVIC_IRQ_UART2);
}

static void __uart_irq_handler(void)
{
	struct uart_data *data = &uart_data;
	int cnt_prev;
	uint8_t c;
	uint8_t s1;
	volatile uint8_t *regs8 = (void*)regs;

	/* read of S1 register is needed for intr flag clearing */
	s1 = regs8[REG_UART_S1(2)];
	if ((s1 & UART_S1_RDRF_MASK) == 0)
		return;

	cnt_prev = CIRC_CNT(data->cbuf.head, data->cbuf.tail, UART_BUF_SIZE);
	if (!CIRC_SPACE(data->cbuf.head, data->cbuf.tail, UART_BUF_SIZE))
		data->overrun = 1;
	c = regs8[REG_UART_D(2)];
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
	.name = "uart2",
	.ops = &uart_dev_ops,
};
