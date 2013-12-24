/*
 * Very simple protocol for microprocessor <-> mcu communication
 */
#include <arch/hw.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/statemachine.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

declare_event(mcuio_function_request);
declare_event(mcuio_function_reply);
declare_event(mcuio_host_request);
declare_event(mcuio_host_reply);

struct mcuio_smr_priv {


};

struct mcuio_data {
	struct bathos_pipe *input_pipe;
	struct bathos_pipe *output_pipe;
	struct mcuio_base_packet input_packet;
	struct statemachine_runtime smr[32];
};

static struct mcuio_data global_data;

static state_outfunc mcuio_trigger_request_received;
static state_outfunc mcuio_send_request_to_host;
static state_outfunc mcuio_send_reply_to_host;
static state_outfunc mcuio_send_error_to_host;
static state_outfunc mcuio_send_busy_to_host;
static state_outfunc mcuio_send_busy_to_function;
static state_outfunc mcuio_send_reply_to_function;
static state_outfunc mcuio_send_error_to_function;

/* MCUIO state machine */
enum mcuio_state {
	IDLE = 0,
	HOST_REQUEST_RECEIVED,
	FUNCTION_REQUEST_RECEIVED,
	ERROR,
	NSTATES,
};

enum mcuio_event {
	/* Request received from peer (host) */
	REQUEST_FROM_HOST = 0,
	/* Request received from local function */
	REQUEST_FROM_FUNCTION,
	/* Reply received from host */
	REPLY_FROM_HOST,
	/* Reply from local function */
	REPLY_FROM_FUNCTION,
	/* Guess what */
	TIMEOUT,
	NEVENTS,
};

static const int mcuio_idle_next[] = {
	/* Send request to function */
	[REQUEST_FROM_HOST] = HOST_REQUEST_RECEIVED,
	[REQUEST_FROM_FUNCTION] = FUNCTION_REQUEST_RECEIVED,
	[REPLY_FROM_HOST] = IDLE,
	[REPLY_FROM_FUNCTION] = IDLE,
	[TIMEOUT] = IDLE,
};

static state_outfunc *mcuio_idle_out[] = {
	[REQUEST_FROM_HOST] = mcuio_trigger_request_received,
	[REQUEST_FROM_FUNCTION] = mcuio_send_request_to_host,
	[REPLY_FROM_HOST ... TIMEOUT] = NULL,
};

static const int mcuio_host_request_received_next[] = {
	[REQUEST_FROM_HOST] = HOST_REQUEST_RECEIVED,
	[REQUEST_FROM_FUNCTION] = HOST_REQUEST_RECEIVED,
	[REPLY_FROM_HOST] = HOST_REQUEST_RECEIVED,
	/* Send reply to host */
	[REPLY_FROM_FUNCTION] = IDLE,
	[TIMEOUT] = ERROR,
};

static state_outfunc *mcuio_host_request_received_out[] = {
	[REQUEST_FROM_HOST ... REPLY_FROM_HOST] = NULL,
	[REPLY_FROM_FUNCTION] = mcuio_send_reply_to_host,
	[TIMEOUT] = mcuio_send_error_to_host,
};

static const int mcuio_function_request_received_next[] = {
	/* Send busy to host */
	[REQUEST_FROM_HOST] = FUNCTION_REQUEST_RECEIVED,
	/* Send busy to function */
	[REQUEST_FROM_FUNCTION] = FUNCTION_REQUEST_RECEIVED,
	[REPLY_FROM_HOST] = IDLE,
	[REPLY_FROM_FUNCTION] = FUNCTION_REQUEST_RECEIVED,
	[TIMEOUT] = ERROR,
};

static state_outfunc *mcuio_function_request_received_out[] = {
	[REQUEST_FROM_HOST] = mcuio_send_busy_to_host,
	[REQUEST_FROM_FUNCTION] = mcuio_send_busy_to_function,
	[REPLY_FROM_HOST] = mcuio_send_reply_to_function,
	[REPLY_FROM_FUNCTION] = NULL,
	[TIMEOUT] = mcuio_send_error_to_function,
};

static state_outfunc *mcuio_error_out[] = {
	[REQUEST_FROM_HOST ... TIMEOUT] = NULL,
};

static const int mcuio_error_next[] = {
	[REQUEST_FROM_HOST ... REPLY_FROM_FUNCTION] = ERROR,
	[TIMEOUT] = IDLE,
};

static const struct statemachine_state mcuio_states[] = {
	[IDLE] = {
		.next_states = mcuio_idle_next,
		.out = mcuio_idle_out,
	},
	[HOST_REQUEST_RECEIVED] = {
		.next_states = mcuio_host_request_received_next,
		.out = mcuio_host_request_received_out,
	},
	[FUNCTION_REQUEST_RECEIVED] = {
		.next_states = mcuio_function_request_received_next,
		.out = mcuio_function_request_received_out,
	},
	[ERROR] = {
		.next_states = mcuio_error_next,
		.out = mcuio_error_out,
	}
};

static void mcuio_request_received(const struct statemachine_state *s,
				   struct statemachine_runtime *rt)
{
	
}

