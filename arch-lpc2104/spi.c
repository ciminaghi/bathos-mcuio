/*
 * SPI interface for LPC-2104
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/types.h>
#include <bathos/delay.h>
#include <bathos/spi.h>
#include <arch/gpio.h>
#include <bathos/io.h>
#include <arch/hw.h>

#ifndef DEBUG_SPI
#define DEBUG_SPI 0
#endif

#define SPI_CS_DELAY 10

struct spi_dev *spi_create(struct spi_dev *dev)
{
	const struct spi_cfg *cfg = dev->cfg;
	u32 spcr;
	int i;

	if (DEBUG_SPI)
		printf("%s: gpio is %i\n", __func__, cfg->gpio_cs);

	dev->current_freq = 0;

	if (cfg->devn > 0) /* SPI1 on 2103 is not supported */
		return NULL;

	/* FIXME: no support for two devices on different GPIO */

	/* Configure the GPIO pins (pins 4..7 AF1) */
	for (i = 4; i < 8; i++)
		gpio_dir_af(i, 0, 0, 1);

	/* Configure the CS GPIO */
	gpio_set(cfg->gpio_cs, 1);
	gpio_dir_af(cfg->gpio_cs, 1, 1, 0);

	/* No interrupts and MSB-first */
	spcr = (cfg->phase << 3) | (cfg->pol << 4) | (1<<5 /* master */);
	regs[REG_SPCR] = spcr;

	/* FIXME: frequency support is missing */
	regs[REG_SPCCR] = 16; /* 8 is the minimum, start slow (FIXME) */

	/* Clear any pending "complete flag" */
	i = regs[REG_SPSR]; i = regs[REG_SPDR];

	return dev;
}

void spi_destroy(struct spi_dev *dev)
{
	int i;

	/* De-configure the GPIO pins (pins 4..7 AF1) */
	for (i = 4; i < 8; i++)
		gpio_dir_af(i, 0, 0, 1);

	dev->cfg = NULL;
	dev->current_freq = 0;
}

/* Local functions to simplify xfer code */
static void __spi_wait_busy(void)
{
	while( !(regs[REG_SPSR] & 0x80))
		/* FIXME: timeout */;
}

static void __spi_cs(struct spi_dev *dev, int value)
{
	gpio_set(dev->cfg->gpio_cs, value);
}

int spi_xfer(struct spi_dev *dev,
		     enum spi_flags flags,
		     const struct spi_ibuf *ibuf,
		     const struct spi_obuf *obuf)
{
	int i, len;
	u8 val;

	/* if it's both input and output, lenght must be the same */
	if (ibuf && obuf) {
		if (ibuf->len != ibuf->len)
			return -1; /* EINVAL ... */
	}
	if (!ibuf && !obuf)
		return 0; /* nothing to do */

	len = ibuf ? ibuf->len : obuf->len;


	if (DEBUG_SPI)
		printf("%s: %s", __func__, flags & SPI_F_NOINIT ? "--" : "cs");

	if ( !(flags & SPI_F_NOINIT) ) {
		udelay(SPI_CS_DELAY);
		__spi_cs(dev, 0);
		udelay(SPI_CS_DELAY);
	}
	for (i = 0; i < len; i++) {
		regs[REG_SPDR] = obuf ? obuf->buf[i] : 0xff;
		__spi_wait_busy();
		val = regs[REG_SPDR];
		if (DEBUG_SPI)
			printf(" %02x(%02x)", obuf ? obuf->buf[i] : 0xff, val);
		if (ibuf)
			ibuf->buf[i] = val;
	}

	if (DEBUG_SPI)
		printf(" %s\n", flags & SPI_F_NOFINI ? "--" : "cs");

	if ( !(flags & SPI_F_NOFINI) ) {
		udelay(SPI_CS_DELAY);
		__spi_cs(dev, 1);
		udelay(SPI_CS_DELAY);
	}
	return 0;
}
