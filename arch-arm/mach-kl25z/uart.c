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
#include <bathos/dev_ops.h>

static uint8_t initialized;
static struct bathos_dev __uart_dev;

static void __uart_irq_handler(void);

static int kl25z_uart_rx_enable(void *priv)
{
	volatile uint8_t *regs8 = (void*)regs;
	nvic_set_handler(NVIC_IRQ_UART2, __uart_irq_handler);
	nvic_enable(NVIC_IRQ_UART2);
	regs8[REG_UART_C2(2)] |= (UART_C2_RIE_MASK | UART_C2_RE_MASK);
	return 0;
}

static int kl25z_uart_tx_enable(void *priv)
{
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_C2(2)] |= UART_C2_TE_MASK;
	return 0;
}

static int kl25z_uart_rx_disable(void *priv)
{
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_C2(2)] &= ~((UART_C2_RIE_MASK | UART_C2_RE_MASK));
	nvic_disable(NVIC_IRQ_UART2);
	return 0;
}

static int kl25z_uart_tx_disable(void *priv)
{
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_C2(2)] &= ~UART_C2_TE_MASK;
	return 0;
}

static int kl25z_uart_putc(void *priv, const char c)
{
	volatile uint8_t *regs8 = (void*)regs;
	while(!(regs8[REG_UART_S1(2)] & UART_S1_TDRE_MASK));
	regs8[REG_UART_D(2)] = c;
	return 1;
}

const struct bathos_ll_dev_ops kl25z_dev_ops = {
	.putc = kl25z_uart_putc,
	.rx_disable = kl25z_uart_rx_disable,
	.rx_enable = kl25z_uart_rx_enable,
	.tx_disable = kl25z_uart_tx_disable,
	.tx_enable = kl25z_uart_tx_enable,
};

static void __uart_irq_handler(void)
{
	volatile uint8_t *regs8 = (void*)regs;
	char c;
	uint8_t s1;
	/* read of S1 register is needed for intr flag clearing */

	s1 = regs8[REG_UART_S1(2)];
	if ((s1 & UART_S1_RDRF_MASK) == 0)
		return;

	c = regs8[REG_UART_D(2)];
	(void)bathos_dev_push_chars(&__uart_dev, &c, 1);
}

static int kl25z_uart_set_baudrate(uint32_t baud)
{
	/* set BDH and BDL */
	uint32_t bd = BUS_FREQ / 16 / baud;
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_BDH(2)] &= ~(0x1f);
	regs8[REG_UART_BDH(2)] |= (bd >> 8) & 0x1f;
	regs8[REG_UART_BDL(2)] = bd & 0xff;
	return 0;
}

static int kl25z_uart_init(void)
{
	void *udata;
	if (initialized)
		return 0;
	/* Init UART2, target baud rate = 250000 */
	regs[REG_SCGC4] |= SIM_SCGC4_UART2_MASK;
	kl25z_uart_set_baudrate(250000);
	udata = bathos_dev_init(&kl25z_dev_ops, NULL);
	if (!udata)
		return -ENODEV;
	__uart_dev.priv = udata;
	initialized = 1;
	/* FIXME set af on tx/rx enable/disable funcs */
	gpio_dir_af(GPIO_NR(PORTE, 22), 1, 1, 4);
	gpio_dir_af(GPIO_NR(PORTE, 23), 0, 1, 4);
	return 0;
}

#if defined CONFIG_CONSOLE_UART && CONFIG_EARLY_CONSOLE
int console_early_init(void)
{
	return kl25z_uart_init();
}
#else
rom_initcall(kl25z_uart_init);
#endif

static int kl25z_uart_open(struct bathos_pipe *pipe)
{
	int stat = 0;

	if (!initialized)
		stat = kl25z_uart_init();
	if (stat)
		return stat;
	return bathos_dev_open(pipe);
}

const struct bathos_dev_ops PROGMEM uart_dev_ops = {
	.open = kl25z_uart_open,
	.read = bathos_dev_read,
	.write = bathos_dev_write,
	.close = bathos_dev_close,
	.ioctl = bathos_dev_ioctl,
};

static struct bathos_dev __uart_dev
	__attribute__((section(".bathos_devices"),
					    aligned(2))) = {
	.name = "uart2",
	.ops = &uart_dev_ops,
};
