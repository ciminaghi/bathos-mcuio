/*
 * Very simple protocol for microprocessor <-> mcu communication
 */
#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>
#include <bathos/jiffies.h>
#include <bathos/stdio.h>
#include <bathos/allocator.h>
#include <bathos/statemachine.h>
#include <bathos/dev_ops.h>

#include "mcuio-function.h"

#define DEV_BUFS_MAX_NUMBER 16

enum mcuio_event {
	START = 0,
	STD_PACKET_RECEIVED,
	RDMB_PACKET_RECEIVED,
	RDMB_SUBP_SENT,
	LAST_RDMB_SUBP_SENT,
	EXT_PACKET_HDR_RECEIVED,
	END_OF_EXT_PACKET_RECEIVED,
	ERROR,
	STOP,
};

enum mcuio_state {
	IDLE = 0,
	RUNNING,
	EXT_PACKET_RX,
	MCUIO_NEVENTS,
	SENDING_RDMB_REPLY,
};

static const struct statemachine PROGMEM mcuio_sm;

struct mcuio_function_runtime mcuio_func_common_runtime;

#ifdef CONFIG_MCUIO_DEBUG
static void dump_packet(const struct mcuio_base_packet *packet)
{
	int i;
	uint8_t *ptr;
	printf("%c %c %c\n", mcuio_packet_is_error(packet) ? '!' : ' ' ,
	       mcuio_packet_is_reply(packet) ? '>' : '<',
	       mcuio_packet_is_fill_data(packet) ? '+' : ' ');
	printf("\t%u.%u %s o 0x%04x\n",
	       mcuio_packet_dev(packet),
	       mcuio_packet_func(packet),
	       mcuio_packet_type_to_str(mcuio_packet_type(packet)),
	       mcuio_packet_offset(packet));
	printf("\tp: ");
	for (i = 0, ptr = (uint8_t *)packet; i < 2*sizeof(uint64_t); i++)
		printf("0x%02x ", ptr[i]);
	printf("\n");
}
#else
static inline void dump_packet(const struct mcuio_base_packet *packet)
{
}
#endif

declare_event(mcuio_function_request);

/*
 * mcuio private data, passed as runtime data to the statemachine functions
 *
 * @total_ext_len: total length of extended packet
 * @curr_ext_len: currently received length of extended packet
 * @ext_function: pointer to extended packet target function
 * @ext_range: pointer to extended packet range
 * @curr_ext_offset: pointer to extended packet current offset
 * @ext_wr: pointer to extended packet write function (NULL for rd packets)
 * @ext_err: extended packet error status
 * @input_pipe: pointer to input pipe
 * @output_pipe: pointer to output pipe
 * @dev_bufs: array to pointer of packet buffers
 * @sm_runtime: runtime data struct for statemachine
 */
struct mcuio_data {
	int total_ext_len;
	int curr_ext_len;
	struct mcuio_function *ext_function;
	const struct mcuio_range *ext_range;
	unsigned curr_ext_offset;
	mcuio_read ext_rd;
	mcuio_write ext_wr;
	int ext_err;
	struct bathos_pipe *input_pipe;
	struct bathos_pipe *output_pipe;
	struct mcuio_base_packet *input_packet;
	char *dev_bufs[DEV_BUFS_MAX_NUMBER];
	struct statemachine_runtime smr;
};

static struct mcuio_data *global_pdata;

#define to_mcuio_data(p) container_of(p, struct mcuio_data, smr)

#if defined ARCH_IS_HARVARD
/*
 * f is a pointer to data memory
 */
static inline int __get_range(const struct mcuio_function *f,
			      int index, struct mcuio_range *out)
{
	const struct mcuio_range *r;
	r = &f->ranges[index];
	memcpy_P(out, r, sizeof(*r));
	return 0;
}

static inline int __get_nranges(const struct mcuio_function *f)
{
	return f->nranges;
}

static inline struct mcuio_function *__get_function(int fn,
						    struct mcuio_function *f)
{
	struct mcuio_function *out = &mcuio_functions_start[fn];
	if (out >= mcuio_functions_end)
		return NULL;
	memcpy_P(f, out, sizeof(*f));
	return f;
}

static inline const struct mcuio_range_ops *
__get_range_ops(const struct mcuio_range *r,
		struct mcuio_range_ops *out)
{
	if (!r->ops)
		return NULL;
	memcpy_p(out, r->ops, sizeof(*out));
	return out;
}

