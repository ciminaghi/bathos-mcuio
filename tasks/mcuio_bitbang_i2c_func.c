/*
 * bitbang i2c mcuio function, some code taken from
 * drivers/i2c/algos/i2c-algo-bit.c
 */
#include <arch/hw.h>
#include <bathos/stdio.h>
#include <bathos/gpio.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <bathos/jiffies.h>
#include <bathos/delay.h>
#include <bathos/circ_buf.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

extern struct mcuio_function mcuio_bitbang_i2c;
declare_extern_event(mcuio_irq);

enum i2c_transaction_state {
	IDLE,
	START_SENT,
	ADDR_SENT,
	SENDING_DATA,
	AWAITING_OUTPUT_DATA,
	DATA_SENT,
	REPEATED_START_SENT,
	ADDR_SENT_2,
	RECEIVING_DATA,
	AWAITING_INPUT_SPACE,
	DATA_RECEIVED,
	ERROR,
};

enum i2c_event {
	I2C_GO,
	I2C_RESET,
};

#define OBUF_LOW_WATERMARK 4
#define IBUF_HI_WATERMARK  (32 - 4)

static struct mcuio_i2c_data {
	int udelay;
	int timeout;
	uint16_t slave_address;
	uint16_t buf_len;
	uint16_t data_cnt;
	uint8_t  status;
	uint8_t  int_enable;
	uint8_t buffer[32];
	uint8_t ibuf_head;
	uint8_t ibuf_tail;
	uint8_t obuf_head;
	uint8_t obuf_tail;
	enum i2c_transaction_state state;
} i2c_data;

#define MCUIO_CLASS_I2C_CONTROLLER 0x8

#if defined CONFIG_MCUIO_BITBANG_I2C_SDA
# define GPIO_SDA CONFIG_MCUIO_BITBANG_I2C_SDA
#else
# define GPIO_SDA 6
#endif

#if defined CONFIG_MCUIO_BITBANG_I2C_SCL
# define GPIO_SCL CONFIG_MCUIO_BITBANG_I2C_SCL
#else
# define GPIO_SCL 7
#endif

#define I2C_MCUIO_SADDR	   0x008
#define I2C_MCUIO_STATUS   0x00c
#define	  TRANSACTION_OK       0x1
#define	  BUSY		       0x2
#define	  NAK_RECEIVED	       0x80
#define I2C_MCUIO_CFG	   0x010
#define I2C_MCUIO_BRATE	   0x014
#define I2C_MCUIO_CMD	   0x018
#define	  START_TRANSACTION 0x1

#define I2C_MCUIO_INTEN	   0x01c
#define I2C_MCUIO_BUF_SIZE 0x020
#define I2C_MCUIO_OBUF_LEN 0x024
#define I2C_MCUIO_IBUF_LEN 0x028
#define I2C_MCUIO_OBUF_HEAD 0x02c
#define I2C_MCUIO_OBUF_TAIL 0x030
#define I2C_MCUIO_IBUF_HEAD 0x034
#define I2C_MCUIO_IBUF_TAIL 0x038
#define I2C_MCUIO_OBUF	   0x040
#define I2C_MCUIO_IBUF	   (I2C_MCUIO_OBUF + I2C_MCUIO_OBUF_MAX_SIZE)

static const struct mcuio_func_descriptor PROGMEM i2c_bitbang_descr = {
	.device = CONFIG_MCUIO_BITBANG_I2C_DEVICE,
	.vendor = CONFIG_MCUIO_BITBANG_I2C_VENDOR,
	.rev = 0,
	/* I2C controller class */
	.class = MCUIO_CLASS_I2C_CONTROLLER,
};

static const unsigned int PROGMEM i2c_bitbang_descr_length =
    sizeof(i2c_bitbang_descr);

static const unsigned int PROGMEM i2c_bitbang_registers_length =
	I2C_MCUIO_IBUF_TAIL + sizeof(uint32_t) - I2C_MCUIO_SADDR;

static const unsigned int PROGMEM i2c_bitbang_obuf_length =
	sizeof(i2c_data.buffer);

static const unsigned int PROGMEM i2c_bitbang_ibuf_length =
	sizeof(i2c_data.buffer);

