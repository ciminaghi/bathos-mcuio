/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>, original code by
 * Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

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
#include <bathos/irq.h>
#include <bathos/dev_ops.h>
#include <bathos/freescale-uart.h>

#define REG_UART_BDH(b)		(b)
#define REG_UART_BDL(b)		((b) + 1)
#define REG_UART_C1(b)		((b) + 2)

#define REG_UART_C2(b)		((b) + 3)
#define UART_C2_TIE_MASK	0x80
#define UART_C2_TCIE_MASK	0x40
#define UART_C2_RIE_MASK	0x20
#define UART_C2_ILIE_MASK	0x10
#define UART_C2_TE_MASK		0x08
#define UART_C2_RE_MASK		0x04
#define UART_C2_RWU_MASK	0x02
#define UART_C2_SBK_MASK	0x01

#define REG_UART_S1(b)		((b) + 4)
#define UART_S1_TDRE_MASK	0x80
#define UART_S1_TC_MASK		0x40
#define UART_S1_RDRF_MASK	0x20
#define UART_S1_IDLE_MASK	0x10
#define UART_S1_OR_MASK		0x08
#define UART_S1_NF_MASK		0x04
#define UART_S1_FE_MASK		0x02
#define UART_S1_PF_MASK		0x01

#define REG_UART_S2(b)		((b) + 5)
#define REG_UART_C3(b)		((b) + 6)
#define REG_UART_D(b)		((b) + 7)



static int freescale_uart_rx_enable(void *_priv)
{
	struct freescale_uart_priv *priv = (struct freescale_uart_priv*)_priv;
	const struct freescale_uart_platform_data *plat = priv->platform_data;
	volatile uint8_t *regs8 = (void*)regs;
	uint32_t base = plat->base;

	bathos_enable_irq(plat->irq);
	regs8[REG_UART_C2(base)] |= (UART_C2_RIE_MASK | UART_C2_RE_MASK);
	return 0;
}

static int freescale_uart_tx_enable(void *_priv)
{
	struct freescale_uart_priv *priv = (struct freescale_uart_priv*)_priv;
	const struct freescale_uart_platform_data *plat = priv->platform_data;
	volatile uint8_t *regs8 = (void*)regs;
	uint32_t base = plat->base;

	regs8[REG_UART_C2(base)] |= UART_C2_TE_MASK;
	return 0;
}

static int freescale_uart_rx_disable(void *_priv)
{
	struct freescale_uart_priv *priv = (struct freescale_uart_priv*)_priv;
	const struct freescale_uart_platform_data *plat = priv->platform_data;
	volatile uint8_t *regs8 = (void*)regs;
	uint32_t base = plat->base;

	regs8[REG_UART_C2(base)] &= ~((UART_C2_RIE_MASK | UART_C2_RE_MASK));
	bathos_disable_irq(plat->irq);
	return 0;
}

static int freescale_uart_tx_disable(void *_priv)
{
	struct freescale_uart_priv *priv = (struct freescale_uart_priv*)_priv;
	const struct freescale_uart_platform_data *plat = priv->platform_data;
	volatile uint8_t *regs8 = (void*)regs;
	uint32_t base = plat->base;

	regs8[REG_UART_C2(base)] &= ~UART_C2_TE_MASK;
	return 0;
}

static int freescale_uart_putc(void *_priv, const char c)
{
	struct freescale_uart_priv *priv = (struct freescale_uart_priv*)_priv;
	const struct freescale_uart_platform_data *plat = priv->platform_data;
	volatile uint8_t *regs8 = (void*)regs;
	uint32_t base = plat->base;

	while(!(regs8[REG_UART_S1(base)] & UART_S1_TDRE_MASK));
	regs8[REG_UART_D(base)] = c;
	return 1;
}

const struct bathos_ll_dev_ops freescale_ll_uart_dev_ops = {
	.putc = freescale_uart_putc,
	.rx_disable = freescale_uart_rx_disable,
	.rx_enable = freescale_uart_rx_enable,
	.tx_disable = freescale_uart_tx_disable,
	.tx_enable = freescale_uart_tx_enable,
};

void freescale_uart_irq_handler(void *_priv)
{
	volatile uint8_t *regs8 = (void*)regs;
	struct freescale_uart_priv *priv = _priv;
	uint32_t base = priv->platform_data->base;
	char c;
	uint8_t s1;

	s1 = regs8[REG_UART_S1(base)];
	if ((s1 & UART_S1_RDRF_MASK) == 0)
		/* Spurious irq ? */
		return ;
	c = regs8[REG_UART_D(base)];
	(void)bathos_dev_push_chars(priv->dev, &c, 1);
}

static int freescale_uart_set_baudrate(struct freescale_uart_priv *priv,
				       uint32_t baud)
{
	/* set BDH and BDL */
	uint32_t bd = BUS_FREQ / 16 / baud;
	volatile uint8_t *regs8 = (void*)regs;
	uint32_t base = priv->platform_data->base;

	regs8[REG_UART_BDH(base)] &= ~(0x1f);
	regs8[REG_UART_BDH(base)] |= (bd >> 8) & 0x1f;
	regs8[REG_UART_BDL(base)] = bd & 0xff;
	return 0;
}

static int freescale_uart_init(struct freescale_uart_priv *priv)
{
	int ret = 0;
	const struct freescale_uart_platform_data *plat = priv->platform_data;

	if (priv->inited)
		return ret;

	if (!plat)
		return -EINVAL;

	if (plat->power)
		ret = plat->power(priv, 1);
	if (ret < 0)
		return ret;

	/* Setup rx and tx pins */
	gpio_dir_af(plat->rx_pin, 0, 1, plat->afnum);
	gpio_dir_af(plat->tx_pin, 0, 1, plat->afnum);

	freescale_uart_set_baudrate(priv, 250000);
	priv->dev_data = bathos_dev_init(&freescale_ll_uart_dev_ops, priv);
	if (!priv->dev_data)
		return -ENODEV;
	priv->inited = 1;
	return 0;
}

static int freescale_uart_open(struct bathos_pipe *pipe)
{
	int stat = 0;

	stat = freescale_uart_init(pipe->dev->priv);
	if (stat)
		return stat;
	return bathos_dev_open(pipe);
}

const struct bathos_dev_ops freescale_uart_dev_ops = {
	.open = freescale_uart_open,
	.read = bathos_dev_read,
	.write = bathos_dev_write,
	.close = bathos_dev_close,
	.ioctl = bathos_dev_ioctl,
};