#else /* !ARCH_IS_HARVARD */
static inline void __get_range(const struct mcuio_function *f,
			       int index, struct mcuio_range *out)
{
	*out = f->ranges[i];
}

static inline int __get_nranges(const struct mcuio_function *f)
{
	*out = f->nranges;
}

static inline struct mcuio_function *__get_function(int fn,
						    struct mcuio_function *f)
{
	struct mcuio_function *out = &mcuio_functions_start[packet->func];
	if (out >= mcuio_functions_end)
		return NULL;
	*f = *out;
	return f;
}

static inline int __copy_byte(uint8_t *dst, const uint8_t *src)
{
	*dst = *src;
	return 0;
}

static inline int __copy_word(uint16_t *dst, const uint16_t *src)
{
	*dst = *src;
	return 0;
}

static inline int __copy_dword(uint32_t *dst, const uint32_t *src)
{
	*dst = *src;
	return 0;
}

static inline void *memcpy_p(void *dst, const void *src, int size)
{
	return memcpy(dst, src, size);
}


static inline const struct mcuio_range_ops *
__get_range_ops(const struct mcuio_range *r,
		struct mcuio_range_ops *out)
{
	if (!r->ops)
		return NULL;
	memcpy(out, r->ops, sizeof(*out));
	return out;
}
#endif

/*
 * mcuio state machine (mealey)
 *
 *                        STD_PACKET_RECEIVED/         EXT_SUB_PACKET_RECEIVED/
 *                        FORWARD_TO_FUNCTION & REPLY  FORWARD_TO_FUNCTION
 *                          +---+                                +-------+
 *                          |   |                                |       |
 *                          |   |    EXT_PACKET_HDR_RECEIVED/    |       |
 *         START/START_PIPE |   V    FIRST_FORWARD_TO_FUNCTION   |       V
 *    IDLE ---------------> RUNNING --------------------------> EXT_PACKET_RX
 *                        / |  ^^^^                                 |   |
 *   RDMB_PACKET_RECEIVED/  |  ||||                                 |   |
 *   RDMB_FIRST         /   +--+||+---------------------------------+   |
 *                     / ERROR/ |+-------+            ERROR/            |
 *                    |  RESET  |LAST_RDMB_SUBP_SENT/ RESET             |
 *                    |         |        |                              |
 *                    V         |        +------------------------------+
 *               SENDING_RDMB_REPLY         END_OF_EXT_PACKET_RECEIVED/
 *                |             ^           LAST_FORWARD_TO_FUNCTION & REPLY
 *                |             |
 *                +-------------+
 *                RDMB_SUBP_SENT/
 *                RDMB
 */

static const struct mcuio_range *__lookup_range(struct mcuio_function *f,
						struct mcuio_base_packet *p,
						struct mcuio_range *r)
{
	int i, len;

	len = mcuio_packet_datalen(p);

	/* FIXME: SMARTER SEARCH */
	for (i = 0; i < f->nranges; i++) {
		unsigned int l;
		int end;
		__get_range(f, i, r);
		__copy_int(&l, r->length);
		end = r->start + l - 1;
		if ((mcuio_packet_offset(p) >= r->start) &&
		    (mcuio_packet_offset(p) + len - 1 <= end))
			return r;
	}
	return NULL;
}

static void __mcuio_send_to_host(struct mcuio_data *d,
				 struct mcuio_function *f)
{
	int stat;
	const struct mcuio_base_packet *p = &f->runtime->to_host;
	dump_packet(p);
	stat = pipe_write(d->output_pipe, (const char *)p, sizeof(*p));
	if (stat < 0)
		printf("mcuio: output error\n");
}

static void mcuio_send_request_to_host(struct mcuio_data *d,
				       struct mcuio_function *f)
{
	__mcuio_send_to_host(d, f);
}

static void mcuio_send_reply_to_host(struct mcuio_data *d,
				     struct mcuio_function *f)
{
	__mcuio_send_to_host(d, f);
}

static void __mcuio_send_error_to_host(struct mcuio_data *d,
				       struct mcuio_base_packet *p,
				       int e)
{
	p->body.base.data[0] = e;
	mcuio_packet_set_error(p);
	pipe_write(d->output_pipe, (const char *)p, sizeof(*p));
}

static void mcuio_send_reply_to_function(struct mcuio_data *d,
					 struct mcuio_function *f)
{
	int err = 0;

	if (mcuio_packet_is_error(d->input_packet))
		err = d->input_packet->body.base.data[0];

