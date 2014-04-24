#include <arch/hw.h>
#include <bathos/gpio.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"

extern struct mcuio_function gpio;
declare_extern_event(gpio_evt);
declare_extern_event(mcuio_irq);


extern unsigned int PROGMEM gpio_labels_size;
extern const char PROGMEM gpio_labels_start[];
extern const unsigned int PROGMEM gpio_caps_size;
extern const uint32_t PROGMEM gpio_caps_start[];
extern const unsigned int PROGMEM gpio_evts_caps_size;
extern const uint32_t PROGMEM gpio_evts_caps_start[];

static const unsigned int PROGMEM gpio_evts_status_length = 8;
static const unsigned int PROGMEM gpio_data_length = 8;
static const unsigned int PROGMEM gpio_modes_size = 0x200;
static const unsigned int PROGMEM gpio_evts_masks_size = 0x200;
static const unsigned int PROGMEM __gpio_labels_size =
	(const unsigned int)&gpio_labels_size;
static const unsigned int PROGMEM __gpio_caps_size =
	(const unsigned int)&gpio_caps_size;
static const unsigned int PROGMEM __gpio_evts_caps_size =
	(const unsigned int)&gpio_evts_caps_size;

static const struct mcuio_func_descriptor PROGMEM gpio_descr = {
	.device = CONFIG_MCUIO_GPIO_DEVICE,
	.vendor = CONFIG_MCUIO_GPIO_VENDOR,
	.rev = 0,
	/* GPIOs class */
	.class = 0x00000002,
};

static const unsigned int PROGMEM gpio_descr_length = sizeof(gpio_descr);

/* FIXME !! Does not work in case of multiple instances */
static uint32_t gpio_events_status[2];

static uint32_t gpio_events_falling[2];
static uint32_t gpio_events_rising[2];
static uint32_t gpio_events_enable[2];

static uint32_t PROGMEM gpio_ro_range1[] = {
	/* FIXME: MAKE THIS CONFIGURABLE !! */
    [0] = 'T' | (((unsigned long)'E') << 8) | (((unsigned long)'S') << 16) | \
    (((unsigned long)'T') << 24),
	/* Number of gpios is defined on the command line */
	[1] = MCUIO_NGPIO,
};

static const unsigned int PROGMEM gpio_ro_range1_length =
	sizeof(gpio_ro_range1);

static struct mcuio_function_runtime gpio_rt;

/* WARNING: ASSUMES width = 8 or 16 or 32 */
static int __gpio_data_wr(const struct mcuio_range *r, unsigned offset,
			  const void *__in, int fill, int width)
{
	int start = offset * 8, n = width, i;

	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t));
	for (i = 0; i < n; i += width) {
		if (__gpio_set_portw(start + i, (__in + i/8), width) < 0) {
			/*
			 * Cannot handle all bits at once,
			 * fall back to slow implementation
			 */
			int j;
			uint32_t mask = 1;
			for (j = 0; j < width; j++, mask <<= 1) {
				uint32_t in;
				switch(width) {
				case 8:
					in = *((uint8_t *)__in + i/8);
					break;
				case 16:
					in = *((uint16_t *)__in + i/16);
					break;
				case 32:
					in = *((uint32_t *)__in + i/32);
					break;
				default:
					return -EINVAL;
				}
				gpio_set(i + j + start, (in & mask) ? 1 : 0);
			}
		}
	}
	return n;
}


static int gpio_data_wrb(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_data_wr(r, offset, __in, fill, 8);
}

static int gpio_data_wrw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_data_wr(r, offset, __in, fill, 16);
}

static int gpio_data_wrdw(const struct mcuio_range *r, unsigned offset,
			  const uint32_t *__in, int fill)
{
	return __gpio_data_wr(r, offset, __in, fill, 32);
}

static int gpio_data_wrq(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *in, int fill)
{
	return -EPERM;
}

static int __gpio_data_rd(const struct mcuio_range *r, unsigned offset,
			  void *__out, int fill, int width)
{
	int start = offset * 8, n = width, i;

	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t));
	for (i = 0; i < n; i += width) {
		if (__gpio_get_portw(start + i, (__out + i/8), width) < 0) {
			/*
			 * Cannot handle all bits at once,
			 * fall back to slow implementation
			 */
			int j;
			uint32_t p;
			for (j = 0, p = 0; j < width; j++)
				p |= (((uint32_t)gpio_get(i + j + start)) << j);
			switch(width) {
			case 8:
			    *((uint8_t *)__out + i/8) = p;
			    break;
			case 16:
			    *((uint16_t *)__out + i/16) = p;
			    break;
			case 32:
			    *((uint32_t *)__out + i/32) = p;
			    break;
			default:
			    return -EINVAL;
			}
		}
	}
	return n;
}

