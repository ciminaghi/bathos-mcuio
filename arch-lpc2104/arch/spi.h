/*
 * SPI interface for LPC-2104
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#ifndef __LPC21_SPI_H__
#define __LPC21_SPI_H__

#include <bathos/types.h>

/* This is an interface for SPI, only master is supported */
struct spi_cfg {
	int gpio_cs;
	int freq;		/* Suggested frequency */
	u8 pol, phase, devn;	/* devn selects spi0 or spi1 or more */
};

/* Data transfers include read and write, either or both */
struct spi_ibuf {
	int len;
	u8 *buf;
};

struct spi_obuf {
	int len;
	const u8 *buf;
};

/* Usually a data transfer is a single buffer, but we may need to chain more */
enum spi_flags {
    SPI_F_DEFAULT = 0,
    SPI_F_NOINIT = 1,	/* Don't lower CS (and don't take the lock) */
    SPI_F_NOFINI = 2,	/* Don't raise CS (and don't release the lock) */
    SPI_F_NOCS = 3,	/* Dont' act on CS at all (nor on the lock) */
};

/* This would be opaque if we had malloc, but we miss it by now */
struct spi_dev {
	const	struct spi_cfg *cfg;
	int	current_freq;
};

extern struct spi_dev *spi_create(struct spi_dev *cfg);
extern void spi_destroy(struct spi_dev *dev);
extern int spi_xfer(struct spi_dev *dev,
		    enum spi_flags flags,
		    const struct spi_ibuf *ibuf,
		    const struct spi_obuf *obuf);

/* Sometimes we want to only read or only write */
static inline int spi_read(struct spi_dev *dev,
			   enum spi_flags flags,
			   const struct spi_ibuf *ibuf)
{
	return spi_xfer(dev, flags, ibuf, NULL);
}

static inline int spi_write(struct spi_dev *dev, enum spi_flags flags,
			    const struct spi_obuf *obuf)
{
	return spi_xfer(dev, flags, NULL, obuf);
}

/* And sometimes we want raw-read  and write (with no buffer) */
static inline int spi_raw_read(struct spi_dev *dev, enum spi_flags flags,
			       int len, u8 *buf)
{
	struct spi_ibuf b = {len, buf};
	return spi_read(dev, flags, &b);
}

static inline int spi_raw_write(struct spi_dev *dev, enum spi_flags flags,
				int len, const u8 *buf)
{
	const struct spi_obuf b = {len, buf};
	return spi_write(dev, flags, &b);
}

/* SPI (Serial Peripheral Interface) */
#define REG_SPCR	(0xE0020000 / 4)
#define REG_SPSR	(0xE0020004 / 4)
#define REG_SPDR	(0xE0020008 / 4)
#define REG_SPCCR	(0xE002000C / 4)
#define REG_SPTCR	(0xE0020010 / 4)
#define REG_SPTSR	(0xE0020014 / 4)
#define REG_SPTOR	(0xE0020018 / 4)
#define REG_SPINT	(0xE002001C / 4)

#define REG_S0SPCR	(0xE0020000 / 4)
#define REG_S0SPSR 	(0xE0020004 / 4)
#define REG_S0SPDR 	(0xE0020008 / 4)
#define REG_S0SPCCR	(0xE002000C / 4)
#define REG_S0SPTCR	(0xE0020010 / 4)
#define REG_S0SPTSR	(0xE0020014 / 4)
#define REG_S0SPTOR	(0xE0020018 / 4)
#define REG_S0SPINT	(0xE002001C / 4)

/* SPI1 (Serial Peripheral Interface 1) */
#define REG_S1SPCR 	(0xE0030000 / 4)
#define REG_S1SPSR 	(0xE0030004 / 4)
#define REG_S1SPDR 	(0xE0030008 / 4)
#define REG_S1SPCCR	(0xE003000C / 4)
#define REG_S1SPTCR	(0xE0030010 / 4)
#define REG_S1SPTSR	(0xE0030014 / 4)
#define REG_S1SPTOR	(0xE0030018 / 4)
#define REG_S1SPINT	(0xE003001C / 4)

/* SSP Controller */
#define REG_SSPCR0 	(0xE0068000 / 4)
#define REG_SSPCR1 	(0xE0068004 / 4)
#define REG_SSPDR  	(0xE0068008 / 4)
#define REG_SSPSR  	(0xE006800C / 4)
#define REG_SSPCPSR	(0xE0068010 / 4)
#define REG_SSPIMSC	(0xE0068014 / 4)
#define REG_SSPRIS	(0xE0068018 / 4)
#define REG_SSPMIS 	(0xE006801C / 4)
#define REG_SSPICR 	(0xE0068020 / 4)

#endif /* __LPC21_SPI_H__ */
