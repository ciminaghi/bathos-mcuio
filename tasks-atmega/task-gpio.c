#include <arch/hw.h>
#include <arch/bathos-arch.h>
#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/gpio.h>
#include <bathos/bitops.h>
#include <bathos/errno.h>

static uint8_t ports[1];
static uint8_t rising[1];
static uint8_t falling[1];
static uint8_t enable[1];

static uint32_t events[2];

declare_event(gpio_evt);

static void *gpio_sample(void *arg)
{
	int i, j;
	uint8_t s, d, m;
	int addr, do_trigger = 0;
	for (i = 0; i < sizeof(ports); i++) {
		if (!rising[i] && !falling[i])
			continue;
		addr = PORTS_BASE + i * 3;
		s = _SFR_IO8(addr);
		d = s ^ ports[i];
		for (m = 1, j = 0; d; m <<= 1, j++) {
			if ((enable[i] & m) && (rising[i] & m) &&
			    !(ports[i] & m)) {
				/* Rising edge */
				do_trigger = 1;
				//printf("redge (%d)\n", i * 8 + j);
				set_bit(i * 8 + j, events);
			}
			if ((enable[i] & m) && (falling[i] & m) &&
			    (ports[i] & m)) {
				/* Falling edge */
				do_trigger = 1;
				//printf("fedge (%d)\n", i * 8 + j);
				set_bit(i * 8 + j, events);
			}
			/* Store previous status */
			ports[i] = s;
			d &= ~m;
		}
	}
	if (do_trigger)
		trigger_event(&event_name(gpio_evt), events, EVT_PRIO_MAX);
	return arg;
}

static struct bathos_task __task t_mcuio = {
	.name = "gpio", .period = HZ/100,
	.job = gpio_sample, .arg = NULL,
	.init = NULL,
	.release = 20,
};


static int gpio_request_events(int gpio, int flags)
{
	uint8_t mask, port;
	if (gpio < 0 || gpio > 39)
		return -EINVAL;
	printf("%s: gpio = %d, flags = %d\n", __func__, gpio, flags);
	mask = 1 << GPIO_BIT(gpio);
	port = GPIO_PORT(gpio);
	rising[port] &= ~mask;
	falling[port] &= ~mask;
	enable[port] &= ~mask;
	if (flags & GPIO_EVT_RISING)
		rising[port] |= mask;
	if (flags & GPIO_EVT_FALLING)
		falling[port] |= mask;
	if (flags & GPIO_EVT_ENABLE)
		enable[port] |= mask;
	return -ENOSYS;
}
