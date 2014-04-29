#include <arch/hw.h>
#include <bathos/gpio.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/gpio.h>
#include <generated/autoconf.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

#ifndef CONFIG_MCUIO_IRQ_WIRE_GPIO_NUMBER
#define IRQ_LINE_GPIO 30
#else
#define IRQ_LINE_GPIO CONFIG_MCUIO_IRQ_WIRE_GPIO_NUMBER
#endif

#ifndef CONFIG_MCUIO_IRQ_WIRE_MPU_GPIO_NUMBER
#define MPU_IRQ_LINE_GPIO 19
#else
#define MPU_IRQ_LINE_GPIO CONFIG_MCUIO_IRQ_WIRE_GPIO_NUMBER
#endif

#ifndef CONFIG_MCUIO_IRQ_WIRE_ACTIVE_LEVEL
#define IRQ_ACTIVE_LEVEL 0
#else
#define IRQ_ACTIVE_LEVEL CONFIG_MCUIO_IRQ_WIRE_ACTIVE_LEVEL
#endif

#define IRQ_INACTIVE_LEVEL ((IRQ_ACTIVE_LEVEL) ? 0 : 1)

#define STATUS_OFFSET 0x00
#define MASK_OFFSET   0x04
#define UNMASK_OFFSET 0x08
#define ACK_OFFSET    0x0c

static inline void __activate_irq_line(int active)
{
	int l = active ? IRQ_ACTIVE_LEVEL : IRQ_INACTIVE_LEVEL;
	printf("gpio%d->%d\n", IRQ_LINE_GPIO, l);
	gpio_set(IRQ_LINE_GPIO, l);
}

static struct mcuio_irq_controller_wire_data {
	uint32_t registers[4];
	uint32_t requested_status;
} data = {
	/* All irqs masked on startup */
	.registers = { 0, 0xffffffff, 0, 0, },
	.requested_status = 0,
};

static const unsigned int PROGMEM irq_controller_registers_length =
	sizeof(data.registers);

static const struct mcuio_func_descriptor PROGMEM irq_controller_wire_descr = {
	.device = CONFIG_MCUIO_IRQ_CONTROLLER_WIRE_DEVICE,
	.vendor = CONFIG_MCUIO_IRQ_CONTROLLER_WIRE_VENDOR,
	.rev = 0,
	/* Irq controller (wire) class */
	.class = 0x0000000b,
};

static const unsigned int PROGMEM irq_controller_wire_descr_length =
    sizeof(irq_controller_wire_descr);

static const uint32_t PROGMEM cfg_area[] = {
	[0] = MPU_IRQ_LINE_GPIO,
	[1] = 0,
};

static const unsigned int PROGMEM cfg_area_length = sizeof(cfg_area);

static int mcuio_irqw_rddw(const struct mcuio_range *r,
			   unsigned offset, uint32_t *out,
			   int fill)
{
	if (fill)
		return -EPERM;
	if (offset % sizeof(uint32_t))
		return -EINVAL;
	*out = data.registers[(offset/sizeof(uint32_t))];
	return sizeof(uint32_t);
}

static int mcuio_irqw_wrdw(const struct mcuio_range *r, unsigned offset,
			   const uint32_t *in, int fill)
{
	if (fill)
		return -EPERM;
	switch (offset) {
	case STATUS_OFFSET:
		/* status, read only */
		return -EPERM;
	case MASK_OFFSET:
	case UNMASK_OFFSET:
	{
		uint32_t old_status =
			data.registers[STATUS_OFFSET/sizeof(uint32_t)];
		uint32_t new_status, new_mask;
		/* Mask register */
		new_mask = offset == MASK_OFFSET ?
		    data.registers[MASK_OFFSET/sizeof(uint32_t)] | *in :
		    data.registers[MASK_OFFSET/sizeof(uint32_t)] & ~*in;
		data.registers[MASK_OFFSET/sizeof(uint32_t)] = new_mask;
		new_status = data.requested_status & ~new_mask;
		if (!(new_status ^ old_status))
			/* No change, done */
			return sizeof(uint32_t);
		__activate_irq_line(new_status != 0);
		data.registers[STATUS_OFFSET/sizeof(uint32_t)] = new_status;
		return sizeof(uint32_t);
	}
	case ACK_OFFSET:
	{
		uint32_t old_status =
			data.registers[STATUS_OFFSET/sizeof(uint32_t)];
		uint32_t new_status =
			(old_status & ~*in) | data.requested_status;
		if (!(new_status ^ old_status))
			/* No change, done */
			return sizeof(uint32_t);
		__activate_irq_line(new_status != 0);
		data.registers[STATUS_OFFSET/sizeof(uint32_t)] = new_status;
		return sizeof(uint32_t);
	}
	default:
		return -EPERM;
	}
	return -EPERM;
}