	if (f->ops->reply)
		f->ops->reply(f, err);
}

#define DEV_BUFS_MAX_NUMBER 16
#define MCUIO_SYNC_SIL_TIME (HZ/10)

static const char mcuio_sync_seq[] = { 0x55, 0xaa };

/*
 * Open and setup an input pipe: buffer queue mode with complete synchronization
 * (silence time and mcuio sync sequence): pipe_input_ready events will be
 * received as soon as a new 16bytes sub-packets is ready.
 */
static struct bathos_pipe *__open_input_pipe(struct mcuio_data *data)
{
	struct bathos_pipe *out = NULL;
	struct bathos_ioctl_data iocdata;
	int i, siltime = MCUIO_SYNC_SIL_TIME;
	struct dev_ioc_set_bqueue_data bqdata = {
		.bufsize = sizeof(struct mcuio_base_packet),
		.bufs = data->dev_bufs,
	};
	struct dev_ioc_set_rx_sync_seq_data ssdata = {
		.seq = mcuio_sync_seq,
		.seqsize = sizeof(mcuio_sync_seq),
	};

	out = pipe_open(CONFIG_MCUIO_PIPE_INPUT_PATH, BATHOS_MODE_INPUT, data);
	if (!out)
		out = bathos_stdin;

	iocdata.code = DEV_IOC_SET_RX_SYNC_SEQ;
	iocdata.data = &ssdata;
	if (pipe_ioctl(out, &iocdata) < 0) {
		printf("mcuio: could not set sync sequence\n");
		goto error0;
	}
	iocdata.code = DEV_IOC_SET_RX_SYNC_SIL_TIME;
	iocdata.data = &siltime;
	if (pipe_ioctl(out, &iocdata) < 0) {
		printf("mcuio: could not set sync silence time\n");
		goto error0;
	}

	for (i = 0; i < ARRAY_SIZE(data->dev_bufs); i++) {
		data->dev_bufs[i] = bathos_alloc_buffer(bqdata.bufsize);
		if (!data->dev_bufs[i])
			break;
	}
	printf("mcuio: allocated %d sub-packets buffers\n", i);
	if (!i) {
		printf("No memory for sub-packet buffers, aborting\n");
		goto error0;
	}
	bqdata.nbufs = i;
	iocdata.code = DEV_IOC_RX_SET_BQUEUE_MODE;
	iocdata.data = &bqdata;
	if (pipe_ioctl(out, &iocdata) < 0) {
		printf("mcuio: could not set bqueue mode\n");
		goto error1;
	}
	iocdata.code = DEV_IOC_RX_ENABLE;
	iocdata.data = NULL;
	if (pipe_ioctl(out, &iocdata) < 0) {
		printf("mcuio: could not enable rx\n");
		goto error1;
	}
	return out;

error1:
	for (i = 0; i < ARRAY_SIZE(data->dev_bufs); i++)
		bathos_free_buffer(data->dev_bufs[i],
				   sizeof(struct mcuio_base_packet));
error0:
	pipe_close(out);
	return NULL;
}

static void mcuio_start_handle(struct event_handler_data *ed)
{
	if (global_pdata)
		bathos_free_buffer(global_pdata, sizeof(*global_pdata));
	global_pdata = bathos_alloc_buffer(sizeof(*global_pdata));
	if (!global_pdata) {
		printf("no memory for mcuio\n");
		return;
	}
	if (init_statemachine(&mcuio_sm, &global_pdata->smr, IDLE) < 0) {
		printf("mcuio: error initializing state machine\n");
		goto error;
	}
	if (feed_statemachine(&mcuio_sm, &global_pdata->smr, START) < 0) {
		printf("mcuio: error starting state machine\n");
		goto error;
	}
	printf("mcuio started\n");
	return;

error:
	bathos_free_buffer(global_pdata, sizeof(*global_pdata));
}

declare_event_handler(mcuio_start, NULL, mcuio_start_handle, NULL);

static void mcuio_stop_handle(struct event_handler_data *ed)
{
	if (!global_pdata) {
		printf("WARNING: mcuio_stop: global_pdata is NULL\n");
		return;
	}
	feed_statemachine(&mcuio_sm, &global_pdata->smr, STOP);
	bathos_free_buffer(global_pdata, sizeof(*global_pdata));
	global_pdata = NULL;
}

declare_event_handler(mcuio_stop, NULL, mcuio_stop_handle, NULL);

