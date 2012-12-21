#ifndef __LPC21_GPIO_H__
#define __LPC21_GPIO_H__
#include <bathos/types.h>
#include <arch/hw.h>

/* We have 32 gpio bits per "port", this is hardwired */
#define GPIO_NR(port, bit)	((port) * 32 + (bit))
#define GPIO_PORT(nr)		((nr) / 32)
#define GPIO_BIT(nr)		((nr) % 32)

#define GPIO_MAX		31

extern void gpio_init(void);

extern int gpio_dir_af(int gpio, int output, int value, int afnum);
extern void gpio_dir(int gpio, int output, int value);

/* Warning: the following functions assume we have a single "port" */
static inline int gpio_get(int gpio)
{
	return (regs[REG_IOPIN] >> gpio) & 1;
}

static inline u32 __gpio_get(int gpio)
{
	return regs[REG_IOPIN] & (1 << gpio);
}

static inline void gpio_set(int gpio, int value)
{
	if (value)
		regs[REG_IOSET] = 1 << gpio;
	else
		regs[REG_IOCLR] = 1 << gpio;
}

static inline void __gpio_set(int gpio, u32 value)
{
	if (value)
		regs[REG_IOSET] = value;
	else
		regs[REG_IOCLR] = 1 << gpio;
}

#endif /* __LPC13_GPIO_H__ */