declare_event(i2c_transaction);

static inline int is_i2c_write(void)
{
	return !(i2c_data.slave_address & 1);
}

static inline void setsda(int v)
{
	gpio_dir(GPIO_SDA, 1, v);
	if (v)
		gpio_dir(GPIO_SDA, 0, 1);
}

static inline void setscl(int v)
{
	gpio_dir(GPIO_SCL, 1, v);
	if (v)
		gpio_dir(GPIO_SCL, 0, 1);
}

static inline int getscl(void)
{
	return gpio_get(GPIO_SCL);
}

static inline int getsda(void)
{
	int ret = gpio_get(GPIO_SDA);
	return ret;
}

static inline void sdalo(void)
{
	setsda(0);
	udelay((i2c_data.udelay + 1) / 2);
}

static inline void sdahi(void)
{
	setsda(1);
	udelay((i2c_data.udelay + 1) / 2);
}

static inline void scllo(void)
{
	setscl(0);
	udelay(i2c_data.udelay / 2);
}

static inline int sclhi(void)
{
	unsigned long start;
	setscl(1);
	start = jiffies;
	while (!getscl()) {
		/* Wait for slave to leave scl (clock stretching) */
		if (time_after(jiffies, start + i2c_data.timeout))
			return -ETIMEDOUT;
	}
	udelay(i2c_data.udelay);
	return 0;
}


static void __i2c_bitbang_send_start(void)
{
	setsda(0);
	udelay(i2c_data.udelay);
	scllo();
}

static void __i2c_bitbang_send_repstart(void)
{
	sdahi();
	sclhi();
	setsda(0);
	udelay(i2c_data.udelay);
	scllo();
}

static void __i2c_bitbang_send_stop(void)
{
	sdalo();
	sclhi();
	setsda(1);
	udelay(i2c_data.udelay);
}

static int __i2c_bitbang_send_byte(uint8_t c)
{
	int i;
	int sb;
	int ack;

	/* assert: scl is low */
	for (i = 7; i >= 0; i--) {
		sb = (c >> i) & 1;
		setsda(sb);
		udelay((i2c_data.udelay + 1) / 2);
		if (sclhi() < 0)
			return -ETIMEDOUT;
		scllo();
	}
	sdahi();
	if (sclhi() < 0)
		return -ETIMEDOUT;

	/* read ack: SDA should be pulled down by slave, or it may
	 * NAK (usually to report problems with the data we wrote).
	 */
	ack = !getsda();	/* ack: sda is pulled low -> success */
	scllo();
	return ack ? 0 : -1;
	/* assert: scl is low (sda undef) */
}

static int __i2c_bitbang_recv_byte(uint8_t *out)
{
	/* read byte via i2c port, without start/stop sequence	*/
	/* acknowledge is sent in i2c_read.			*/
	int i;
	*out = 0;

	/* assert: scl is low */
	sdahi();
	for (i = 0; i < 8; i++) {
		if (sclhi() < 0)
			return -ETIMEDOUT;
		*out *= 2;
		if (getsda())
			*out |= 0x01;
		setscl(0);
		udelay(i == 7 ? i2c_data.udelay / 2 : i2c_data.udelay);
	}
	/* assert: scl is low */
	return 0;
}

static int __i2c_bitbang_send_slave_addr(int force_rd)
{
	u8 a = i2c_data.slave_address;
	return __i2c_bitbang_send_byte(a | (force_rd ? 1 : 0));
}

static int __i2c_bitbang_send_acknak(int is_ack)
{
	if (is_ack)
		setsda(0);
	udelay((i2c_data.udelay + 1) / 2);
	if (sclhi() < 0)
		return -ETIMEDOUT;
	scllo();
	return 0;
}

static void __i2c_bitbang_trig_evt_error(void)
{
	printf("__i2c_bitbang_trig_evt_error\n");
}

static void __i2c_bitbang_next_state(enum i2c_transaction_state s)
{
	int stat;
	i2c_data.state = s;
	if (s == IDLE || s == AWAITING_OUTPUT_DATA ||
	    s == AWAITING_INPUT_SPACE)
		return;
	stat = trigger_event(&event_name(i2c_transaction),
			     (void *)I2C_GO, EVT_PRIO_MAX);
	if (stat < 0)
		__i2c_bitbang_trig_evt_error();
}

