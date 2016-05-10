/*
 * Copyright 2011 Dog Hunter SA
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 *
 * GNU GPLv2 or later
 */
/* Common operations for bathos devices */

#ifndef __BATHOS_DEV_OPS_H__
#define __BATHOS_DEV_OPS_H__

#include <bathos/pipe.h>
#include <arch/bathos-arch.h>

/*
 * ioctl codes
 */
/* set rx high watermark */
#define DEV_IOC_RX_SET_HIGH_WATERMARK	1
/* enable rx (0 args) */
#define DEV_IOC_RX_ENABLE		2

/* Start of reserved device-specific LL ioctls */
#define DEV_IOC_PRIVATE                 0x1000

/*
 * Low level device operations
 *
 * @putc: transmit single char
 * @write: transmit buffer
 * @set_bufsize: set buffer size hook
 * @rx_disable: low level rx disable
 * @rx_enable: low level rx enable
 * @tx_disable: low level tx disable
 * @tx_enable: low level tx enable
 * @ioctl: low level ioctl
 *
 */
struct bathos_ll_dev_ops {
	int (*putc)(void *priv, const char);
	int (*write)(void *priv, const char *, int len);
	int (*set_bufsize)(void *priv, int bufsize);
	int (*rx_disable)(void *priv);
	int (*rx_enable)(void *priv);
	int (*tx_disable)(void *priv);
	int (*tx_enable)(void *priv);
	int (*ioctl)(void *priv, struct bathos_ioctl_data *data);
};

struct bathos_dev_data;

/*
 * bathos_dev_init()
 *
 * Initialize device
 *
 * @priv: low level private data (passed as first arg of low level ops)
 *
 * Returns opaque pointer to be passed to bathos_dev_push_chars() and other
 * high level device functions
 */
struct bathos_dev_data *
bathos_dev_init(const struct bathos_ll_dev_ops * PROGMEM ops, void *priv);

/*
 * bathos_dev_push_chars()
 *
 * To be invoked when chars have been received
 *
 * @dev: pointer to corresponding bathos device
 * @buf: pointer to buffer containing received chars
 * @len: length of @buf
 *
 * Returns 0 on success, errno on error.
 */
int bathos_dev_push_chars(struct bathos_dev *dev, const char *buf, int len);

/*
 * The following functions can be used as device methods for a generic device
 */
int bathos_dev_open(struct bathos_pipe *pipe);
int bathos_dev_read(struct bathos_pipe *pipe, char *buf, int len);
int bathos_dev_write(struct bathos_pipe *pipe, const char *buf, int len);
int bathos_dev_close(struct bathos_pipe *pipe);
int bathos_dev_ioctl(struct bathos_pipe *pipe, struct bathos_ioctl_data *data);

		
#endif /* __BATHOS_DEV_OPS_H__ */

