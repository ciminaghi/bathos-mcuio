/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __BATHOS_IRQ_H__
#define __BATHOS_IRQ_H__

#include <generated/autoconf.h>
#include <bathos/errno.h>

typedef void (*bathos_irq_handler)(void);

extern int bathos_request_irq(int irq, bathos_irq_handler handler);

#ifndef CONFIG_RELOCATE_VECTORS_TABLE
/*
 * Cannot set interrupt vector if vector table cannot be relocated to ram
 */
static inline int arch_set_irq_vector(int irq, bathos_irq_handler handler)
{
	return -ENOSYS;
}
#else
/* arch_set_irq_vector() */
#include <arch/irq.h>
#endif

static inline int bathos_free_irq(int irq)
{
	return bathos_request_irq(irq, NULL);
}

int bathos_enable_irq(int irq);
int bathos_disable_irq(int irq);
int bathos_set_pending_irq(int irq);
int bathos_clear_pending_irq(int irq);
int bathos_mask_ack_irq(int irq);
int bathos_unmask_irq(int irq);

#endif /* __BATHOS_IRQ_H__ */