static int mcuio_init(void *arg)
{
	printf("lininoIO initialized\n");
	return 0;
}

static void *mcuio_alive(void *arg)
{
	printf("mcuio alive\n");
	return NULL;
}

static struct bathos_task __task t_mcuio = {
	.name = "mcuio", .period = 60 * HZ,
	.job = mcuio_alive, .arg = NULL,
	.init = mcuio_init,
	.release = 3,
};

static void data_ready_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe;
	int stat, last;
	struct mcuio_data *data = *(struct mcuio_data **)ed->priv;
	struct mcuio_base_packet *packet = data->input_packet;
	struct bathos_ioctl_data iocdata = {
		.code = DEV_IOC_RX_DEQUEUE_BUFFER,
		.data = &packet,
	};

	pipe = data->input_pipe;

	stat = pipe_ioctl(pipe, &iocdata);
	if (stat < 0) {
		printf("mcuio: data_ready_handler: error peeking input\n");
		return;
	}
	dump_packet(packet);
	switch (statemachine_get_state(&data->smr)) {
	case RUNNING:
		/* HACK: fixed dev 1, ignore other destinations */
		if (mcuio_packet_dev(packet) != 1)
			return;
		if (mcuio_packet_is_extended(packet)) {
			feed_statemachine(&mcuio_sm, &data->smr,
					  (mcuio_packet_type(packet) &
					   mcuio_actual_type_mask) ==
					  mcuio_type_rdmb ?
					  RDMB_PACKET_RECEIVED :
					  EXT_PACKET_HDR_RECEIVED);
			break;
		}
		feed_statemachine(&mcuio_sm, &data->smr, STD_PACKET_RECEIVED);
		break;
	case EXT_PACKET_RX:
		/*
		 * there's no address in extended subpacket, don't check it,
		 * just check current length
		 */
		last = data->total_ext_len - data->curr_ext_len <=
			LAST_SUBF_DLEN;
		feed_statemachine(&mcuio_sm, &data->smr,
				  last ?
				  END_OF_EXT_PACKET_RECEIVED :
				  STD_PACKET_RECEIVED);
		break;
	default:
		/* Just throw packet away */
		break;
	}
}

declare_event_handler_with_priv(mcuio_data_ready, NULL, data_ready_handle,
				NULL, &global_pdata);


static void rdmb_subpacket_sent_handle(struct event_handler_data *ed)
{
	struct mcuio_data *data = *(struct mcuio_data **)ed->priv;


	feed_statemachine(&mcuio_sm, &data->smr,
			  data->curr_ext_len < data->total_ext_len ?
			  RDMB_SUBP_SENT : LAST_RDMB_SUBP_SENT);
}

declare_event_handler_with_priv(rdmb_subpacket_sent, NULL,
				rdmb_subpacket_sent_handle,
				NULL, &global_pdata);


static void mcuio_function_request_handle(struct event_handler_data *ed)
{
	struct mcuio_data *data = ed->priv;
	struct mcuio_function *f = ed->data;
	mcuio_send_request_to_host(data, f);
}


declare_event_handler_with_priv(mcuio_function_request, NULL,
				mcuio_function_request_handle,
				NULL, &global_pdata);

/* The mcuio state machine */

/* START output function: opens pipes */
static void __sm_start(const struct statemachine_state * PROGMEM state,
		       struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	data->input_pipe = __open_input_pipe(data);
	if (!data->input_pipe) {
		printf("mcuio: in pipe open err\n");
		return;
	}
	data->output_pipe = pipe_open(CONFIG_MCUIO_PIPE_OUTPUT_PATH,
				      BATHOS_MODE_OUTPUT,
				      data);
	if (!data->output_pipe)
		data->output_pipe = bathos_stdout;
	if (!data->output_pipe)
		printf("mcuio: out pipe open err\n");
}

static void __sm_stop(const struct statemachine_state * PROGMEM state,
		      struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	if (data->input_pipe)
		pipe_close(data->input_pipe);
	if (data->output_pipe)
		pipe_close(data->output_pipe);
}

/*
 * RESET output function, close and reopen pipes
 */
static void __sm_reset(const struct statemachine_state * PROGMEM state,
		       struct statemachine_runtime *runtime)
{
	__sm_stop(state, runtime);
	__sm_start(state, runtime);
}

enum rx_mode {
	SHORT_PACKET = 0,
	FIRST_SUBPACKET,
	SUBPACKET,
	LAST_SUBPACKET,
	RDMB_FIRST,
	RDMB,
};

