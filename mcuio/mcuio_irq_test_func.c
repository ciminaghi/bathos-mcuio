#include <arch/hw.h>
#include <bathos/pwm.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <bathos/init.h>
#include <bathos/jiffies.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

#define MCUIO_IRQ_TEST_DEVICE 0x1212
#define MCUIO_IRQ_TEST_VENDOR 0x0001

declare_extern_event(mcuio_irq);

struct mcuio_function irq_test;

static const struct mcuio_func_descriptor PROGMEM irq_test_descr = {
	.device = MCUIO_IRQ_TEST_DEVICE,
	.vendor = MCUIO_IRQ_TEST_VENDOR,
	.rev = 0,
	.class = 0x0000000d,
};

static uint8_t irq_active;
static uint8_t irq_enabled;

static const unsigned int PROGMEM irq_test_descr_length =
	sizeof(irq_test_descr);
static const unsigned int PROGMEM single_reg_length = 4;

static int irq_test_status_rddw(const struct mcuio_range *r, unsigned offset,
				uint32_t *__out, int fill)
{
	*__out = irq_active != 0;
	if (irq_active) {
		/* Auto clear interrupt */
		static struct mcuio_function_irq_data idata;

		irq_active = 0;
		idata.func = &irq_test - mcuio_functions_start;
		idata.active = irq_active;
		trigger_event(&event_name(mcuio_irq), &idata);
	}
	return sizeof(*__out);
}

static int irq_test_status_wrdw(const struct mcuio_range *r, unsigned offset,
				const uint32_t *__in, int fill)
{
	irq_enabled = (*__in != 0);
	return sizeof(*__in);
}


const struct mcuio_range_ops PROGMEM irq_test_status_ops = {
	.rd = { NULL, NULL, irq_test_status_rddw, NULL, },
	.wr = { NULL, NULL, irq_test_status_wrdw, NULL, },
};

static const struct mcuio_range PROGMEM irq_test_ranges[] = {
	/* irq test func descriptor */
	{
		.start = 0,
		.length = &irq_test_descr_length,
		.rd_target = &irq_test_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* irq control/status register */
	{
		.start  = 0xc,
		.length = &single_reg_length,
		.ops = &irq_test_status_ops,
	}
};

declare_mcuio_function(irq_test, irq_test_ranges, NULL, NULL,
		       &mcuio_func_common_runtime);

static void hwtimer_evt_handle(struct event_handler_data *ed)
{
	static struct mcuio_function_irq_data idata;

	if (jiffies % CONFIG_MCUIO_IRQ_TEST_HW_TICKS)
		return;
	if (!irq_enabled)
		return;
	if (irq_active)
		return;
	irq_active = 1;
	idata.func = &irq_test - mcuio_functions_start;
	idata.active = irq_active;
	trigger_event(&event_name(mcuio_irq), &idata);
}

declare_event_handler(hw_timer_tick, NULL, hwtimer_evt_handle, NULL);
