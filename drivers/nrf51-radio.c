/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

/*
 * Basic driver for nrf51 radio, just sends and receives packets. No collision
 * avoidance is performed at this level. Pipe interface is provided.
 * Radio is always in rx mode until a packet is written via write. As soon as
 * tx has been completed, radio is switched back to rx mode.
 * 
 * Events: pipe_input_ready
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
#include <bathos/nrf51-radio.h>
#include <bathos/allocator.h>
#include <bathos/bitops.h>


#define RADIO_TASK_TXEN		0x000
#define RADIO_TASK_RXEN		0x004
#define RADIO_TASK_START	0x008
#define RADIO_TASK_STOP		0x00c
#define RADIO_TASK_DISABLE	0x010
#define RADIO_TASK_RSSISTART	0x014
#define RADIO_TASK_RSSISTOP	0x018
#define RADIO_TASK_BCSTART	0x01c
#define RADIO_TASK_BCSTOP	0x020

#define RADIO_EVENT_READY	0x100
#define RADIO_EVENT_ADDRESS	0x104
#define RADIO_EVENT_PAYLOAD	0x108
#define RADIO_EVENT_END		0x10c
#define RADIO_EVENT_DISABLED	0x110
#define RADIO_EVENT_DEVMATCH	0x114
#define RADIO_EVENT_DEVMISS	0x118
#define RADIO_EVENT_RSSIEND	0x11c
#define RADIO_EVENT_BCMATCH	0x128

#define RADIO_REG_SHORTS	0x200
#define RADIO_REG_INTENSET	0x304
#define RADIO_REG_INTENCLR	0x308
#define RADIO_REG_CRCSTATUS	0x400
#define RADIO_REG_RXMATCH	0x408
#define RADIO_REG_RXCRC		0x40c
#define RADIO_REG_DAI		0x410
#define RADIO_REG_PACKETPTR	0x504
#define RADIO_REG_FREQUENCY	0x508
#define RADIO_REG_TXPOWER	0x50c
#define RADIO_REG_MODE		0x510
#define RADIO_REG_PCNF0		0x514
#define RADIO_REG_PCNF1		0x518
#define RADIO_REG_BASE(a)	(0x51c + (a)*4)
/* a can range from 0 to 7 */
#define RADIO_REG_PREFIX(a)	(0x524 + ((a)/4) * 4)
#define RADIO_REG_TXADDRESS	0x52c
#define RADIO_REG_RXADDRESSES	0x530
#define RADIO_REG_CRCCNF	0x534
#define RADIO_REG_CRCPOLY	0x538
#define RADIO_REG_CRCINIT	0x53c
#define RADIO_REG_RSSISAMPLE	0x548
#define RADIO_REG_STATE		0x550
#define RADIO_REG_BCC		0x560
#define RADIO_REG_DAB(a)	(0x600 + (a) * 4)
#define RADIO_REG_DAP(a)	(0x620 + (a) * 4)
#define RADIO_REG_DACNF		0x640
#define RADIO_REG_POWER		0xffc

struct nrf51_radio_priv {
	struct bathos_dev_data *dev_data;
	int tx_enabled;
	const struct nrf51_radio_platform_data *platform_data;
};

static int nrf51_radio_rx_enable(void *_priv)
{
	struct nrf51_radio_priv *priv = (struct nrf51_radio_priv*)_priv;
	const struct nrf51_radio_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;

	writel((uint32_t)plat->packet_area, base + RADIO_REG_PACKETPTR);
	writel(1, base + RADIO_TASK_RXEN);
	bathos_enable_irq(plat->irq);
	return 0;
}

static int nrf51_radio_tx_enable(void *_priv)
{
	struct nrf51_radio_priv *priv = (struct nrf51_radio_priv *)_priv;

	/* Software tx enable, hw enable is actually done on tx start */
	priv->tx_enabled = 1;
	return 0;
}

static int nrf51_radio_rx_disable(void *_priv)
{
	struct nrf51_radio_priv *priv = (struct nrf51_radio_priv*)_priv;
	const struct nrf51_radio_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;

	bathos_disable_irq(plat->irq);
	writel(0, base + RADIO_TASK_RXEN);
	return 0;
}

static int nrf51_radio_tx_disable(void *_priv)
{
	struct nrf51_radio_priv *priv = (struct nrf51_radio_priv*)_priv;

	priv->tx_enabled = 0;
	return 0;
}