/*
 * common workhorse for __smd_fwd_xxx and __smd_rdmb[_xxx] functions
 */
static void __sm_fwd_common(struct mcuio_data *data, enum rx_mode m)
{
	struct mcuio_function __f;
	struct mcuio_function *f;
	struct mcuio_range __r;
	const struct mcuio_range *r = NULL;
	struct mcuio_range_ops ops;
	const struct mcuio_range_ops *__ops;
	mcuio_read rptr = NULL;
	mcuio_write wptr = NULL;
	struct mcuio_base_packet *p = data->input_packet;
	int fill = mcuio_packet_is_fill_data(p);
	int t = mcuio_packet_type(p);
	int stat = -EPERM;

	switch (m) {
	case SHORT_PACKET:
	case FIRST_SUBPACKET:
	case RDMB_FIRST:
		f = __get_function(mcuio_packet_func(data->input_packet), &__f);
		if (!f) {
			__mcuio_send_error_to_host(data,
						   data->input_packet, -ENODEV);
			return;
		}
		if (m == SHORT_PACKET &&
		    mcuio_packet_is_reply(data->input_packet)) {
			/* Short reply from other end */
			mcuio_send_reply_to_function(data, f);
			return;
		}
		if (m == FIRST_SUBPACKET || m == RDMB_FIRST) {
			data->total_ext_len = mcuio_packet_datalen(p);
			data->curr_ext_len = FIRST_SUBF_DLEN;
		}
		r = __lookup_range(f, p, &__r);
		if (!r || !r->ops) {
			if (m == SHORT_PACKET || m == RDMB_FIRST)
				__mcuio_send_error_to_host(data,
							   data->input_packet,
							   -EPERM);
			return;
		}
		__ops = __get_range_ops(r, &ops);
		if (mcuio_packet_is_read(p))
			rptr = __ops->rd[(t & mcuio_actual_type_mask) >> 1];
		if (mcuio_packet_is_write(p))
			wptr = __ops->wr[(t & mcuio_actual_type_mask) >> 1];
		f->runtime->to_host = *p;
		mcuio_packet_set_reply(&f->runtime->to_host);
		if (rptr)
			stat = rptr(r,
				    mcuio_packet_offset(p) - r->start,
				    f->runtime->to_host.body.base.data,
				    fill);
		if (wptr)
			stat = wptr(r, mcuio_packet_offset(p) -
				    r->start,
				    (const uint32_t *)p->body.base.data,
				    fill);
		if (stat < 0) {
			if (m == SHORT_PACKET || m == RDMB_FIRST) {
				__mcuio_send_error_to_host(data,
							   data->input_packet,
							   -EPERM);
				return;
			}
			data->ext_err = stat;
			return;
		}
		if (m == SHORT_PACKET) {
			mcuio_send_reply_to_host(data, f);
			return;
		}
		/* FIRST SUBPACKET OR FIRST RDMB */
		data->ext_function = f;
		data->ext_range = r;
		data->curr_ext_offset =
			mcuio_packet_offset(p) + FIRST_SUBF_DLEN;
		data->ext_wr = wptr;
		data->ext_rd = rptr;
		data->ext_err = 0;
		if (m == RDMB_FIRST)
			mcuio_send_reply_to_host(data, f);
		return;
	case SUBPACKET:
	case LAST_SUBPACKET:
	case RDMB:
	{
		f = data->ext_function;
		const struct mcuio_range *r = data->ext_range;
		unsigned o = data->curr_ext_offset;
		mcuio_write wr = data->ext_wr;
		mcuio_read rd = data->ext_rd;
		int i, j;

		if (data->ext_err) {
			if (m == SUBPACKET)
				return;
			/* Last subpacket with error: send reply to host */
			__mcuio_send_error_to_host(data,
						   data->input_packet,
						   data->ext_err);
			return;
		}
		if (m == SUBPACKET) {
			/* Not last subpacket */
			data->ext_err = wr(r, o - r->start, p, MID_SUBF_DLEN);
			data->curr_ext_offset += MID_SUBF_DLEN;
			return;
		}
		if (m == RDMB) {
			/* Not last read many bytes reply */
			stat = rd(r,
				  o - r->start,
				  &f->runtime->to_host,
				  MID_SUBF_DLEN);
			data->curr_ext_offset += MID_SUBF_DLEN;
			data->curr_ext_len += MID_SUBF_DLEN;
			if (stat)
				printf("WARNING: unexpected rdmb error\n");
			mcuio_send_reply_to_host(data, f);
			return;
		}
		/* LAST SUBPACKET */
		if (data->ext_err) {
			__mcuio_send_error_to_host(data,
						   data->input_packet,
						   data->ext_err);
			return;
		}
		for (i = data->curr_ext_len, j = 0 ; i < data->total_ext_len &&
			     data->ext_err >= 0; i++, o++, j++)
			/* write byte by byte, no fill */
			data->ext_err = wr(r, o - r->start,
					   &((uint8_t *)p)[i], 0);
		/* Send reply */
		if (data->ext_err) {
			__mcuio_send_error_to_host(data,
						   data->input_packet,
						   data->ext_err);
			return;
		}
		mcuio_send_reply_to_host(data, f);
		return;
	}
	default:
		/* NEVER REACHED */
		return;
	}
	return;
}