static void __i2c_bitbang_trigger_irq(void)
{
	static struct mcuio_function_irq_data idata;
	int f;

	f = &mcuio_bitbang_i2c - mcuio_functions_start;
	idata.func = f;
	idata.active = 1;
	if (trigger_event(&event_name(mcuio_irq), &idata, EVT_PRIO_MAX))
		printf("__i2c_bitbang_trigger_irq: evt error\n");
}

static void __i2c_bitbang_end_transaction(uint8_t s)
{
	/* send stop */
	__i2c_bitbang_send_stop();

	i2c_data.status = s;
	i2c_data.state = s != TRANSACTION_OK ? ERROR : IDLE;
	__i2c_bitbang_trigger_irq();
}

static void __i2c_handle_reset(void)
{
	printf("rst s%d\n", i2c_data.state);
	i2c_data.status = 0;
	__i2c_bitbang_next_state(IDLE);
}

static void __i2c_handle_go(void)
{
	i2c_data.timeout = HZ;
	i2c_data.udelay = 100;
	switch (i2c_data.state) {
	case IDLE:
		/* Send start condition */
		__i2c_bitbang_send_start();
		/* -> START_SENT */
		__i2c_bitbang_next_state(START_SENT);
		break;
	case START_SENT:
		/* Send slave address + w bit */
		if (__i2c_bitbang_send_slave_addr(0) < 0) {
			__i2c_bitbang_end_transaction(NAK_RECEIVED);
			return;
		}
		/* -> ADDR_SENT */
		__i2c_bitbang_next_state(ADDR_SENT);
		break;
	case ADDR_SENT:
		if (!i2c_data.buf_len) {
			/* No data */
			__i2c_bitbang_next_state(DATA_SENT);
			break;
		}
		/* Reset byte counter */
		i2c_data.data_cnt = 0;
		/* -> SENDING_DATA */
		__i2c_bitbang_next_state(SENDING_DATA);
		break;
	case SENDING_DATA:
	{
		int done, count;
		uint8_t c = i2c_data.buffer[i2c_data.obuf_tail];
		/* Send a single byte */
		if (__i2c_bitbang_send_byte(c) < 0) {
			__i2c_bitbang_end_transaction(NAK_RECEIVED);
			return;
		}
		i2c_data.data_cnt++;
		i2c_data.obuf_tail = (i2c_data.obuf_tail + 1) &
			(sizeof(i2c_data.buffer) - 1);
		/* Set next state */
		done = i2c_data.data_cnt == i2c_data.buf_len;
		if (done) {
			/* equalize tail to head: the host always writes
			   dwords, maybe 1 to 3 bytes are in eccess
			*/
			i2c_data.obuf_tail = i2c_data.obuf_head;
			__i2c_bitbang_next_state(DATA_SENT);
			break;
		}
		count = CIRC_CNT(i2c_data.obuf_head, i2c_data.obuf_tail,
				 sizeof(i2c_data.buffer));
		if (count == OBUF_LOW_WATERMARK)
			/* Trigger out low watermark interrupt */
			__i2c_bitbang_trigger_irq();

		__i2c_bitbang_next_state(count ?
					 SENDING_DATA :
					 AWAITING_OUTPUT_DATA);
		break;
	}
	case DATA_SENT:
		if (is_i2c_write()) {
			__i2c_bitbang_end_transaction(TRANSACTION_OK);
			return;
		}
		/* Send repeated start */
		__i2c_bitbang_send_repstart();
		__i2c_bitbang_next_state(REPEATED_START_SENT);
		break;
	case REPEATED_START_SENT:
		/* Send slave address + r bit */
		if (__i2c_bitbang_send_slave_addr(1) < 0) {
			__i2c_bitbang_end_transaction(NAK_RECEIVED);
			return;
		}
		/* -> ADDR_SENT_2 */
		__i2c_bitbang_next_state(ADDR_SENT_2);
		break;
	case ADDR_SENT_2:
		/* Reset counter */
		i2c_data.data_cnt = 0;
		/* -> RECEIVING_DATA */
		__i2c_bitbang_next_state(RECEIVING_DATA);
		break;
	case RECEIVING_DATA:
	{
		int done, space;
		/* Read byte */
		__i2c_bitbang_recv_byte(&i2c_data.buffer[i2c_data.data_cnt++]);
		done = i2c_data.data_cnt == i2c_data.buf_len;
		__i2c_bitbang_send_acknak(!done);
		if (done) {
			__i2c_bitbang_next_state(DATA_RECEIVED);
			break;
		}
		space = CIRC_SPACE(i2c_data.ibuf_head,
				   i2c_data.ibuf_tail,
				   sizeof(i2c_data.buffer));
		if (space == IBUF_HI_WATERMARK)
			__i2c_bitbang_trigger_irq();

		__i2c_bitbang_next_state(space ?
					 RECEIVING_DATA :
					 AWAITING_INPUT_SPACE);
		break;
	}
	case DATA_RECEIVED:
		/* All done, end transaction */
		__i2c_bitbang_end_transaction(TRANSACTION_OK);
		break;
	case AWAITING_OUTPUT_DATA:
	case AWAITING_INPUT_SPACE:
	case ERROR:
		/* Error state, ignore go events */
		break;
	}
}