static int nrf51_radio_write(void *_priv, const char *buf, int len)
{
	struct nrf51_radio_priv *priv = _priv;
	const struct nrf51_radio_platform_data *plat = priv->platform_data;
	uint32_t base = plat->base;
	
	int l;

	if (!priv->tx_enabled)
		return -EPERM;

	l = min(len, plat->packet_size);

	/* Copy buffer and setup packet pointer */
	memcpy(plat->packet_area->payload, buf, l);
	writel((uint32_t)plat->packet_area, base + RADIO_REG_PACKETPTR);

	/* Setup length */
	plat->packet_area->len = l;

	/* Enable tx */
	writel(1, base + RADIO_TASK_TXEN);

	/* Disable interrupt on packet end */
	writel(BIT(3), base + RADIO_REG_INTENCLR);

	/* Wait for tx done */
	while (!readl(base + RADIO_EVENT_END));

	/* Clear event */
	writel(0, base + RADIO_EVENT_END);

	/*
	 * Re-enable rx, tx should already have been disabled because of
	 * the shortcut
	 */
	writel(1, base + RADIO_TASK_RXEN);

	return l;
}

void nrf51_radio_irq_handler(struct bathos_dev *dev)
{
	const struct nrf51_radio_platform_data *plat = dev->platform_data;
	uint32_t base = plat->base;

	/* Clear END event */
	writel(0, base + RADIO_EVENT_END);
	
	(void)bathos_dev_push_chars(dev, plat->packet_area->payload,
				    plat->packet_area->len);
}

const struct bathos_ll_dev_ops nrf51_ll_radio_dev_ops = {
	/* We have our own write() */
	.putc = NULL,
	.write = nrf51_radio_write,
	.rx_disable = nrf51_radio_rx_disable,
	.rx_enable = nrf51_radio_rx_enable,
	.tx_disable = nrf51_radio_tx_disable,
	.tx_enable = nrf51_radio_tx_enable,
};

static int nrf51_radio_init(struct bathos_dev *dev)
{
	const struct nrf51_radio_platform_data *plat = dev->platform_data;
	struct nrf51_radio_priv *priv;
	int i;
	uint32_t base, tmp;

	if (!plat)
		return -EINVAL;

	if (plat->addr_length < 1 || plat->addr_length > 5)
		return -EINVAL;


	priv = bathos_alloc_buffer(sizeof(*priv));
	if (!priv)
		return -ENOMEM;

	priv->platform_data = plat;
	priv->tx_enabled = 0;
	base = plat->base;

	priv->dev_data = bathos_dev_init(&nrf51_ll_radio_dev_ops, priv);
	if (!priv->dev_data) {
		bathos_free_buffer(priv, sizeof(*priv));
		return -ENOMEM;
	}
	dev->priv = priv->dev_data;

	/* 
	   Power on peripheral, setup registers, do not enable rx,
	   that will be done later on
	*/
	writel(1, base + RADIO_REG_POWER);
	/*
	  We just use length (8 bits on air),
	  no need for S0 and S1.
	*/
	writel(8, base + RADIO_REG_PCNF0);
	/* Setup shortcuts ready/start and end/disable */
	writel(3, base + RADIO_REG_SHORTS);
	/* Setup address 0 */
	writel(plat->my_addr[0], base + RADIO_REG_PREFIX(0));
	for (i = 1, tmp = 0; i < plat->addr_length; i++)
		tmp |= plat->my_addr[i] << (i - 1);
	writel(tmp, base + RADIO_REG_BASE(0));	
	/* Setup address 1 */
	writel(plat->dst_addr[0], base + RADIO_REG_PREFIX(1));
	for (i = 1, tmp = 0; i < plat->addr_length; i++)
		tmp |= plat->dst_addr[i] << (i - 1);
	writel(tmp, base + RADIO_REG_BASE(1));	
	/* Select logical address 0 for rx */
	writel(BIT(0), base + RADIO_REG_RXADDRESSES);
	/* And logical address 1 for tx */
	writel(1, base + RADIO_REG_TXADDRESS);
	/* Enable interrupt on packet end */
	writel(BIT(3), base + RADIO_REG_INTENSET);
	return 0;
}


static int nrf51_radio_open(struct bathos_pipe *pipe)
{
	int stat = 0;

	stat = nrf51_radio_init(pipe->dev);
	if (stat)
		return stat;
	return bathos_dev_open(pipe);
}

const struct bathos_dev_ops nrf51_radio_dev_ops = {
	.open = nrf51_radio_open,
	.read = bathos_dev_read,
	.write = bathos_dev_write,
	.close = bathos_dev_close,
	.ioctl = bathos_dev_ioctl,
};
