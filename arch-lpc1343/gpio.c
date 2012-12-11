#include <bathos/types.h>
#include <bathos/io.h>
#include <arch/hw.h>

#define __LPC13_GPIO_INTERNALS__
#include <arch/gpio.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/*
 * This table is needed to turn gpio number to address. The hairy
 * macros are used to shrink the source code.
 */
#define CFG_REG(port, bit)  __GPIO_CFG_P ## port ## _ ## bit
#define __A(port, bit)  [GPIO_NR(port,bit)] = CFG_REG(port,bit)
static u8 gpio_addr[] = {
	__A(0, 0), __A(0, 1), __A(0, 2), __A(0, 3), __A(0, 4), __A(0, 5),
	__A(0, 6), __A(0, 7), __A(0, 8), __A(0, 9), __A(0,10), __A(0,11),
	__A(1, 0), __A(1, 1), __A(1, 2), __A(1, 3), __A(1, 4), __A(1, 5),
	__A(1, 6), __A(1, 7), __A(1, 8), __A(1, 9), __A(1,10), __A(1,11),
	__A(2, 0), __A(2, 1), __A(2, 2), __A(2, 3), __A(2, 4), __A(2, 5),
	__A(2, 6), __A(2, 7), __A(2, 8), __A(2, 9), __A(2,10), __A(2,11),
	__A(3, 0), __A(3, 1), __A(3, 2), __A(3, 3), __A(3, 4), __A(3, 5),
};

#define GPIO_MAX  (ARRAY_SIZE(gpio_addr) - 1)

/* This other table marks the ones where AF0 is swapped with AF1 */
static u16 gpio_weird[] = {
	__GPIO_WEIRDNESS_0, __GPIO_WEIRDNESS_1,
	__GPIO_WEIRDNESS_2, __GPIO_WEIRDNESS_3
};

/*
 * What follows is the public interface.
 * Note that only gpio_dir_af() checks the gpio is valid. Other
 * functions are expected to be called often and only after setting the mode.
 */
void gpio_init(void)
{
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_GPIO | REG_AHBCLKCTRL_IOCON;
}

void gpio_dir(int gpio, int output, int value)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);
	u32 reg;

	reg = readl(__GPIO_DIR(port));
	if (output)
		reg |= (1 << bit);
	else
		reg &= ~(1 << bit);

	writel (reg , __GPIO_DIR(port));

	/* After changing the direction we must re-force the value */
	if (output)
		gpio_set(gpio, value);
}

int gpio_dir_af(int gpio, int output, int value, int afnum)
{
	if (gpio > GPIO_MAX || gpio < 0)
		return -1;

	if (afnum < 2) { /* if weird bit, swap AF0 and AF1 */
		int port = GPIO_PORT(gpio);
		int bit = GPIO_BIT(gpio);
		if (gpio_weird[port] & (1 << bit))
			afnum ^= 1;
	}
	/* First set dir to prevent glitches when moving to AF0 */
	gpio_dir(gpio, output, value);
	writel(afnum | 0x80, /* This 0x80 for "digital mode" */
	       0x40044000 + gpio_addr[gpio]);
	/* Finally, dir again to force value when moving to gpio-out */
	gpio_dir(gpio, output, value);
	return 0;
}

/* The following functions don't check the gpio value, for speed */
u32 __gpio_get(int gpio)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);

	return readl(__GPIO_DAT(port)) & (1 << bit);
}

int gpio_get(int gpio)
{
	return __gpio_get(gpio) ? 1 : 0;
}

void __gpio_set(int gpio, u32 value)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);

	writel(value, __GPIO_BASE(port) + (0x4 << bit));
}

void gpio_set(int gpio, int value)
{
	__gpio_set(gpio, value << GPIO_BIT(gpio));
}
