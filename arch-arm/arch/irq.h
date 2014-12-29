/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __ARCH_ARM_IRQ_H__
#define __ARCH_ARM_IRQ_H__

#include <generated/autoconf.h>
#include <bathos/irq.h>

extern int arch_set_irq_vector(int irq, bathos_irq_handler handler);

#endif /* __ARCH_ARM_IRQ_H__ */