/*
 * FORWARD_TO_FUNCTION_AND_REPLY output function: forward a short (regular)
 * packet to relevant function and send reply
 */
static void __sm_fwd_and_reply(const struct statemachine_state * PROGMEM state,
			       struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	__sm_fwd_common(data, SHORT_PACKET);
}

/*
 * FORWARD_TO_FUNCTION output function: forward second to (last - 1) sub-packet
 * to relevant function and send reply
 */
static void __sm_fwd(const struct statemachine_state * PROGMEM state,
		     struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	__sm_fwd_common(data, SUBPACKET);
}

/*
 * FIRST_FORWARD_TO_FUNCTION output function: forward first sub-packet
 * to relevant function.
 */
static void __sm_first_fwd(const struct statemachine_state * PROGMEM state,
			   struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	__sm_fwd_common(data, FIRST_SUBPACKET);
}

/*
 * LAST_FORWARD_TO_FUNCTION_AND_REPLY output function: forward last sub-packet
 * to relevant function and send reply
 */
static void
__sm_last_fwd_and_reply(const struct statemachine_state * PROGMEM state,
			struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	__sm_fwd_common(data, LAST_SUBPACKET);
}

/*
 * RDMB_FIRST output function: send first read many bytes sub-packet
 */
static void
__sm_rdmb_first(const struct statemachine_state * PROGMEM state,
		struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	__sm_fwd_common(data, RDMB_FIRST);
}

/*
 * RDMB output function: send first + 1 to last read many bytes sub-packet
 */
static void __sm_rdmb(const struct statemachine_state * PROGMEM state,
		      struct statemachine_runtime *runtime)
{
	struct mcuio_data *data = to_mcuio_data(runtime);

	__sm_fwd_common(data, RDMB);
}


/*
 * These output functions just show warnings
 */

static void __show_warn(const char * PROGMEM str,
			struct statemachine_runtime *runtime)
{
	printf("WARNING: %s in state %d\n", statemachine_get_state(runtime));
}

static void
__sm_show_warn_unexp_ext_hdr(const struct statemachine_state * PROGMEM state,
			     struct statemachine_runtime *runtime)
{
	__show_warn(PSTR("unexpected extended header"), runtime);
}

static void
__sm_show_warn_unexp_ext_end(const struct statemachine_state * PROGMEM state,
			     struct statemachine_runtime *runtime)
{
	__show_warn(PSTR("unexpected extended end of packet"), runtime);
}

static void
__sm_show_warn_unexp_rdmb_sent(const struct statemachine_state * PROGMEM state,
			       struct statemachine_runtime *runtime)
{
	__show_warn(PSTR("unexpected rdmb subpacket sent"), runtime);
}

static void
__sm_show_warn_unexp_last_rdmb_sent(const struct statemachine_state *
				    PROGMEM state,
				    struct statemachine_runtime *runtime)
{
	__show_warn(PSTR("unexpected last rdmb subpacket sent"), runtime);
}

static void
__sm_show_warn_unexp_std_packet_received(const struct statemachine_state *
				    PROGMEM state,
				    struct statemachine_runtime *runtime)
{
	__show_warn(PSTR("unexpected std packet received"), runtime);
}

static void
__sm_show_warn_unexp_rdmb_packet_received(const struct statemachine_state *
					  PROGMEM state,
					  struct statemachine_runtime *runtime)
{
	__show_warn(PSTR("unexpected rdmb packet received"), runtime);
}

