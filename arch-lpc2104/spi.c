#include <ssw/kernel.h>
#include <ssw/malloc.h>
#include <ssw/types.h>
#include <ssw/module.h>
#include <ssw/spi.h>
#include <ssw/gpio.h>
#include <ssw/io.h>
#include <ssw/delay.h>
#include <ssw/debug.h>
#include <hw/lpc2xxx.h>
#include <mach/spi.h>

#ifndef DEBUG_SPI
#define DEBUG_SPI 0
#endif

#define SPI_CS_DELAY 10

struct spi_dev {
	const	struct spi_cfg *cfg;
	int	current_freq;
};

struct spi_dev *lpc2xxx_spi_create(const struct spi_cfg *cfg)
{
	struct spi_dev *dev;
	u32 spcr;
	int i;

	if (DEBUG_SPI)
		printk("%s: gpio is %i\n", __func__, cfg->gpio_cs);

	dev = malloc(sizeof(*dev));
	if (!dev) return dev;

	dev->cfg = cfg;
	dev->current_freq = 0;

	if (cfg->devn > 0) /* SPI1 on 2103 is not supported */
		return NULL;

	/* FIXME: no support for two devices on different GPIO */

	/* Configure the GPIO pins (pins 4..7 AF1) */
	for (i = 4; i < 8; i++)
		gpio_dir_af(i, SSW_GPIO_IN, SSW_GPIO_AF_1);

	/* Configure the CS GPIO */
	gpio_set(cfg->gpio_cs, 1);
	gpio_dir_af(cfg->gpio_cs, SSW_GPIO_OUT, SSW_GPIO_AF_GPIO);

	/* No interrupts and MSB-first */
	spcr = (cfg->phase << 3) | (cfg->pol << 4) | (1<<5 /* master */);
	SPI_SPCR = spcr;

	/* FIXME: frequency support is missing */
	SPI_SPCCR = 128; /* 8 is the minimum, start slow (FIXME) */

	/* Clear any pending "complete flag" */
	i = SPI_SPSR; i = SPI_SPDR;

	return dev;
}

void lpc2xxx_spi_destroy(struct spi_dev *dev)
{
	int i;

	/* De-configure the GPIO pins (pins 4..7 AF1) */
	for (i = 4; i < 8; i++)
		gpio_dir_af(i, 0, 1);

	free(dev);
}

/* Local functions to simplify xfer code */
static void __spi_wait_busy(void)
{
	while( !(SPI_SPSR & 0x80))
		/* FIXME: timeout */;
}

static void __spi_cs(struct spi_dev *dev, int value)
{
	gpio_set(dev->cfg->gpio_cs, value);
}

int lpc2xxx_spi_xfer(struct spi_dev *dev,
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
		printk("%s: %s", __func__, flags & SPI_F_NOINIT ? "--" : "cs");

	if ( !(flags & SPI_F_NOINIT) ) {
		udelay(SPI_CS_DELAY);
		__spi_cs(dev, 0);
		udelay(SPI_CS_DELAY);
	}
	for (i = 0; i < len; i++) {
		SPI_SPDR = obuf ? obuf->buf[i] : 0xff;
		__spi_wait_busy();
		val = SPI_SPDR;
		if (DEBUG_SPI)
			printk(" %02x(%02x)", obuf->buf[i], val);
		if (ibuf)
			ibuf->buf[i] = val;
	}

	if (DEBUG_SPI)
		printk(" %s\n", flags & SPI_F_NOFINI ? "--" : "cs");

	if ( !(flags & SPI_F_NOFINI) ) {
		udelay(SPI_CS_DELAY);
		__spi_cs(dev, 1);
		udelay(SPI_CS_DELAY);
	}
	return 0;
}

module_provide(spi_21xxx);
