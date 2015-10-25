#ifndef __NRF5X_GPIO_H__
#define __NRF5X_GPIO_H__

#include <bathos/io.h>
#include <mach/hw.h>

/* Common gpios definitions for nrf5x family */

#define REG_GPIO_OUT    ((GPIO_BASE + 0x504) / 4)
#define REG_GPIO_OUTSET ((GPIO_BASE + 0x508) / 4)
#define REG_GPIO_OUTCLR ((GPIO_BASE + 0x50c) / 4)
#define REG_GPIO_IN     ((GPIO_BASE + 0x510) / 4)
#define REG_GPIO_DIR    ((GPIO_BASE + 0x514) / 4)
#define REG_GPIO_DIRSET ((GPIO_BASE + 0x518) / 4)
#define REG_GPIO_DIRCLR ((GPIO_BASE + 0x51c) / 4)
#define REG_PIN_CNF(i)  ((GPIO_BASE + 0x700 + (i) * 4) / 4)

#define PIN_CNF_DIR_IN (0 << 0)
#define PIN_CNF_DIR_OUT (1 << 0)
#define PIN_CNF_IN_CON (0 << 1)
#define PIN_CNF_IN_DIS (1 << 1)
#define PIN_CNF_NOPULL (0 << 2)
#define PIN_CNF_PULLDN (1 << 2)
#define PIN_CNF_PULLUP (2 << 2)
#define PIN_CNF_S0S1   (0 << 8)
#define PIN_CNF_H0S1   (1 << 8)
#define PIN_CNF_S0H1   (2 << 8)
#define PIN_CNF_H0H1   (3 << 8)
#define PIN_CNF_D0S1   (4 << 8)
#define PIN_CNF_D0H1   (5 << 8)
#define PIN_CNF_S0D1   (6 << 8)
#define PIN_CNF_H0D1   (7 << 8)
#define PIN_CNF_SNS_DIS (0 << 16)
#define PIN_CNF_SNS_HI (1 << 16)
#define PIN_CNF_SNS_LO (2 << 16)

static inline void gpio_set(int gpio, int value)
{
	int pin = GPIO_BIT(gpio);

	if (value)
		regs[REG_GPIO_OUTSET] = (1 << pin);
	else
		regs[REG_GPIO_OUTCLR] = (1 << pin);
}

static inline void gpio_dir(int gpio, int output, int value)
{
	int pin = GPIO_BIT(gpio);

	gpio_set(gpio, value);

	if (output)
		regs[REG_GPIO_DIRSET] = (1 << pin);
	else {
		uint32_t v = PIN_CNF_DIR_IN |
			PIN_CNF_IN_CON |
			PIN_CNF_S0S1 |
			PIN_CNF_SNS_DIS;
		if (value)
			v |= PIN_CNF_PULLDN;
		regs[REG_PIN_CNF(pin)] = v;
	}
}

static inline int gpio_get(int gpio)
{
	int pin = GPIO_BIT(gpio);

	return regs[REG_GPIO_IN] & (1 << pin);
}

static inline int gpio_dir_af(int gpio, int output, int value, int afnum)
{
	if (afnum)
		return -EINVAL;
	gpio_dir(gpio, output, value);
	return 0;
}


static inline int gpio_get_dir_af(int gpio, int *output, int *value, int *afnum)
{
	int pin = GPIO_BIT(gpio);

	if (output)
		*output = regs[REG_GPIO_DIR] & (1 << pin);
	if (afnum)
		*afnum = 0;
	if (value)
		*value = gpio_get(gpio);
	return 0;
}

extern void gpio_init(void);


#endif /* __NRF5X_GPIO_H__ */