static void
__sm_show_warn_unexp_exp_ext_packet_hdr(const struct statemachine_state *
					PROGMEM state,
					struct statemachine_runtime *runtime)
{
	__show_warn(PSTR("unexpected extended packet hdr received"), runtime);
}

static state_outfunc * PROGMEM const idle_out_funcs[] = {
	[START] = __sm_start,
	[STD_PACKET_RECEIVED ... STOP] = NULL,
};

static const int PROGMEM idle_next_states[] = {
	[START] = RUNNING,
	[STD_PACKET_RECEIVED ... STOP] = IDLE,
};

static state_outfunc * PROGMEM const running_out_funcs[] = {
	[START] = NULL,
	[STD_PACKET_RECEIVED] = __sm_fwd_and_reply,
	[RDMB_PACKET_RECEIVED] = __sm_rdmb_first,
	[RDMB_SUBP_SENT] = __sm_show_warn_unexp_rdmb_sent,
	[LAST_RDMB_SUBP_SENT] = __sm_show_warn_unexp_last_rdmb_sent,
	[EXT_PACKET_HDR_RECEIVED] = __sm_first_fwd,
	[END_OF_EXT_PACKET_RECEIVED] = __sm_show_warn_unexp_ext_end,
	[ERROR] = __sm_reset,
	[STOP] = __sm_stop,
};

static const int PROGMEM running_next_states[] = {
	[START] = RUNNING,
	[STD_PACKET_RECEIVED] = RUNNING,
	[RDMB_PACKET_RECEIVED] = SENDING_RDMB_REPLY,
	[RDMB_SUBP_SENT] = RUNNING,
	[LAST_RDMB_SUBP_SENT] = RUNNING,
	[EXT_PACKET_HDR_RECEIVED] = EXT_PACKET_RX,
	[END_OF_EXT_PACKET_RECEIVED] = RUNNING,
	[ERROR] = RUNNING,
	[STOP] = IDLE,
};

static state_outfunc * PROGMEM const sending_rdmb_reply_out_funcs[] = {
	[START] = NULL,
	[STD_PACKET_RECEIVED] = __sm_show_warn_unexp_std_packet_received,
	[RDMB_PACKET_RECEIVED] = __sm_show_warn_unexp_rdmb_packet_received,
	[RDMB_SUBP_SENT] = __sm_rdmb,
	[LAST_RDMB_SUBP_SENT] = NULL,
	[EXT_PACKET_HDR_RECEIVED] = __sm_show_warn_unexp_exp_ext_packet_hdr,
	[END_OF_EXT_PACKET_RECEIVED] = __sm_show_warn_unexp_ext_end,
	[ERROR] = __sm_reset,
	[STOP] = __sm_stop,
};

static const int PROGMEM sending_rdmb_reply_next_states[] = {
	[START] = SENDING_RDMB_REPLY,
	[STD_PACKET_RECEIVED] = SENDING_RDMB_REPLY,
	[RDMB_PACKET_RECEIVED] = SENDING_RDMB_REPLY,
	[RDMB_SUBP_SENT] = SENDING_RDMB_REPLY,
	[LAST_RDMB_SUBP_SENT] = RUNNING,
	[EXT_PACKET_HDR_RECEIVED] = SENDING_RDMB_REPLY,
	[END_OF_EXT_PACKET_RECEIVED] = SENDING_RDMB_REPLY,
	[ERROR] = RUNNING,
	[STOP] = IDLE,
};

static state_outfunc * PROGMEM const ext_packet_rx_out_funcs[] = {
	[START] = NULL,
	[STD_PACKET_RECEIVED] = __sm_fwd,
	[RDMB_PACKET_RECEIVED] = __sm_show_warn_unexp_rdmb_packet_received,
	[RDMB_SUBP_SENT] = __sm_show_warn_unexp_rdmb_sent,
	[LAST_RDMB_SUBP_SENT] = __sm_show_warn_unexp_last_rdmb_sent,
	[EXT_PACKET_HDR_RECEIVED] = __sm_show_warn_unexp_ext_hdr,
	[END_OF_EXT_PACKET_RECEIVED] = __sm_last_fwd_and_reply,
	[ERROR] = __sm_reset,
	[STOP] = __sm_stop,
};