static void __i2c_bitbang_do_transaction(struct event_handler_data *ed)
{
	enum i2c_event evt = (enum i2c_event)ed->data;

	switch (evt) {
	case I2C_RESET:
		__i2c_handle_reset();
		break;
	case I2C_GO:
		__i2c_handle_go();
		break;
	default:
		printf("__i2c_bitbang_do_transaction ! %d\n", evt);
		break;
	}
}

declare_event_handler(i2c_transaction, NULL,
		      __i2c_bitbang_do_transaction, NULL);

static int __i2c_bitbang_start_transaction(void)
{
	printf("__i2c_bitbang_start_transaction %d s %d\n", i2c_data.status);
	if (i2c_data.status & BUSY)
		return -EBUSY;
	i2c_data.status |= BUSY;
	return trigger_event(&event_name(i2c_transaction), (void *)I2C_GO,
			     EVT_PRIO_MAX);
}

static int i2c_bitbang_registers_rddw(const struct mcuio_range *r,
				      unsigned offset,
				      uint32_t *__out, int fill)
{
	unsigned i, size = fill ? sizeof(uint64_t) : sizeof(uint32_t);
	uint32_t *out;
	static struct mcuio_function_irq_data idata;

	for (i = 0; i < size; i += sizeof(uint32_t)) {
		out = &__out[i / sizeof(uint32_t)];
		switch (i + offset + r->start) {
		case I2C_MCUIO_SADDR:
			/* Slave address */
			*out = (uint32_t)i2c_data.slave_address;
			break;
		case I2C_MCUIO_STATUS:
			/* Status */
			*out = (uint32_t)i2c_data.status;
			if (*out) {
				/* Automatically clear interrupt */
				int f = &mcuio_bitbang_i2c -
					mcuio_functions_start;
				idata.func = f;
				idata.active = 0;
				if (trigger_event(&event_name(mcuio_irq),
						  &idata, EVT_PRIO_MAX))
					printf("i2c_bitbang_registers_rddw: "
					       "evt error\n");
				if (i2c_data.state != AWAITING_OUTPUT_DATA &&
				    i2c_data.state != AWAITING_INPUT_SPACE) {
					trigger_event(&event_name(
							      i2c_transaction),
						      (void *)I2C_RESET,
						      EVT_PRIO_MAX);
				}
			}
			break;
		case I2C_MCUIO_CFG:
		case I2C_MCUIO_BRATE:
			/* unsupported at the moment */
			*out = 0;
			break;
		case I2C_MCUIO_CMD:
			/* command always reads back 0 */
			*out = 0;
			break;
		case I2C_MCUIO_INTEN:
			/* interrupt enable */
			*out = i2c_data.int_enable;
			break;
		case I2C_MCUIO_BUF_SIZE:
			/* Buffer size */
			*out = sizeof(i2c_data.buffer);
			break;
		case I2C_MCUIO_OBUF_HEAD:
			*out = i2c_data.obuf_head;
			break;
		case I2C_MCUIO_OBUF_TAIL:
			*out = i2c_data.obuf_tail;
			break;
		case I2C_MCUIO_IBUF_HEAD:
			*out = i2c_data.ibuf_head;
			break;
		case I2C_MCUIO_IBUF_TAIL:
			*out = i2c_data.ibuf_tail;
			break;
		case I2C_MCUIO_OBUF_LEN:
		case I2C_MCUIO_IBUF_LEN:
			*out = i2c_data.buf_len;
			break;			
		default:
			return -EPERM;
		}
	}
	return size;
}

