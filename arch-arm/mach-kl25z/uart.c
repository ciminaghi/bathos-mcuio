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

#define UIDX(x) (x - __kl25z_uarts)

static struct bathos_dev __udev0, __udev1, __udev2;

static void __uart_irq_handler(void);

struct kl25z_uart {
	uint8_t tx_gpio;
	uint8_t rx_gpio;
	uint32_t irq;
	int afnum;
	int inited;
};

#define KL25ZUART(i, t, r, q, a) [i] = \
	{.tx_gpio = t, .rx_gpio = r, .irq = q, .afnum = a}

static struct kl25z_uart __kl25z_uarts[] = {
	KL25ZUART(0, GPIO_NR(PORTE, 20), GPIO_NR(PORTE, 21), NVIC_IRQ_UART0, 4),
	KL25ZUART(1, GPIO_NR(PORTE, 0), GPIO_NR(PORTE, 1), NVIC_IRQ_UART1, 3),
	KL25ZUART(2, GPIO_NR(PORTE, 22), GPIO_NR(PORTE, 23), NVIC_IRQ_UART2, 4),
};

static struct bathos_dev *__uart_devs[] = {&__udev0, &__udev1, &__udev2};

static int kl25z_uart_rx_enable(void *_priv)
{
	struct kl25z_uart *priv = (struct kl25z_uart*)_priv;
	volatile uint8_t *regs8 = (void*)regs;
	gpio_dir_af(priv->rx_gpio, 0, 1, 4);
	nvic_set_handler(priv->irq, __uart_irq_handler);
	nvic_enable(priv->irq);
	regs8[REG_UART_C2(UIDX(priv))] |= (UART_C2_RIE_MASK | UART_C2_RE_MASK);
	return 0;
}

static int kl25z_uart_tx_enable(void *_priv)
{
	struct kl25z_uart *priv = (struct kl25z_uart*)_priv;
	volatile uint8_t *regs8 = (void*)regs;
	gpio_dir_af(priv->tx_gpio, 1, 1, 4);
	regs8[REG_UART_C2(UIDX(priv))] |= UART_C2_TE_MASK;
	return 0;
}

static int kl25z_uart_rx_disable(void *_priv)
{
	struct kl25z_uart *priv = (struct kl25z_uart*)_priv;
	volatile uint8_t *regs8 = (void*)regs;
	gpio_dir_af(priv->rx_gpio, 0, 1, 0);
	regs8[REG_UART_C2(UIDX(priv))] &= ~((UART_C2_RIE_MASK | UART_C2_RE_MASK));
	nvic_disable(priv->irq);
	return 0;
}

static int kl25z_uart_tx_disable(void *_priv)
{
	struct kl25z_uart *priv = (struct kl25z_uart*)_priv;
	volatile uint8_t *regs8 = (void*)regs;
	gpio_dir_af(priv->tx_gpio, 0, 1, 0);
	regs8[REG_UART_C2(UIDX(priv))] &= ~UART_C2_TE_MASK;
	return 0;
}

static int kl25z_uart_putc(void *_priv, const char c)
{
	struct kl25z_uart *priv = (struct kl25z_uart*)_priv;
	volatile uint8_t *regs8 = (void*)regs;
	while(!(regs8[REG_UART_S1(UIDX(priv))] & UART_S1_TDRE_MASK));
	regs8[REG_UART_D(UIDX(priv))] = c;
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
	int u;

	for (u = 0; u < ARRAY_SIZE(__kl25z_uarts); u++) {
		if (!__kl25z_uarts[u].inited)
			continue;
		s1 = regs8[REG_UART_S1(u)];
		if ((s1 & UART_S1_RDRF_MASK) == 0)
			continue;
		c = regs8[REG_UART_D(u)];
		(void)bathos_dev_push_chars(__uart_devs[u], &c, 1);
	}
}

static int kl25z_uart_set_baudrate(uint8_t u, uint32_t baud)
{
	/* set BDH and BDL */
	uint32_t bd = BUS_FREQ / 16 / baud;
	volatile uint8_t *regs8 = (void*)regs;
	regs8[REG_UART_BDH(u)] &= ~(0x1f);
	regs8[REG_UART_BDH(u)] |= (bd >> 8) & 0x1f;
	regs8[REG_UART_BDL(u)] = bd & 0xff;
	return 0;
}

static int kl25z_uart_init(int u)
{
	void *udata;
	if (__kl25z_uarts[u].inited)
		return 0;

	/* Init UART #u, target baud rate = 250000 */
	switch (u) {
		case 0: regs[REG_SCGC4] |= SIM_SCGC4_UART0_MASK; break;
		case 1: regs[REG_SCGC4] |= SIM_SCGC4_UART1_MASK; break;
		case 2: regs[REG_SCGC4] |= SIM_SCGC4_UART2_MASK; break;
	}

	kl25z_uart_set_baudrate(u, 250000);
	udata = bathos_dev_init(&kl25z_dev_ops, &__kl25z_uarts[u]);
	if (!udata)
		return -ENODEV;
	__uart_devs[u]->priv = udata;
	__kl25z_uarts[u].inited = 1;
	return 0;
}

#if defined CONFIG_CONSOLE_UART && CONFIG_EARLY_CONSOLE
int console_early_init(void)
{
	return kl25z_uart_init(2);
}
#endif

static int kl25z_uart_open(struct bathos_pipe *pipe)
{
	int stat = 0;

	/* Take uart number from its name uartX */
	stat = kl25z_uart_init(pipe->dev->name[4] - '0');
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

static struct bathos_dev __udev0
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart0",
	.ops = &uart_dev_ops,
};

static struct bathos_dev __udev1
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart1",
	.ops = &uart_dev_ops,
};

static struct bathos_dev __udev2
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart2",
	.ops = &uart_dev_ops,
};