static int gpio_data_rdb(const struct mcuio_range *r, unsigned offset,
			 uint32_t *__out, int fill)
{
	return __gpio_data_rd(r, offset, __out, fill, 8);
}

static int gpio_data_rdw(const struct mcuio_range *r, unsigned offset,
			 uint32_t *__out, int fill)
{
	return __gpio_data_rd(r, offset, __out, fill, 16);
}

static int gpio_data_rddw(const struct mcuio_range *r, unsigned offset,
			  uint32_t *__out, int fill)
{
	return __gpio_data_rd(r, offset, __out, fill, 32);
}

static int gpio_data_rdq(const struct mcuio_range *r, unsigned offset,
			 uint32_t *__out, int fill)
{
	return -EPERM;
}

const struct mcuio_range_ops PROGMEM gpio_data_ops = {
	.rd = { gpio_data_rdb, gpio_data_rdw, gpio_data_rddw, gpio_data_rdq, },
	.wr = { gpio_data_wrb, gpio_data_wrw, gpio_data_wrdw, gpio_data_wrq, },
};


static int __gpio_set_wr(const struct mcuio_range *r, unsigned offset,
			 const void *__in, int fill, int width)
{
	int n = width, i;
	uint8_t status[8];

	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t)*8);
	if (__gpio_data_rd(r, offset, &status, fill, width) < 0)
		return -1;
	for (i = 0; i < n/8; i++)
		status[i] |= ((uint8_t *)__in)[i];
	return __gpio_data_wr(r, offset, status, width, fill);
}


static int gpio_set_wrb(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_data_wr(r, offset, __in, fill, 8);
}

static int gpio_set_wrw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_set_wr(r, offset, __in, fill, 16);
}

static int gpio_set_wrdw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_set_wr(r, offset, __in, fill, 32);
}

static int gpio_set_wrq(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *in, int fill)
{
	return -EPERM;
}

const struct mcuio_range_ops PROGMEM gpio_set_ops = {
	.rd = { NULL, NULL, NULL, },
	.wr = { gpio_set_wrb, gpio_set_wrw, gpio_set_wrdw, gpio_set_wrq, },
};

static int __gpio_clr_wr(const struct mcuio_range *r, unsigned offset,
			 const void *__in, int fill, int width)
{
	int n = width, i;
	uint8_t status[8];

	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t)*8);
	if (__gpio_data_rd(r, offset, &status, fill, width) < 0)
		return -1;
	for (i = 0; i < n/8; i++)
		status[i] |= ((uint8_t *)__in)[i];
	return __gpio_data_wr(r, offset, status, width, fill);
}


static int gpio_clr_wrb(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_data_wr(r, offset, __in, fill, 8);
}

static int gpio_clr_wrw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_clr_wr(r, offset, __in, fill, 16);
}

static int gpio_clr_wrdw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_clr_wr(r, offset, __in, fill, 32);
}

static int gpio_clr_wrq(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *in, int fill)
{
	return -EPERM;
}

const struct mcuio_range_ops PROGMEM gpio_clr_ops = {
	.rd = { NULL, NULL, NULL, },
	.wr = { gpio_clr_wrb, gpio_clr_wrw, gpio_clr_wrdw, gpio_clr_wrq, },
};


#define INPUT  (1 << 0)
#define OUTPUT (1 << 1)
#define PULLUP (1 << 2)
#define PULLDOWN (1 << 3)
#define ODRAIN (1 << 4)

static int __gpio_modes_wr(const struct mcuio_range *r, unsigned offset,
			   const void *__in, int fill, int width)
{
	int start = offset, n = width/8, i, ret;
	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t));
	ret = n;
	for (i = 0; i < n && ret > 0; i++) {
		uint8_t in = ((uint8_t *)__in)[i];
		if ((in & (INPUT|OUTPUT)) == (INPUT|OUTPUT)) {
			ret = -EINVAL;
			break;
		}
		/* PULLUP, PULLDOWN and ODRAIN are presently ignored */
		gpio_dir_af(i + start, in & OUTPUT, gpio_get(i + start), 0);
	}
	return ret;
}

