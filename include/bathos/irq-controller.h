/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
#ifndef __IRQ_CONTROLLER_H__
#define __IRQ_CONTROLLER_H__

struct bathos_irq_controller {
	void (*enable_irq)(struct bathos_irq_controller *, int irq);
	void (*disable_irq)(struct bathos_irq_controller *, int irq);
	void (*clear_pending)(struct bathos_irq_controller *, int irq);
	void (*set_pending)(struct bathos_irq_controller *, int irq);
	void *priv;
};

#ifndef CONFIG_HAVE_IRQ_CONTROLLER
static inline struct bathos_irq_controller *bathos_irq_to_ctrl(int irq)
{
	return NULL;
}
#endif /* CONFIG_HAVE_IRQ_CONTROLLER */

#endif /* __IRQ_CONTROLLER_H__ */