static const int PROGMEM ext_packet_rx_next_states[] = {
	[START] = EXT_PACKET_RX,
	[STD_PACKET_RECEIVED] = EXT_PACKET_RX,
	[RDMB_PACKET_RECEIVED] = EXT_PACKET_RX,
	[RDMB_SUBP_SENT] = EXT_PACKET_RX,
	[LAST_RDMB_SUBP_SENT] = EXT_PACKET_RX,
	[EXT_PACKET_HDR_RECEIVED] = EXT_PACKET_RX,
	[END_OF_EXT_PACKET_RECEIVED] = RUNNING,
	[ERROR] = RUNNING,
	[STOP] = IDLE,
};

static const struct statemachine_state PROGMEM mcuio_states[] = {
	[IDLE] = {
		.next_states = idle_next_states,
		.out = idle_out_funcs,
	},
	[RUNNING] = {
		.next_states = running_next_states,
		.out = running_out_funcs,
	},
	[EXT_PACKET_RX] = {
		.next_states = ext_packet_rx_next_states,
		.out = ext_packet_rx_out_funcs,
	},
};

static const struct statemachine PROGMEM mcuio_sm = {
	.type = MEALEY,
	.nstates = ARRAY_SIZE(mcuio_states),
	.states = mcuio_states,
	.nevents = MCUIO_NEVENTS,
};



/* Default read/write operations */

int mcuio_rdb(const struct mcuio_range *r, unsigned offset, void *__out,
	      int fill)
{
	uint8_t *out = __out;
	if (!fill) {
		if (__copy_byte(out, (uint8_t *)(r->rd_target + offset)) < 0)
			return -1;
	} else
		memcpy_p(out, r->rd_target + offset, sizeof(uint64_t));
	return fill ? sizeof(uint64_t) : sizeof(uint8_t);
}

int mcuio_rdw(const struct mcuio_range *r, unsigned offset, void *__out,
	      int fill)
{
	uint16_t *out = __out;
	if (!fill) {
		if (__copy_word(out, (uint16_t *)(r->rd_target + offset)) < 0)
			return -1;
	} else
		memcpy_p(out, r->rd_target + offset, sizeof(uint64_t));
	return fill ? sizeof(uint64_t): sizeof(uint16_t);
}

int mcuio_rddw(const struct mcuio_range *r, unsigned offset, void *__out,
	       int fill)
{
	uint32_t *out = __out;
	if (__copy_dword(out, (uint32_t *)(r->rd_target + offset)) < 0)
		return -1;
	if (fill)
		if (__copy_dword(&out[1], (uint32_t *)(r->rd_target + offset +
						       sizeof(uint32_t))) < 0)
			return -1;
	return fill ? sizeof(uint64_t) : sizeof(uint32_t);
}

int mcuio_rdmb(const struct mcuio_range *r, unsigned offset, void *__out,
	       int len)
{
	uint8_t *out = __out;
	memcpy_p(out, r->rd_target + offset, len);
	return len;
}

int mcuio_wrb(const struct mcuio_range *r, unsigned offset,
	      const void *__in, int fill)
{
	const uint8_t *in = __in;
	if (!fill)
		*(uint8_t *)(r->wr_target + offset) = *in;
	else
		memcpy(r->wr_target + offset, in, sizeof(uint64_t));
	return fill ? sizeof(uint64_t) : sizeof(uint8_t);
}

int mcuio_wrw(const struct mcuio_range *r, unsigned offset,
	      const void *__in, int fill)
{
	const uint16_t *in = __in;
	if (!fill)
		*(uint16_t *)(r->wr_target + offset) = *in;
	else
		memcpy(r->wr_target + offset, in, sizeof(uint64_t));
	return fill ? sizeof(uint64_t) : sizeof(uint16_t);
}

int mcuio_wrdw(const struct mcuio_range *r, unsigned offset,
	       const void *__in, int fill)
{
	const uint32_t *in = __in;
	*(uint32_t *)(r->wr_target + offset) = *in;
	if (fill)
		*(uint32_t *)(r->wr_target + offset + sizeof(uint32_t)) = in[1];
	return fill ? sizeof(uint64_t) : sizeof(uint32_t);
}

int mcuio_wrmb(const struct mcuio_range *r, unsigned offset,
	       const void *__in, int len)
{
	const uint8_t *in = __in;
	memcpy(r->wr_target + offset, in, len);
	return len;
}

const struct mcuio_range_ops PROGMEM default_mcuio_range_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_rddw, mcuio_rdmb },
	.wr = { mcuio_wrb, mcuio_wrw, mcuio_wrdw, mcuio_wrmb },
};


const struct mcuio_range_ops PROGMEM default_mcuio_range_ro_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_rddw, mcuio_rdmb, },
};