static int gpio_modes_wrb(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_modes_wr(r, offset, __in, fill, 8);
}

static int gpio_modes_wrw(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *__in, int fill)
{
	return __gpio_modes_wr(r, offset, __in, fill, 16);
}

static int gpio_modes_wrdw(const struct mcuio_range *r, unsigned offset,
			  const uint32_t *__in, int fill)
{
	return __gpio_modes_wr(r, offset, __in, fill, 32);
}

static int gpio_modes_wrq(const struct mcuio_range *r, unsigned offset,
			 const uint32_t *in, int fill)
{
	return -EPERM;
}

static int __gpio_modes_rd(const struct mcuio_range *r, unsigned offset,
			   void *__out, int fill, int width)
{
	int start = offset, n = width/8, i, ret, dir = 0;
	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t)/8);
	ret = n;
	for (i = 0; i < n && ret > 0; i++)
		gpio_get_dir_af(i + start, &dir, NULL, NULL);
	return ret;
}

static int gpio_modes_rdb(const struct mcuio_range *r, unsigned offset,
			  uint32_t *__out, int fill)
{
	return __gpio_modes_rd(r, offset, __out, fill, 8);
}

static int gpio_modes_rdw(const struct mcuio_range *r, unsigned offset,
			  uint32_t *__out, int fill)
{
	return __gpio_modes_rd(r, offset, __out, fill, 16);
}

static int gpio_modes_rddw(const struct mcuio_range *r, unsigned offset,
			   uint32_t *__out, int fill)
{
	return __gpio_modes_rd(r, offset, __out, fill, 32);
}

static int gpio_modes_rdq(const struct mcuio_range *r, unsigned offset,
			  uint32_t *out, int fill)
{
	return -EPERM;
}

const struct mcuio_range_ops PROGMEM gpio_modes_ops = {
	.rd = { gpio_modes_rdb, gpio_modes_rdw, gpio_modes_rddw,
		gpio_modes_rdq,},
	.wr = { gpio_modes_wrb, gpio_modes_wrw, gpio_modes_wrdw,
		gpio_modes_wrq, },
};

static int __gpio_evts_rd(const struct mcuio_range *r, unsigned offset,
			  void *__out, int fill, int width)
{
	int start = offset, n = width/8, i, ret;
	uint8_t *out = __out;
	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t)/8);
	ret = n;
	for (i = 0; i < n && ret > 0; i++, out++) {
		*out = 0;
		if (test_bit(i + start, gpio_events_falling))
			*out |= 2;
		if (test_bit(i + start, gpio_events_rising))
			*out |= 1;
		if (test_bit(i + start, gpio_events_enable))
			*out |= 0x80;
	}
	return ret;
}

static int gpio_evts_rddw(const struct mcuio_range *r, unsigned offset,
			   uint32_t *__out, int fill)
{
	return __gpio_evts_rd(r, offset, __out, fill, 32);
}

static int __gpio_evts_wr(const struct mcuio_range *r, unsigned offset,
			  const void *__in, int fill, int width)
{
	int start = offset, n = width/8, i, ret, stat;
	const uint8_t *in = __in;
	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t)/8);
	ret = n;
	for (i = 0; i < n; i++, in++) {
		if (*in & 2)
			set_bit(i + start, gpio_events_falling);
		if (*in & 1)
			set_bit(i + start, gpio_events_rising);
		if (*in & 0x80)
			set_bit(i + start, gpio_events_enable);
		stat = gpio_request_events(i + start, *in);
		if (stat < 0)
			return stat;
			
	}
	return ret;
}

static int gpio_evts_wrdw(const struct mcuio_range *r, unsigned offset,
			  const uint32_t *__in, int fill)
{
	return __gpio_evts_wr(r, offset, __in, fill, 32);
}

const struct mcuio_range_ops PROGMEM gpio_evts_ops = {
	.rd = { NULL, NULL, gpio_evts_rddw, NULL, },
	.wr = { NULL, NULL, gpio_evts_wrdw, NULL, },
};

