/*
 * Very simple protocol for microprocessor <-> mcu communication
 */
#include <arch/hw.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

#ifdef CONFIG_MCUIO_DEBUG
static void dump_packet(struct mcuio_base_packet *packet)
{
	printf("I: bus = %u, dev = %u, func = %u, type = %s, "
	       "offset = 0x%04x\r\n",
	       packet->bus, packet->dev, packet->func,
	       mcuio_packet_type_to_str(packet->type),
	       packet->offset);
}
#else
static inline void dump_packet(struct mcuio_base_packet *packet)
{
}
#endif

declare_event(mcuio_function_request);

struct mcuio_data {
	struct bathos_pipe *input_pipe;
	struct bathos_pipe *output_pipe;
	struct mcuio_base_packet input_packet;
};

static struct mcuio_data global_data;

static const struct mcuio_range *__lookup_range(struct mcuio_function *f,
						struct mcuio_base_packet *p)
{
	int i, len;

	len = mcuio_packet_data_len(p);

	/* FIXME: SMARTER SEARCH */
	for (i = 0; i < f->nranges; i++) {
		unsigned int start = f->ranges[i].start;
		unsigned int end = start + f->ranges[i].length - 1;
		if ((p->offset >= start) && (p->offset + len - 1 <= end))
			return &f->ranges[i];
	}
	return NULL;
}

static int __exec_request(struct mcuio_function *f,
			  const struct mcuio_range *r,
			  struct mcuio_base_packet *p)
{
	mcuio_read rptr = NULL;
	mcuio_write wptr = NULL;
	int fill = mcuio_packet_is_fill_data(p);
	printf("%s %d, p->type = %d\n", __func__, __LINE__, p->type);
	if (mcuio_packet_is_read(p))
		rptr = r->ops->rd[(p->type & ((1 << 5) - 1)) >> 1];
	if (mcuio_packet_is_write(p))
		wptr = r->ops->wr[((p->type - 1) & ((1 << 5) - 1)) >> 1];
	f->runtime->to_host = *p;
	printf("%s %d: rptr = %p, wptr = %p\n", __func__, __LINE__, rptr, wptr);
	mcuio_packet_set_reply(&f->runtime->to_host);
	if (rptr)
		return rptr(r, p->offset - r->start, f->runtime->to_host.data,
			    fill);
	if (wptr)
		return wptr(r, p->offset - r->start, (const uint32_t *)p->data,
			    fill);
	return -EPERM;
}

static void __mcuio_send_to_host(struct mcuio_data *d,
				 struct mcuio_function *f)
{
	const struct mcuio_base_packet *p = &f->runtime->to_host;
	pipe_write(d->output_pipe, (const char *)p, sizeof(*p));
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
	p->data[0] = e;
	mcuio_packet_set_error(p);
	pipe_write(d->output_pipe, (const char *)p, sizeof(*p));
}

static void __mcuio_send_error_to_function(struct mcuio_data *d,
					   struct mcuio_function *f,
					   int err)
{
	if (f->ops->reply)
		f->ops->reply(f, err);
}

static void mcuio_send_reply_to_function(struct mcuio_data *d,
					 struct mcuio_function *f)
{
	f->runtime->from_host = d->input_packet;
	if (f->ops->reply)
		f->ops->reply(f, 0);
}

static void mcuio_request_received(struct mcuio_data *d,
				   struct mcuio_function *f)
{
	const struct mcuio_range *r = __lookup_range(f, &d->input_packet);
	int stat;
	printf("d = %p\n", d);
	printf("d->input_pipe = %p, d->output_pipe = %p, r = %p\n",
	       d->input_pipe, d->output_pipe, r);

	if (!r) {
		__mcuio_send_error_to_host(d, &f->runtime->to_host, EINVAL);
		return;
	}
	stat = __exec_request(f, r, &d->input_packet);
	if (stat < 0) {
		__mcuio_send_error_to_host(d, &f->runtime->to_host, -stat);
		return;
	}
	mcuio_send_reply_to_host(d, f);
}

static struct bathos_pipe *__open_input_pipe(struct mcuio_data *data)
{
	struct bathos_pipe *out;
	out = pipe_open("fd:3", BATHOS_MODE_INPUT, data);
	if (!out)
		out = bathos_stdin;
	return out;
}

static int mcuio_init(void *arg)
{
	struct mcuio_data *data = arg;
	if (!data)
		return -1;
	data->input_pipe = __open_input_pipe(data);
	if (!data->input_pipe) {
		printf("mcuio: error opening input pipe\n");
		return -1;
	}
	data->output_pipe = pipe_open("fd:4", BATHOS_MODE_OUTPUT,
				      data);
	if (!data->output_pipe)
		data->output_pipe = bathos_stdout;
	if (!data->output_pipe) {
		printf("mcuio: error opening output pipe\n");
		return -1;
	}
	printf("mcuio init ok, d = %p, input_pipe = %p, output_pipe = %p\n",
	       data, data->input_pipe, data->output_pipe);
	return 0;
}

static void *mcuio_alive(void *arg)
{
	printf("mcuio is %s\n", arg ? "alive" : "dead");
	return arg;
}

