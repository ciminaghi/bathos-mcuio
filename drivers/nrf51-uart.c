/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>, original code by
 * Adriano Costanzo <>
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
#include <bathos/nrf51-uart.h>
#include <bathos/allocator.h>
#include <bathos/bitops.h>

#define UART_TASK_STARTRX	0x00
#define UART_TASK_STARTTX	0x08
#define UART_TASK_STOPRX	0x04
#define UART_TASK_STOPTX	0x0c

#define UART_EVENT_RXDRDY	0x108
#define UART_EVENT_TXDRDY	0x11c

#define UART_REG_INTEN		0x300
#define RXRDY			BIT(2)
#define TXRDY			BIT(7)

#define UART_REG_INTENSET	0x304
#define UART_REG_INTENCLR	0x308
#define UART_REG_ERRORSRC	0x480
#define UART_REG_ENABLE		0x500
#define UART_REG_PSELTXD	0x50c
#define UART_REG_PSELRXD	0x514
#define UART_REG_RXD		0x518
#define UART_REG_TXD		0x51c
#define UART_REG_BAUDRATE	0x524
#define UART_REG_CONFIG		0x56c

#define UART_FIFO_SIZE		6

struct nrf51_uart_priv {
	struct bathos_dev_data *dev_data;
	const struct nrf51_uart_platform_data *platform_data;
};

static int nrf51_uart_rx_enable(void *_priv)
{
	struct nrf51_uart_priv *priv = (struct nrf51_uart_priv*)_priv;
	const struct nrf51_uart_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;

	writel(RXRDY, base + UART_REG_INTENSET);
	writel(1, base + UART_TASK_STARTRX);
	bathos_enable_irq(plat->irq);
	return 0;
}

static int nrf51_uart_tx_enable(void *_priv)
{
	struct nrf51_uart_priv *priv = (struct nrf51_uart_priv*)_priv;
	const struct nrf51_uart_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;

	return 0;
}

static int nrf51_uart_rx_disable(void *_priv)
{
	struct nrf51_uart_priv *priv = (struct nrf51_uart_priv*)_priv;
	const struct nrf51_uart_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;

	writel(1, base + UART_TASK_STOPRX);
	writel(RXRDY, base + UART_REG_INTENCLR);
	bathos_disable_irq(plat->irq);
	return 0;
}

static int nrf51_uart_tx_disable(void *_priv)
{
	struct nrf51_uart_priv *priv = (struct nrf51_uart_priv*)_priv;
	const struct nrf51_uart_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;

	writel(1, base + UART_TASK_STOPTX);
	return 0;
}

static int nrf51_uart_putc(void *_priv, const char c)
{
	struct nrf51_uart_priv *priv = (struct nrf51_uart_priv*)_priv;
	const struct nrf51_uart_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;

	writel(1, base + UART_TASK_STARTTX);
	writel(c, base + UART_REG_TXD);
	while (!readl(base + UART_EVENT_TXDRDY));
	writel(0, base + UART_EVENT_TXDRDY);
	return 1;
}

const struct bathos_ll_dev_ops nrf51_ll_uart_dev_ops = {
	.putc = nrf51_uart_putc,
	.rx_disable = nrf51_uart_rx_disable,
	.rx_enable = nrf51_uart_rx_enable,
	.tx_disable = nrf51_uart_tx_disable,
	.tx_enable = nrf51_uart_tx_enable,
};

void nrf51_uart_irq_handler(struct bathos_dev *dev)
{
	const struct nrf51_uart_platform_data *plat = dev->platform_data;
	uint32_t base = plat->base;
	char c[UART_FIFO_SIZE];
	int i;

	for (i = 0; readl(base + UART_EVENT_RXDRDY); ) {
		writel(0, base + UART_EVENT_RXDRDY);
		c[i++] = readl(base + UART_REG_RXD);
		if (i < UART_FIFO_SIZE)
			continue;
		/*
		 * Local buffer is full, send chars to upper layer
		 */
		(void)bathos_dev_push_chars(dev, c, i);
		i = 0;
	}
	if (i)
		(void)bathos_dev_push_chars(dev, c, i);
}

static int nrf51_uart_set_baudrate(struct nrf51_uart_priv *priv, uint32_t baud)
{
	uint32_t base = priv->platform_data->base;
	uint32_t b;

	switch (baud) {
	case 1200:
		b = 0x0004F000;
		break;
	case 2400:
		b = 0x0009D000;
		break;
	case 4800:
		b = 0x0013B000;
		break;
	case 9600:
		b = 0x00275000;
		break;
	case 14400:
		b = 0x003B0000;
		break;
	case 19200:
		b = 0x004EA000;
		break;
	case 28800:
		b = 0x0075F000;
		break;
	case 38400:
		b = 0x009D5000;
		break;
	case 57600:
		b = 0x00EBF000;
		break;
	case 76800:
		b = 0x013A9000;
		break;
	case 115200:
		b = 0x01D7E000;
		break;
	case 230400:
		b = 0x03AFB000;
		break;
	case 250000:
		b = 0x04000000;
		break;
	case 460800:
		b = 0x075F7000;
		break;
	case 921600:
		b = 0x0EBEDFA4;
		break;
	case 1000000:
		b = 0x10000000;
		break;
	default:
		return -EINVAL;
	}
	writel(b, base + UART_REG_BAUDRATE);
	return 0;
}

int nrf51_uart_init(struct bathos_dev *dev)
{
	const struct nrf51_uart_platform_data *plat = dev->platform_data;
	struct nrf51_uart_priv *priv;
	uint32_t base = plat->base;
	int ret = 0;

	if (!plat)
		return -EINVAL;

	priv = bathos_alloc_buffer(sizeof(*priv));
	if (!priv)
		return -ENOMEM;
	priv->platform_data = plat;
	/* Enable uadr */
	writel(BIT(2), base + UART_REG_ENABLE);

	/* Setup rx and tx pins */
	writel(plat->rx_pin, base + UART_REG_PSELRXD);
	writel(plat->tx_pin, base + UART_REG_PSELTXD);
	gpio_dir_af(plat->rx_pin, 0, 1, 0);
	gpio_dir_af(plat->tx_pin, 1, 1, 0);

	nrf51_uart_set_baudrate(priv, 115200);
	priv->dev_data = bathos_dev_init(&nrf51_ll_uart_dev_ops, priv);
	if (!priv->dev_data) {
		ret = -ENOMEM;
		goto error;
	}
	dev->priv = priv->dev_data;
	return 0;

error:
	bathos_free_buffer(priv, sizeof(*priv));
	return ret;
}

static int nrf51_uart_open(struct bathos_pipe *pipe)
{
	int stat = 0;

	stat = nrf51_uart_init(pipe->dev);
	if (stat)
		return stat;
	return bathos_dev_open(pipe);
}

const struct bathos_dev_ops nrf51_uart_dev_ops = {
	.open = nrf51_uart_open,
	.read = bathos_dev_read,
	.write = bathos_dev_write,
	.close = bathos_dev_close,
	.ioctl = bathos_dev_ioctl,
};
