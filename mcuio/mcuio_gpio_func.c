/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <arch/hw.h>
#include <arch/gpio.h>
#include <bathos/gpio.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/errno.h>
#include <bathos/bitops.h>
#include <bathos/string.h>
#include <tasks/mcuio.h>

#include "mcuio-function.h"
#include "mcuio_gpio_func.h"

extern struct mcuio_function gpio;
declare_extern_event(gpio_evt);
declare_extern_event(mcuio_irq);

#ifdef ARCH_IS_HARVARD
static inline const struct mcuio_gpio_port_data *
__get_port_data(const struct mcuio_gpio_port_data * PROGMEM pd,
		struct mcuio_gpio_port_data *out)
{
	memcpy_P(out, pd, sizeof(*out));
	return out;
}
#else
static inline const struct mcuio_gpio_port_data *
__get_port_data(const struct mcuio_gpio_port_data * PROGMEM pd,
		struct mcuio_gpio_port_data *out)
{
	return pd;
}
#endif

/* WARNING: ASSUMES width = 8 or 16 or 32 */
static int __gpio_data_wr(const struct mcuio_range *r, unsigned offset,
			  const void *__in, int fill, int width)
{
	int n = width, i;
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd =
		__get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	int start = pd->gpio_start + offset * 8;

	if (fill)
		n = min(ngpio, sizeof(uint64_t));
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
	int n = width, i;
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	int start = pd->gpio_start + offset * 8;

	if (fill)
		n = min(ngpio, sizeof(uint64_t));
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
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	uint8_t status[8];

	if (fill)
		n = min(ngpio, sizeof(uint64_t)*8);
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
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	uint8_t status[8];

	if (fill)
		n = min(ngpio, sizeof(uint64_t)*8);
	if (__gpio_data_rd(r, offset, &status, fill, width) < 0)
		return -1;
	for (i = 0; i < n/8; i++)
		status[i] |= ((uint8_t *)__in)[i];
	return __gpio_data_wr(r, offset, status, fill, width);
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
	int n = width/8, i, ret;
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	int start = offset + pd->gpio_start;

	if (fill)
		n = min(ngpio, sizeof(uint64_t));
	ret = n;
	for (i = 0; i < n && ret > 0; i++) {
		uint8_t in = ((uint8_t *)__in)[i];
		if ((in & (INPUT|OUTPUT)) == (INPUT|OUTPUT)) {
			ret = -EINVAL;
			break;
		}
		/* PULLUP, PULLDOWN and ODRAIN are presently ignored */
		gpio_dir_af(i + start, in & OUTPUT, gpio_get(i + start),
			    gpio_to_af(i + start));
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
	int n = width, i;
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	int start = pd->gpio_start + offset, ret, dir = 0;

	if (fill)
		n = min(ngpio, sizeof(uint64_t)/8);
	ret = n;
	for (i = 0; i < n && ret > 0; i++) {
		uint8_t *out = &((uint8_t *)__out)[i];
		gpio_get_dir_af(i + start, &dir, NULL, NULL);
		*out = dir ? OUTPUT : INPUT;
	}
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
	int n = width/8, i;
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	int start = offset + pd->gpio_start, ret;
	uint8_t *out = __out;
	struct mcuio_gpio_port_status *s = pd->status;
	if (fill)
		n = min(ngpio, sizeof(uint64_t)/8);
	ret = n;
	for (i = 0; i < n && ret > 0; i++, out++) {
		*out = 0;
		if (test_bit(i + start, s->gpio_events_falling))
			*out |= GPIO_EVT_FALLING;
		if (test_bit(i + start, s->gpio_events_rising))
			*out |= GPIO_EVT_RISING;
		if (test_bit(i + start, s->gpio_events_high))
			*out |= GPIO_EVT_HIGH;
		if (test_bit(i + start, s->gpio_events_low))
			*out |= GPIO_EVT_LOW;
		if (test_bit(i + start, s->gpio_events_enable))
			*out |= GPIO_EVT_ENABLE;
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
	int n = width/8, i;
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	int start = pd->gpio_start + offset, ret, stat;
	const uint8_t *in = __in;
	struct mcuio_gpio_port_status *s = pd->status;
	if (fill)
		n = min(ngpio, sizeof(uint64_t)/8);
	ret = n;
	for (i = 0; i < n; i++, in++) {
		if (*in & GPIO_EVT_FALLING)
			set_bit(i + start, s->gpio_events_falling);
		if (*in & GPIO_EVT_RISING)
			set_bit(i + start, s->gpio_events_rising);
		if (*in & GPIO_EVT_HIGH)
			set_bit(i + start, s->gpio_events_high);
		if (*in & GPIO_EVT_LOW)
			set_bit(i + start, s->gpio_events_low);
		if (*in & GPIO_EVT_ENABLE)
			set_bit(i + start, s->gpio_events_enable);
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

static void __handle_level_gpio_events(const struct mcuio_gpio_port_data *pd)
{
	struct mcuio_gpio_port_status *s = pd->status;
	uint32_t status[2];
	int i;

	for (i = 0; i < 2; i++) {
		gpio_data_rddw(NULL, i * 4, &status[i], 0);
		s->gpio_events_status[i] |= status[i] & s->gpio_events_high[i];
		s->gpio_events_status[i] |= ~status[i] & s->gpio_events_low[i];
	}
}

static int __gpio_evts_status_rd(const struct mcuio_range *r, unsigned offset,
				 void *__out, int fill, int width)
{
	int n = width/8, i;
	const struct mcuio_gpio_port_data * PROGMEM _pd = r->priv;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	int ngpio = pd->gpio_end - pd->gpio_start + 1;
	int start = offset + pd->gpio_start, ret;
	static struct mcuio_function_irq_data idata;
	uint32_t prev_gpio_events_status[2];
	uint32_t *gpio_events_status = pd->status->gpio_events_status;
	uint8_t *out = __out, *ptr = &((uint8_t *)gpio_events_status)[start];
	if (fill)
		n = min(ngpio, sizeof(uint64_t)/8);
	ret = n;
	memcpy(prev_gpio_events_status, gpio_events_status,
	       sizeof(prev_gpio_events_status));
	for (i = 0; i < n && ret > 0; i++, out++, ptr++) {
		/* Read and automatically clear */
		*out = *ptr;
		*ptr = 0;
	}

	__handle_level_gpio_events(pd);

	if ((prev_gpio_events_status[0] || prev_gpio_events_status[1]) &&
	    (!gpio_events_status[0] && !gpio_events_status[1])) {
		idata.func = pd->function - mcuio_functions_start;
		idata.active = 0;
		if (trigger_event(&event_name(mcuio_irq), &idata))
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

/* We assume edata->evt_status is 64 bits long, same size as an mcuio port */
void gpio_evt_handle(struct event_handler_data *ed,
		     const struct mcuio_gpio_port_data * PROGMEM _pd)
{
	struct gpio_event_data *edata = ed->data;
	uint32_t *evt_status = edata->evt_status;
	int gpio_offset = edata->gpio_offset;
	struct mcuio_gpio_port_data __pd;
	const struct mcuio_gpio_port_data *pd = __get_port_data(_pd, &__pd);
	struct mcuio_gpio_port_status *s = pd->status;
	static struct mcuio_function_irq_data idata;
	int i;

	if (gpio_offset != pd->gpio_start)
		/* Not mine. */
		return;

	for (i = 0; i < 2; i++) {
		s->gpio_events_status[i] |= (evt_status[i] &
			~s->gpio_events_high[i] & ~s->gpio_events_low[i]);
		evt_status[i] = 0;
	}

	__handle_level_gpio_events(pd);

	idata.func = pd->function - mcuio_functions_start;
	idata.active = s->gpio_events_status[0] ||
		s->gpio_events_status[1] ? 1 : 0;
	trigger_event(&event_name(mcuio_irq), &idata);
}