const struct mcuio_range_ops PROGMEM registers_ops = {
	.rd = { mcuio_rdb, mcuio_rdw, mcuio_irqw_rddw, mcuio_rdq, },
	/* Only 32bits write is supported */
	.wr = { NULL, NULL, mcuio_irqw_wrdw, NULL, },
};

static const struct mcuio_range PROGMEM irq_controller_wire_ranges[] = {
	/* IRQ controller (wire) func descriptor */
	{
		.start = 0,
		.length = &irq_controller_wire_descr_length,
		.rd_target = &irq_controller_wire_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	{
		.start = 8,
		.length = &irq_controller_registers_length,
		.rd_target = data.registers,
		.wr_target = data.registers,
		.ops = &registers_ops,
	},
	{
		.start = 0x18,
		.length = &cfg_area_length,
		.rd_target = cfg_area,
		.wr_target = NULL,
		.ops = &default_mcuio_range_ro_ops,
	},
};

declare_mcuio_function(irq_controller_wire, irq_controller_wire_ranges,
		       NULL, NULL, &mcuio_func_common_runtime);

static int mcuio_irq_init(struct event_handler_data *edata)
{
	return gpio_dir_af(IRQ_LINE_GPIO, 1, IRQ_INACTIVE_LEVEL, 0);
}

static void mcuio_irq_handle(struct event_handler_data *edata)
{
	struct mcuio_function_irq_data *idata = edata->data;
	uint32_t old_requested_status, new_status;
	printf("%s, idata = %p\n", __func__, idata);
	if (idata->func > 31)
		return;
	old_requested_status = data.requested_status;
	data.requested_status &= ~(1 << idata->func);
	if (idata->active)
		data.requested_status |= (1 << idata->func);
	if (!(old_requested_status ^ data.requested_status)) {
		/* No change in requested status, return immediately */
		printf("%s %d\n", __func__, __LINE__);
		return;
	}
	new_status = data.requested_status &
		~(data.registers[MASK_OFFSET/sizeof(uint32_t)]);
	printf("r = 0x%08x\n", data.requested_status);
	printf("m = 0x%08x\n", ~data.registers[MASK_OFFSET/sizeof(uint32_t)]);
	if (!(new_status ^ data.registers[STATUS_OFFSET/sizeof(uint32_t)])) {
		printf("%s %d\n", __func__, __LINE__);
		return;
	}
	data.registers[STATUS_OFFSET/sizeof(uint32_t)] = new_status;
	if (new_status) {
		/* automatically mask irq */
		data.registers[MASK_OFFSET/sizeof(uint32_t)] |=
			(1 << idata->func);
	}
	__activate_irq_line(new_status != 0);
}

declare_event_handler(mcuio_irq, mcuio_irq_init, mcuio_irq_handle, NULL);

#if defined CONFIG_MCUIO_TEST_IRQ

declare_event(mcuio_irq);

static void __pipe_input_handle(struct event_handler_data *ed)
{
	struct bathos_pipe *pipe = ed->data;
	static struct mcuio_function_irq_data idata;

	if (list_empty(&pipe->dev->pipes))
		/* Not opened */
		return;

	if (pipe != bathos_stdin)
		return;

	idata.func = 2;
	idata.active = 1;
	trigger_event(&event_name(mcuio_irq), &idata, EVT_PRIO_MAX);
}

declare_event_handler(pipe_input_ready, NULL, __pipe_input_handle, NULL);

#endif /* CONFIG_MCUIO_TEST_IRQ */
