#ifndef __SSW_SPI_H__
#define __SSW_SPI_H__

#include <ssw/types.h>
#include <ssw/io.h>
#include <mach/spi.h> /* This may rename some functions */

/* This is an interface for ssw SPI, only master is supported */
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

struct spi_dev; /* Opaque device structure */

extern struct spi_dev *spi_create(const struct spi_cfg *cfg);
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

#endif /* __SSW_SPI_H__ */
