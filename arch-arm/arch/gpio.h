/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __GPIO_ARM_H__
#define __GPIO_ARM_H__

#include <bathos/types.h>
#include <bathos/errno.h>
#include <generated/autoconf.h>

#define GPIO_PORT_WIDTH 32
#define GPIO_NR(port, bit) ((port) * GPIO_PORT_WIDTH + (bit))
#define GPIO_PORT(nr)      ((nr) / GPIO_PORT_WIDTH)
#define GPIO_BIT(nr)       ((nr) % GPIO_PORT_WIDTH)


#include <mach/gpio.h>


#endif /* __GPIO_ARM_H__ */