static struct bathos_task __task t_mcuio = {
	.name = "mcuio", .period = 60 * HZ,
	.job = mcuio_alive, .arg = &global_data,
	.init = mcuio_init,
	.release = 3,
};


static void pipe_input_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe;
	struct mcuio_base_packet *packet;
	int stat;
	struct mcuio_data *data = ed->priv;
	struct mcuio_function *f;

	pipe = ed->data;
	packet = &data->input_packet;
	stat = pipe_read(pipe, (char *)packet, sizeof(*packet));
	if (!stat) {
		/* The other end closed the pipe, reopen it */
		pipe_close(pipe);
		data->input_pipe = __open_input_pipe(data);
		return;
	}
	if (stat < sizeof(packet))
		return;
	/* HACK: fixed dev 1, ignore other destinations */
	if (packet->dev != 1)
		return;
	dump_packet(packet);
	f = &mcuio_functions_start[packet->func];
	if (f >= mcuio_functions_end) {
		__mcuio_send_error_to_host(data, packet, -ENODEV);
		return;
	}
	mcuio_packet_is_reply(packet) ?
		mcuio_send_reply_to_function(data, f) :
		mcuio_request_received(data, f);
}

static void pipe_input_exit(struct event_handler_data *ed)
{
	struct mcuio_data *data = ed->data;
	pipe_close(data->input_pipe);
	pipe_close(data->output_pipe);
}


declare_event_handler_with_priv(pipe_input_ready, NULL, pipe_input_handle,
				pipe_input_exit, &global_data);


static void mcuio_function_request_handle(struct event_handler_data *ed)
{
	struct mcuio_data *data = ed->priv;
	struct mcuio_function *f = ed->data;
	mcuio_send_request_to_host(data, f);
}


declare_event_handler_with_priv(mcuio_function_request, NULL,
				mcuio_function_request_handle,
				NULL, &global_data);


/* Default read/write operations */

int mcuio_rdb(const struct mcuio_range *r, unsigned offset, uint32_t *__out,
	      int fill)
{
	uint8_t *out = (uint8_t *)__out;
	if (!fill)
		*out = *(uint8_t *)(r->rd_target + offset);
	else
		memcpy(out, r->rd_target + offset, sizeof(uint64_t));
	return fill ? sizeof(uint64_t) : sizeof(uint8_t);
}

int mcuio_rdw(const struct mcuio_range *r, unsigned offset, uint32_t *__out,
	      int fill)
{
	uint16_t *out = (uint16_t *)__out;
	if (!fill)
		*out = *(uint16_t *)(r->rd_target + offset);
	else
		memcpy(out, r->rd_target + offset, sizeof(uint64_t));
	return fill ? sizeof(uint64_t): sizeof(uint16_t);
}

int mcuio_rddw(const struct mcuio_range *r, unsigned offset, uint32_t *out,
	       int fill)
{
	printf("%s: reading %p (offset = %u), fill = %d\n", __func__,
	       r->target, offset, fill);
	*out = *(uint32_t *)(r->rd_target + offset);
	if (fill)
		out[1] = *(uint32_t *)(r->rd_target + offset +
				       sizeof(uint32_t));
	return fill ? sizeof(uint64_t) : sizeof(uint32_t);
}

int mcuio_rdq(const struct mcuio_range *r, unsigned offset, uint32_t *out,
	      int fill)
{
	memcpy(out, r->rd_target + offset, sizeof(uint64_t));
	return sizeof(uint64_t);
}

int mcuio_wrb(const struct mcuio_range *r, unsigned offset,
	      const uint32_t *__in, int fill)
{
	const uint8_t *in = (const uint8_t *)__in;
	if (!fill)
		*(uint8_t *)(r->wr_target + offset) = *in;
	else
		memcpy(r->wr_target + offset, in, sizeof(uint64_t));
	return fill ? sizeof(uint64_t) : sizeof(uint8_t);
}

int mcuio_wrw(const struct mcuio_range *r, unsigned offset,
	      const uint32_t *__in, int fill)
{
	const uint16_t *in = (const uint16_t *)__in;
	if (!fill)
		*(uint16_t *)(r->wr_target + offset) = *in;
	else
		memcpy(r->wr_target + offset, in, sizeof(uint64_t));
	return fill ? sizeof(uint64_t) : sizeof(uint16_t);
}

int mcuio_wrdw(const struct mcuio_range *r, unsigned offset,
	       const uint32_t *in, int fill)
{
	*(uint32_t *)(r->wr_target + offset) = *in;
	if (fill)
		*(uint32_t *)(r->wr_target + offset + sizeof(uint32_t)) = in[1];
	return fill ? sizeof(uint64_t) : sizeof(uint32_t);
}

int mcuio_wrq(const struct mcuio_range *r, unsigned offset, const uint32_t *in,
	      int fill)
{
	memcpy(r->wr_target + offset, in, sizeof(uint64_t));
	return sizeof(uint64_t);
}

const struct mcuio_range_ops default_mcuio_range_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_rddw, mcuio_rdq, },
	.wr = { mcuio_wrb, mcuio_wrw, mcuio_wrdw, mcuio_wrq, },
};


const struct mcuio_range_ops default_mcuio_range_ro_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_rddw, mcuio_rdq, },
};