static int __gpio_evts_status_rd(const struct mcuio_range *r, unsigned offset,
				 void *__out, int fill, int width)
{
	int start = offset, n = width/8, i, ret;
	static struct mcuio_function_irq_data idata;
	uint32_t prev_gpio_events_status[2];
	uint8_t *out = __out, *ptr = &((uint8_t *)gpio_events_status)[start];
	if (fill)
		n = min(MCUIO_NGPIO, sizeof(uint64_t)/8);
	ret = n;
	memcpy(prev_gpio_events_status, gpio_events_status,
	       sizeof(gpio_events_status));
	for (i = 0; i < n && ret > 0; i++, out++, ptr++) {
		/* Read and automatically clear */
		*out = *ptr;
		*ptr = 0;
	}
	if ((prev_gpio_events_status[0] || prev_gpio_events_status[1]) &&
	    (!gpio_events_status[0] && !gpio_events_status[1])) {
		idata.func = &gpio - mcuio_functions_start;
		idata.active = 0;
		if (trigger_event(&event_name(mcuio_irq), &idata, EVT_PRIO_MAX))
			printf("%s: evt error\n", __func__);
	}
	return ret;
}

static int gpio_evts_status_rddw(const struct mcuio_range *r, unsigned offset,
				 uint32_t *__out, int fill)
{
	return __gpio_evts_status_rd(r, offset, __out, fill, 32);
}

const struct mcuio_range_ops PROGMEM gpio_evts_status_ops = {
	.rd = { NULL, NULL, gpio_evts_status_rddw, NULL, },
	.wr = { NULL, NULL, NULL, NULL, },
};

static const struct mcuio_range PROGMEM gpio_ranges[] = {
	/* GPIO func descriptor */
	{
		.start = 0,
		.length = &gpio_descr_length,
		.rd_target = &gpio_descr,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords 0x8 and 0xc, port name and number of gpios */
	{
		.start = 8,
		.length = &gpio_ro_range1_length,
		.rd_target = (char *)gpio_ro_range1,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords 0x10 .. 0x10f, gpio labels */
	{
		.start = 0x10,
		.length = &__gpio_labels_size,
		.rd_target = gpio_labels_start,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords 0x110 .. 0x30f, gpio capabilities */
	{
		.start = 0x110,
		.length = &__gpio_caps_size,
		.rd_target = gpio_caps_start,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords 0x310 .. 0x40f, gpio events capabilities */
	{
		.start = 0x310,
		.length = &__gpio_evts_caps_size,
		.rd_target = gpio_evts_caps_start,
		.ops = &default_mcuio_range_ro_ops,
	},
	/* dwords 0x510 .. 0x70f, actual gpio modes */
	{
		.start = 0x510,
		.length = &gpio_modes_size,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_modes_ops,
	},
	/*
	  dwords 0x710 .. 0x90f, actual gpio events masks,
	  currently not implemented
	*/
	{
		.start = 0x710,
		.length = &gpio_evts_masks_size,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_evts_ops,
	},
	/* dwords 0x910 0x914, gpio data */
	{
		.start = 0x910,
		.length = &gpio_data_length,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_data_ops,
	},
	/* dwords 0x918 0x91c, gpio set */
	{
		.start = 0x918,
		.length = &gpio_data_length,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_set_ops,
	},
	/* dwords 0x920 0x924, gpio clear */
	{
		.start = 0x920,
		.length = &gpio_data_length,
		.rd_target = NULL,
		.wr_target = NULL,
		.ops = &gpio_clr_ops,
	},
	/*
	  dwords 0x928 0x92c gpio events status, currently
	  not implemented
	*/
	{
		.start = 0x928,
		.length = &gpio_evts_status_length,
		.rd_target = gpio_events_status,
		.ops = &gpio_evts_status_ops,
	},
};


declare_mcuio_function(gpio, gpio_ranges, NULL, NULL, &gpio_rt);

declare_event(mcuio_irq);

static void gpio_evt_handle(struct event_handler_data *ed)
{
	uint32_t *status = ed->data;
	static struct mcuio_function_irq_data idata;
	gpio_events_status[0] |= status[0];
	status[0] = 0;
	gpio_events_status[1] |= status[1];
	status[1] = 0;
	if (!gpio_events_status[0] && !gpio_events_status[1])
		printf("NULL gpio event\n");
	idata.func = &gpio - mcuio_functions_start;
	idata.active = 1;
	trigger_event(&event_name(mcuio_irq), &idata, EVT_PRIO_MAX);
}

declare_event_handler(gpio_evt, NULL, gpio_evt_handle, NULL);
