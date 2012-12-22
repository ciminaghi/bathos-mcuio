/*
 * GPIO interface for LPC-1343
 * Alessandro Rubini, 2011-2012GNU GPL2 or later
 */
#ifndef __LPC13_GPIO_H__
#define __LPC13_GPIO_H__
#include <bathos/types.h>

/* We have 32 gpio bits per "port", this is hardwired */
#define GPIO_NR(port, bit)	((port) * 32 + (bit))
#define GPIO_PORT(nr)		((nr) / 32)
#define GPIO_BIT(nr)		((nr) % 32)

extern void gpio_init(void);

extern int gpio_dir_af(int gpio, int output, int value, int afnum);
extern void gpio_dir(int gpio, int output, int value);

extern int gpio_get(int gpio);
extern u32 __gpio_get(int gpio);

extern void gpio_set(int gpio, int value);
extern void __gpio_set(int gpio, u32 value);

/*
 * The following constants and stuff should only be used in gpio.c,
 * but let's export to users who might want to use the internals themselves
 */
#ifdef __LPC13_GPIO_INTERNALS__

#define __GPIO_BASE(x)		(0x50000000 + ((x) << 16))
#define __GPIO_DIR(x)		(__GPIO_BASE(x) + 0x8000)
#define __GPIO_DAT(x)		(__GPIO_BASE(x) + 0x3ffc)

/* The cfg registers for the various pins are not laid out in order */
#define __GPIO_CFG_BASE		0x40044000
#define __GPIO_CFG_P0_0		0x0c
#define __GPIO_CFG_P0_1		0x10
#define __GPIO_CFG_P0_2		0x1c
#define __GPIO_CFG_P0_3		0x2c
#define __GPIO_CFG_P0_4		0x30
#define __GPIO_CFG_P0_5		0x34
#define __GPIO_CFG_P0_6		0x4c
#define __GPIO_CFG_P0_7		0x50
#define __GPIO_CFG_P0_8		0x60
#define __GPIO_CFG_P0_9		0x64
#define __GPIO_CFG_P0_10	0x68
#define __GPIO_CFG_P0_11	0x74
#define __GPIO_CFG_P1_0		0x78
#define __GPIO_CFG_P1_1		0x7c
#define __GPIO_CFG_P1_2		0x80
#define __GPIO_CFG_P1_3		0x90
#define __GPIO_CFG_P1_4		0x94
#define __GPIO_CFG_P1_5		0xa0
#define __GPIO_CFG_P1_6		0xa4
#define __GPIO_CFG_P1_7		0xa8
#define __GPIO_CFG_P1_8		0x14
#define __GPIO_CFG_P1_9		0x38
#define __GPIO_CFG_P1_10	0x6c
#define __GPIO_CFG_P1_11	0x98
#define __GPIO_CFG_P2_0		0x08
#define __GPIO_CFG_P2_1		0x28
#define __GPIO_CFG_P2_2		0x5c
#define __GPIO_CFG_P2_3		0x8c
#define __GPIO_CFG_P2_4		0x40
#define __GPIO_CFG_P2_5		0x44
#define __GPIO_CFG_P2_6		0x00
#define __GPIO_CFG_P2_7		0x20
#define __GPIO_CFG_P2_8		0x24
#define __GPIO_CFG_P2_9		0x54
#define __GPIO_CFG_P2_10	0x58
#define __GPIO_CFG_P2_11	0x70
#define __GPIO_CFG_P3_0		0x84
#define __GPIO_CFG_P3_1		0x88
#define __GPIO_CFG_P3_2		0x9c
#define __GPIO_CFG_P3_3		0xac
#define __GPIO_CFG_P3_4		0x3c
#define __GPIO_CFG_P3_5		0x48

/* This is the bitmask of the gpio pins whose function "0" is not the gpio one */
#define __GPIO_WEIRDNESS_0	0xc01
#define __GPIO_WEIRDNESS_1	0x00f
#define __GPIO_WEIRDNESS_2	0x000
#define __GPIO_WEIRDNESS_3	0x000

/*
 * Note that there is more strangeness and asymmetry:
 * - bit 7 must be 1 for AD-able bits (we set it for everyone)
 * - when you turn input to output it forces the _current_ input
 * - some bits are open-drain (to fix the previous bugs for i2c)
 */

#endif /* __LPC13_GPIO_INTERNALS__ */
#endif /* __LPC13_GPIO_H__ */
