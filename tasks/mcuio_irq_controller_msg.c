#include <arch/hw.h>
#include <bathos/gpio.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/gpio.h>
#include <generated/autoconf.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

#define DEBUG

static struct mcuio_function_runtime irq_ctrl_msg_runtime;

static const struct mcuio_func_descriptor PROGMEM irq_controller_msg_descr = {
	.device = 0x5556,
	.vendor = 0x0001,
	.rev = 0,
	/* Irq controller (msg) class */
	.class = 0x0000000a,
};

static const unsigned int PROGMEM irq_controller_msg_descr_length =
    sizeof(irq_controller_msg_descr);

static const struct mcuio_range PROGMEM irq_controller_msg_ranges[] = {
	/* IRQ controller (msg) func descriptor */
	{
		.start = 0,
		.length = &irq_controller_msg_descr_length,
		.rd_target = &irq_controller_msg_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
};


static void irq_ctrl_msg_on_reply(struct mcuio_base_packet *p,
				  struct mcuio_function *f,
				  int err)
{
	if (err)
		printf("WARNING: lost interrupt, hc reported error\n");
	return;
}

static const struct mcuio_function_ops irq_controller_msg_func_ops = {
	.reply = irq_ctrl_msg_on_reply,
};

declare_mcuio_function(irq_controller_msg, irq_controller_msg_ranges,
		       NULL, &irq_controller_msg_func_ops,
		       &irq_ctrl_msg_runtime);

static int mcuio_irq_init(struct event_handler_data *edata)
{
	return 0;
}

static void mcuio_irq_handle(struct event_handler_data *edata)
{
	struct mcuio_function_irq_data *idata = edata->data;
	struct mcuio_base_packet *p = &irq_ctrl_msg_runtime.to_host;

	if (idata->func > 31)
		return;
	if (!idata->active)
		return;
	/*
	 * Send a message to the host controller telling it that an
	 * interrupt has been requested
	 */
	memset(p, 0, sizeof(*p));
	p->bus = 0; /* HACK, make this my bus */
	p->dev = 0;
	/* FIXME: replace 1 with my dev number */
	p->func = 1;
	p->offset = 0xf80 + idata->func * sizeof(uint32_t);
	p->type = mcuio_type_wrdw;
	p->data[0] = (1 << idata->func);
	if (trigger_event_immediate(&evt_mcuio_function_request, p,
				    EVT_PRIO_MAX) < 0)
		printf("WARNING: LOST INTERRUPT\n");
}

declare_event_handler(mcuio_irq, mcuio_irq_init, mcuio_irq_handle, NULL);
