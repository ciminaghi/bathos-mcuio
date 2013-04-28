#ifndef __BATHOS_ENC28J60_H__
#define __BATHOS_ENC28J60_H__

#include <bathos/types.h>
#include <bathos/io.h>
#include <bathos/spi.h>

/* The ENC28 device instance is configured like this */
struct enc28_cfg {
	u8 ipaddr[4];
	u8 macaddr[6]; /* ETH_ALEN */
};

/* This should be opaque, but we have no malloc so the caller passes it in */
struct enc28_dev {
	const struct enc28_cfg	*cfg;
	struct spi_dev		*spi_dev;
	u16	bank;
	u16	txptr;
	u16	rxptr;
	u16	nextpkt;
};

extern struct enc28_dev *enc28_create(struct enc28_dev *dev);
extern void enc28_destroy(struct enc28_dev *dev);
extern int enc28_get_rev(struct enc28_dev *dev);
extern int enc28_send(struct enc28_dev *dev, const u8 *frame, int len);
extern int enc28_recv(struct enc28_dev *dev, u8 *frame, int maxlen);

#endif /* __BATHOS_ENC28J60_H__ */
