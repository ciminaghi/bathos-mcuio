/*
 * Onewire gpio-based operations
 * Alessandro Rubini, 2013 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/delay.h>
#define __W1_INTERNAL__
#include <bathos/w1.h>
#include <arch/gpio.h>

/* Private methods for acting on bits */
static inline void set_w1_gpio(int gpio, int value)
{
	gpio_dir(gpio, 0, 0); /* input for 5usec (recover prev bit) */
	udelay(5);
	gpio_dir(gpio, 1, 0); /* output low */
	udelay(value ? 1 : 60);
	gpio_dir(gpio, 0, 0); /* input again */
	udelay(value ? 60 : 1);
}

static inline int get_w1_gpio(int gpio)
{
	int ret;

	gpio_dir(gpio, 1, 0); /* output low */
	udelay(1);
	gpio_dir(gpio, 0, 0); /* input (pull up) */
	udelay(9);
	ret = gpio_get(gpio);
	udelay(60);
	return ret;
}

/* Public methods follow */
static int w1_gpio_reset(struct w1_bus *bus)
{
	int ret;

	gpio_dir_af(bus->detail, 0, 0, 0); /* input, novalue, af = 0 */
	gpio_dir(bus->detail, 1, 0); /* low pulse */
	udelay(480);
	gpio_dir(bus->detail, 0, 0); /* input */
	udelay(100);
	ret = !gpio_get(bus->detail); /* 0 == present */
	udelay(480);
	if (!gpio_get(bus->detail))
		return 0; /* stuck low: not present */
	return ret;
}

static int w1_gpio_read_bit(struct w1_bus *bus)
{
	return get_w1_gpio(bus->detail);
}

static void w1_gpio_write_bit(struct w1_bus *bus, int bit)
{
	set_w1_gpio(bus->detail, bit);

}

struct w1_ops bathos_w1_ops __attribute__((weak)) = {
	.reset = w1_gpio_reset,
	.read_bit = w1_gpio_read_bit,
	.write_bit = w1_gpio_write_bit,
};

/*
 * provide the "official" name as a weak alias, so it takes over if
 * somebody uses onewrire and there is no specific driver built.
 */
//struct w1_ops __attribute__((weak, alias("w1_gpio_ops"))) bathos_w1_ops;
