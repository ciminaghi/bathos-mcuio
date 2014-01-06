/*
 * Very simple protocol for microprocessor <-> mcu communication
 */
#include <arch/hw.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

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

static int __exec_request(const struct mcuio_range *r,
			  struct mcuio_base_packet *p)
{
	mcuio_read rptr = NULL;
	mcuio_write wptr = NULL;
	if (mcuio_packet_is_read(p))
		rptr = r->ops->rd[(p->type & ((1 << 5) - 1)) >> 1];
	if (mcuio_packet_is_write(p))
		wptr = r->ops->wr[((p->type - 1) & ((1 << 5) - 1)) >> 1];
	if (rptr)
		return rptr(r, p->offset, p->data);
	if (wptr)
		return wptr(r, p->offset, (const uint32_t *)p->data);
	return -EPERM;
}

static void __mcuio_send_to_host(struct mcuio_data *d,
				 struct mcuio_function *f)
{
	const struct mcuio_base_packet *p = f->to_host;
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

static void __mcuio_send_error_to_host(struct mcuio_data *d, int e)
{
	d->input_packet.data[0] = e;
	mcuio_packet_set_reply(&d->input_packet);
	pipe_write(d->output_pipe, (const char *)&d->input_packet,
		   sizeof(d->input_packet));
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
	*f->from_host = d->input_packet;
	if (f->ops->reply)
		f->ops->reply(f, 0);
}

static void mcuio_request_received(struct mcuio_data *d,
				   struct mcuio_function *f)
{
	const struct mcuio_range *r = __lookup_range(f, &d->input_packet);
	int stat;
	printf("d = %p\n", d);
	printf("d->input_pipe = %p, d->output_pipe = %p\n", d, d->input_pipe,
	       d->output_pipe);

	if (!r) {
		__mcuio_send_error_to_host(d, EINVAL);
		return;
	}
	stat = __exec_request(r, &d->input_packet);
	if (stat) {
		__mcuio_send_error_to_host(d, -stat);
		return;
	}
	mcuio_send_reply_to_host(d, f);
}


static int mcuio_init(void *arg)
{
	struct mcuio_data *data = arg;
	if (!data)
		return -1;
	data->input_pipe = pipe_open("/tmp/fifoi", BATHOS_MODE_INPUT,
				     data);
	if (!data->input_pipe)
		data->input_pipe = bathos_stdin;
	if (!data->input_pipe) {
		printf("mcuio: error opening input pipe\n");
		return -1;
	}
	data->output_pipe = pipe_open("/tmp/fifoo", BATHOS_MODE_OUTPUT,
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

static void dump_packet(struct mcuio_base_packet *packet)
{
	printf("bus = %u, dev = %u, func = %u, type = %s, offset = 0x%04x\n",
	       packet->bus, packet->dev, packet->func,
	       mcuio_packet_type_to_str(packet->type),
	       packet->offset);
	if (mcuio_packet_is_write(packet))
		printf("data = 0x%08x - 0x%08x\n", packet->data[0],
		       packet->data[1]);
}

static void pipe_input_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe;
	struct mcuio_base_packet *packet;
	int stat;
	struct mcuio_data *data = ed->priv;
	struct mcuio_function *f;

	pipe = ed->evt->data;
	packet = &data->input_packet;
	stat = pipe_read(pipe, (char *)packet, sizeof(packet));
	if (stat < sizeof(packet))
		return;
	dump_packet(packet);
	f = &mcuio_functions_start[packet->func];
	mcuio_packet_is_reply(packet) ?
		mcuio_send_reply_to_function(data, f) :
		mcuio_request_received(data, f);
}

static void pipe_input_exit(struct event_handler_data *ed)
{
	struct mcuio_data *data = ed->evt->data;
	pipe_close(data->input_pipe);
	pipe_close(data->output_pipe);
}


declare_event_handler_with_priv(pipe_input_ready, NULL, pipe_input_handle,
				pipe_input_exit, &global_data);


static void mcuio_function_request_handle(struct event_handler_data *ed)
{
	struct mcuio_data *data = ed->priv;
	struct mcuio_function *f = ed->evt->data;
	mcuio_send_request_to_host(data, f);
}


declare_event_handler_with_priv(mcuio_function_request, NULL,
				mcuio_function_request_handle,
				NULL, &global_data);


/* Default read/write operations */

int mcuio_rdb(const struct mcuio_range *r, unsigned offset, uint32_t *__out)
{
	uint8_t *out = (uint8_t *)__out;
	*out = *(uint8_t *)(r->target + offset);
	return sizeof(uint8_t);
}

int mcuio_rdw(const struct mcuio_range *r, unsigned offset, uint32_t *__out)
{
	uint16_t *out = (uint16_t *)__out;
	*out = *(uint16_t *)(r->target + offset);
	return sizeof(uint16_t);
}

int mcuio_rddw(const struct mcuio_range *r, unsigned offset, uint32_t *out)
{
	*out = *(uint32_t *)(r->target + offset);
	return sizeof(uint32_t);
}

int mcuio_rdq(const struct mcuio_range *r, unsigned offset, uint32_t *out)
{
	out[0] = *(uint32_t *)(r->target + offset);
	out[1] = *(uint32_t *)(r->target + offset + sizeof(uint32_t));
	return sizeof(uint64_t);
}

int mcuio_wrb(const struct mcuio_range *r, unsigned offset,
	      const uint32_t *__in)
{
	const uint8_t *in = (const uint8_t *)__in;
	*(uint8_t *)(r->target + offset) = *in;
	return sizeof(uint8_t);
}

int mcuio_wrw(const struct mcuio_range *r, unsigned offset,
	      const uint32_t *__in)
{
	const uint16_t *in = (const uint16_t *)__in;
	*(uint16_t *)(r->target + offset) = *in;
	return sizeof(uint16_t);
}

int mcuio_wrdw(const struct mcuio_range *r, unsigned offset, const uint32_t *in)
{
	*(uint32_t *)(r->target + offset) = *in;
	return sizeof(uint32_t);
}

int mcuio_wrq(const struct mcuio_range *r, unsigned offset, const uint32_t *in)
{
	*(uint32_t *)(r->target + offset) = in[0];
	*(uint32_t *)(r->target + offset + sizeof(uint32_t)) = in[1];
	return sizeof(uint64_t);
}

const struct mcuio_range_ops default_mcuio_range_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_rddw, mcuio_rdq, },
	.wr = { mcuio_wrb, mcuio_wrw, mcuio_wrdw, mcuio_wrq, },
};


const struct mcuio_range_ops default_mcuio_range_ro_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_rddw, mcuio_rdq, },
};