static int i2c_bitbang_registers_wrdw(const struct mcuio_range *r,
				      unsigned offset,
				      const uint32_t *__in, int fill)
{
	unsigned i, size = fill ? sizeof(uint64_t) : sizeof(uint32_t);
	for (i = 0; i < size; i += sizeof(uint32_t)) {
		uint32_t in = __in[i / sizeof(uint32_t)];
		switch (i + offset + r->start) {
		case I2C_MCUIO_SADDR:
			/* Slave address */
			i2c_data.slave_address = in;
			break;
		case I2C_MCUIO_STATUS:
			return -EPERM;
		case I2C_MCUIO_CFG:
		case I2C_MCUIO_BRATE:
			/* unsupported at the moment */
			break;
		case I2C_MCUIO_CMD:
		{
			int stat;
			printf("i2c_bitbang_registers_wrdw c %u\n", in);
			if (in & START_TRANSACTION) {
				stat = __i2c_bitbang_start_transaction();
				if (stat < 0)
					return stat;
			}
			break;
		}
		case I2C_MCUIO_INTEN:
			/* interrupt enable */
			i2c_data.int_enable = in;
			break;
		case I2C_MCUIO_BUF_SIZE:
			/* Buffer size, cannot be written */
			return -EPERM;
		case I2C_MCUIO_OBUF_LEN:
		case I2C_MCUIO_IBUF_LEN:
			i2c_data.buf_len = in;
			break;
		case I2C_MCUIO_OBUF_HEAD:
			i2c_data.obuf_head = in;
			if (i2c_data.state == AWAITING_OUTPUT_DATA)
				__i2c_bitbang_next_state(SENDING_DATA);
			break;
		case I2C_MCUIO_IBUF_TAIL:
			i2c_data.ibuf_tail = in;
			if (i2c_data.state == AWAITING_INPUT_SPACE)
				__i2c_bitbang_next_state(RECEIVING_DATA);
			break;
		default:
			return -EPERM;
		}
	}
	return size;
}


const struct mcuio_range_ops PROGMEM i2c_bitbang_registers_rw_ops = {
	.rd = { NULL, NULL, i2c_bitbang_registers_rddw, NULL, },
	.wr = { NULL, NULL, i2c_bitbang_registers_wrdw, NULL, },
};

static const struct mcuio_range PROGMEM i2c_bitbang_ranges[] = {
	/* i2c bitbang func descriptor */
	{
		.start = 0,
		.length = &i2c_bitbang_descr_length,
		.rd_target = &i2c_bitbang_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords 0x8 .. 0x2b: registers, read/write */
	{
		.start = 8,
		.length = &i2c_bitbang_registers_length,
		.rd_target = NULL,
		.ops = &i2c_bitbang_registers_rw_ops,
	},
	/* dwords 0x40 .. 0x13f, output buffer, read/write */
	{
		.start = 0x40,
		.length = &i2c_bitbang_obuf_length,
		.rd_target = i2c_data.buffer,
		.wr_target = i2c_data.buffer,
		.ops = &default_mcuio_range_ops,
	},
	/* dwords 0x140 .. 0x240, input buffer, read only */
	{
		.start = 0x140,
		.length = &i2c_bitbang_ibuf_length,
		.rd_target = i2c_data.buffer,
		.ops = &default_mcuio_range_ro_ops,
	},
};


declare_mcuio_function(mcuio_bitbang_i2c, i2c_bitbang_ranges, NULL, NULL,
		       &mcuio_func_common_runtime);
