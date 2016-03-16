/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>, original code by
 * Aurelio Colosimo <aurelio@aureliocolosimo.it>
 */

#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/types.h>
#include <bathos/irq-controller.h>
#include <arch/nvic.h>

static void nvic_enable(struct bathos_irq_controller *ctrl, int irqn)
{
	int offs = irqn >> 5;
	int bit = irqn & (32 - 1);

	regs[REG_NVIC_ISER + offs] = 1 << bit;
}

static void nvic_disable(struct bathos_irq_controller *ctrl, int irqn)
{
	int offs = irqn >> 5;
	int bit = irqn & (32 - 1);

	regs[REG_NVIC_ICER + offs] = 1 << bit;
}

static void nvic_clear_pending(struct bathos_irq_controller *ctrl, int irqn)
{
	int offs = irqn >> 5;
	int bit = irqn & (32 - 1);

	regs[REG_NVIC_ICPR + offs] = 1 << bit;
}

static void nvic_set_pending(struct bathos_irq_controller *ctrl, int irqn)
{
	int offs = irqn >> 5;
	int bit = irqn & (32 - 1);

	regs[REG_NVIC_ISPR + offs] = 1 << bit;
}

static void nvic_mask_ack(struct bathos_irq_controller *ctrl, int irqn)
{
	int offs = irqn >> 5;
	int bit = irqn & (32 - 1);
	uint32_t mask = 1 << bit;

	regs[REG_NVIC_ICER + offs] = mask;
	regs[REG_NVIC_ICPR + offs] = mask;
}

static struct bathos_irq_controller nvic = {
	.enable_irq = nvic_enable,
	.disable_irq = nvic_disable,
	.clear_pending = nvic_clear_pending,
	.set_pending = nvic_set_pending,
	.mask_ack = nvic_mask_ack,
	.unmask = nvic_enable,
};

/* Always return the same irq controller */
struct bathos_irq_controller *bathos_irq_to_ctrl(int irq)
{
	return &nvic;
}
