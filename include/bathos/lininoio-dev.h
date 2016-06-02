#ifndef __LININOIO_DEV_H__
#define __LININOIO_DEV_H__
/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

extern int lininoio_dev_input(const struct lininoio_channel *ch,
			      void *buf, uint16_t len);

declare_extern_event(lininoio_dev_chan_connected);

#endif /* __LININOIO_DEV_H__ */

