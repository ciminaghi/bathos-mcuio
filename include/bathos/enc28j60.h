#ifndef __SSW_ENC28J60_H__
#define __SSW_ENC28J60_H__

#include <ssw/types.h>
#include <ssw/io.h>
#include <ssw/spi.h>

/* The ENC28 device instance is configured like this */
struct enc28_cfg {
	const struct spi_cfg *spi_cfg;
	u8 ipaddr[4];
	u8 macaddr[6]; /* ETH_ALEN */
};

struct enc28_dev; /* Opaque device structure */

extern struct enc28_dev *enc28_create(const struct enc28_cfg *cfg);
extern void enc28_destroy(struct enc28_dev *dev);
extern int enc28_get_rev(struct enc28_dev *dev);
extern int enc28_send(struct enc28_dev *dev, const u8 *packet, int len);
extern int enc28_recv(struct enc28_dev *dev, u8 *packet, int maxlen);

#endif /* __SSW_ENC28J60_H__ */
