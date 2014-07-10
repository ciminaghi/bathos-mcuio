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

enum bathos_dev_mode {
	CIRC_BUF,
	PACKET,
};


/*
 * struct dev_ioc_set_bqueue_data
 *
 * Data for DEV_IOC_RX_SET_BQUEUE_MODE
 *
 * @nbufs: number of buffers in queue: __must__ be a power of 2
 * @bufsize: size of a single buffer
 * @bufs: pointer to array of buffer pointers
 */
struct dev_ioc_set_bqueue_data {
	int nbufs;
	int bufsize;
	char **bufs;
};

/*
 * struct dev_ioc_set_rx_sync_seq_data
 *
 * Data for DEV_IOC_SET_RX_SYNC_SEQ
 *
 * @seq: pointer to sequence
 * @seqsize: size of sequence in bytes
 */
struct dev_ioc_set_rx_sync_seq_data {
	const char *seq;
	int seqsize;
};

/*
 * ioctl codes
 */
/* set rx high watermark */
#define DEV_IOC_RX_SET_HIGH_WATERMARK	1
/* set circular buffer mode (1 arg: buffer size) */
#define DEV_IOC_RX_SET_CBUF_MODE	2
/* release current circular buffer and set another one (1 arg: pointer to
   buffer)
*/
#define DEV_IOC_RX_SET_CIRC_BUF		3
/* set packet (buffer queue) mode (1 arg: pointer to dev_ioc_set_bqueue_data) */
#define DEV_IOC_RX_SET_BQUEUE_MODE	4
/* dequeue buffer (1 arg: address of output buffer pointer) */
#define DEV_IOC_RX_DEQUEUE_BUFFER	5
/* peek buffer (1 arg: address of output buffer pointer) */
#define DEV_IOC_RX_PEEK_BUFFER		6
/* enable rx (0 args) */
#define DEV_IOC_RX_ENABLE		7
/* set sync sequence for packet mode */
#define DEV_IOC_SET_RX_SYNC_SEQ		8
/* set sync silence time for packet mode (1 arg: silence time in jiffies) */
#define DEV_IOC_SET_RX_SYNC_SIL_TIME	9

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

