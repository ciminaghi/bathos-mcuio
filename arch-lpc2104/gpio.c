#include <bathos/types.h>
#include <bathos/io.h>

#include <arch/gpio.h>

/*
 * Note that only gpio_dir_af() checks the gpio is valid. Other
 * functions are expected to be called often and only after setting the mode.
 */
void gpio_init(void)
{
	/* nothing to do */
}

/* Warning: the following functions assume we have a single "port" */
void gpio_dir(int gpio, int output, int value)
{
	u32 reg;

	gpio_set(gpio, value != 0); /* value first, to avoid transients */

	reg = regs[REG_IODIR];
	if (output)
		reg |= (1 << gpio);
	else
		reg &= ~(1 << gpio);
	regs[REG_IODIR] = reg;
}


int gpio_dir_af(int gpio, int output, int value, int afnum)
{
	int shift = (gpio % 16) * 2;
	u32 reg;

	if (gpio > GPIO_MAX || gpio < 0)
		return -1;
	if (afnum > 3 || afnum < 0)
		return -1;

	/* two bits per bit, in two registers */
	if (gpio < 16)
		reg = regs[REG_PINSEL0];
	else
		reg = regs[REG_PINSEL1];
	reg &= ~(3 << shift);
	reg |= (afnum & 3) << shift;
	if (gpio < 16)
		regs[REG_PINSEL0] = reg;
	else
		regs[REG_PINSEL1] = reg;
	gpio_dir(gpio, output, value);
        return 0;
}