static void __mcuio_send_to_host(const struct statemachine_state *s,
				       struct statemachine_runtime *rt)
{
	struct mcuio_data *d = rt->priv;
	int func = rt - d->smr;
	struct mcuio_function *f = &mcuio_functions_start[func];
	const struct mcuio_base_packet *p = f->to_host;
	pipe_write(d->output_pipe, (const char *)p, sizeof(*p));
}

static void mcuio_send_request_to_host(const struct statemachine_state *s,
				       struct statemachine_runtime *rt)
{
	__mcuio_send_to_host(s, rt);
}

static void mcuio_send_reply_to_host(const struct statemachine_state *s,
				     struct statemachine_runtime *rt)
{
	__mcuio_send_to_host(s, rt);
}

static void __mcuio_send_error_to_host(const struct statemachine_state *s,
				       struct statemachine_runtime *rt,
				       int e)
{
	struct mcuio_data *d = rt->priv;
	d->input_packet.data[0] = e;
	mcuio_packet_set_reply(&d->input_packet);
	pipe_write(d->output_pipe, (const char *)&d->input_packet,
		   sizeof(d->input_packet));
}

static void mcuio_send_error_to_host(const struct statemachine_state *s,
				     struct statemachine_runtime *rt)
{
	__mcuio_send_error_to_host(s, rt, ENXIO);
}

static void mcuio_send_busy_to_host(const struct statemachine_state *s,
				    struct statemachine_runtime *rt)
{
	__mcuio_send_error_to_host(s, rt, EBUSY);
}

static void __mcuio_send_error_to_function(const struct statemachine_state *s,
					   struct statemachine_runtime *rt,
					   int err)
{
	struct mcuio_data *d = rt->priv;
	int func = rt - d->smr;
	struct mcuio_function *f = &mcuio_functions_start[func];
	if (f->ops->reply)
		f->ops->reply(f, err);
}

static void mcuio_send_busy_to_function(const struct statemachine_state *s,
					struct statemachine_runtime *rt)
{
	__mcuio_send_error_to_function(s, rt, EBUSY);
}

static void mcuio_send_reply_to_function(const struct statemachine_state *s,
					 struct statemachine_runtime *rt)
{
	struct mcuio_data *d = rt->priv;
	int func = rt - d->smr;
	struct mcuio_function *f = &mcuio_functions_start[func];
	*f->from_host = d->input_packet;
	if (f->ops->reply)
		f->ops->reply(f, 0);
}

static void mcuio_send_error_to_function(const struct statemachine_state *s,
					 struct statemachine_runtime *rt)
{
	__mcuio_send_error_to_function(s, rt, EIO);
}

static const struct statemachine mcuio_statemachine = {
	.type = MEALEY,
	.nstates = NSTATES,
	.states = mcuio_states,
	.nevents = NEVENTS,
	.runtimes = global_data.smr,
};

static int mcuio_init(void *arg)
{
	struct mcuio_data *data = arg;
	if (!data)
		return -1;
	data->smr->priv = data;
	if (init_statemachine(&mcuio_statemachine, IDLE) < 0) {
		printf("mcuio: error initializing state machine\n");
		return -1;
	}
	data->input_pipe = pipe_open("mcuio_input", BATHOS_MODE_INPUT, data);
	if (!data->input_pipe)
		data->input_pipe = bathos_stdin;
	if (!data->input_pipe) {
		printf("mcuio: error opening input pipe\n");
		return -1;
	}
	data->output_pipe = pipe_open("mcuio_output", BATHOS_MODE_OUTPUT, data);
	if (!data->output_pipe)
		data->output_pipe = bathos_stdout;
	if (!data->output_pipe) {
		printf("mcuio: error opening output pipe\n");
		return -1;
	}
	printf("mcuio init ok\n");
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
	const struct statemachine *m = &mcuio_statemachine;
	struct mcuio_base_packet *packet;
	int stat;
	struct mcuio_data *data = ed->priv;
	pipe = ed->evt->data;
	packet = &data->input_packet;
	stat = pipe_read(pipe, (char *)packet, sizeof(packet));
	if (stat < sizeof(packet))
		return;
	dump_packet(packet);
	feed_statemachine(m, packet->func, REQUEST_FROM_HOST);
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
	const struct statemachine *m = &mcuio_statemachine;
	struct mcuio_function *f = ed->evt->data;
	feed_statemachine(m, f - mcuio_functions_start, REQUEST_FROM_FUNCTION);
}


declare_event_handler_with_priv(mcuio_function_request, NULL,
				mcuio_function_request_handle,
				NULL, &global_data);

static void mcuio_function_reply_handle(struct event_handler_data *ed)
{
	struct mcuio_data *data = ed->priv;
	const struct statemachine *m = &mcuio_statemachine;
	struct mcuio_function *f = ed->evt->data;
	feed_statemachine(m, f - mcuio_functions_start, REPLY_FROM_FUNCTION);
}


declare_event_handler_with_priv(mcuio_function_reply, NULL,
				mcuio_function_reply_handle,
				NULL, &global_data);
