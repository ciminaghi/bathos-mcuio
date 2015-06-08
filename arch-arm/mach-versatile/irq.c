
#include <stdint.h>
#include <bathos/bathos.h>
#include <bathos/io.h>
#include <bathos/types.h>
#include <bathos/errno.h>
#include <bathos/irq-controller.h>


/* Primary irq controller is a pl190@0x10140000 */
/* See http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0224i/ch04s11s03.html */
#define PRIMARY_VIC_BASE		0x10140000

#define PRIMARY_VIC_IRQSTATUS		(PRIMARY_VIC_BASE + 0x0)
#define PRIMARY_VIC_INTENSET		(PRIMARY_VIC_BASE + 0x10)
#define PRIMARY_VIC_INTENCLEAR		(PRIMARY_VIC_BASE + 0x14)

#define SECONDARY_IRQSTATUS		0x10003000
#define SECONDARY_INTENSET		0x10003008
#define SECONDARY_INTENCLEAR		0x1000300c

/*
 * Irq handler, read primary controller. Just read primary and secondary
 * interrupt status and trigger relevant events
 */
void versatile_irq_handler(void)
{
	uint32_t s[2];

	/* Discard bit 31 if set (cascade IRQ from secondary controller */
	s[0] = readl(PRIMARY_VIC_IRQSTATUS) & ~(1 << 31);
	s[1] = readl(SECONDARY_IRQSTATUS);

	(void)trigger_multiple_interrupt_events(0, s, BITS_PER_LONG*2);
}


/* Driver for primary irq controller */

static void pl190_enable(struct bathos_irq_controller *ctrl, int irqn)
{
}

static void pl190_disable(struct bathos_irq_controller *ctrl, int irqn)
{
}

static void pl190_clear_pending(struct bathos_irq_controller *ctrl, int irqn)
{
}

static void pl190_set_pending(struct bathos_irq_controller *ctrl, int irqn)
{
}

static void pl190_mask_ack(struct bathos_irq_controller *ctrl, int irqn)
{
}

static struct bathos_irq_controller primary_irq_ctrl = {
	.enable_irq = pl190_enable,
	.disable_irq = pl190_disable,
	.clear_pending = pl190_clear_pending,
	.set_pending = pl190_set_pending,
	.mask_ack = pl190_mask_ack,
	.unmask = pl190_enable,
};

/* Driver for secondary irq controller */
static void versatile_sec_enable(struct bathos_irq_controller *ctrl, int irqn)
{
}

static void versatile_sec_disable(struct bathos_irq_controller *ctrl, int irqn)
{
}

static void versatile_sec_clear_pending(struct bathos_irq_controller *ctrl,
					int irqn)
{
}

static void versatile_sec_set_pending(struct bathos_irq_controller *ctrl,
				      int irqn)
{
}

static void versatile_sec_mask_ack(struct bathos_irq_controller *ctrl,
				   int irqn)
{
}

static struct bathos_irq_controller secondary_irq_ctrl = {
	.enable_irq = versatile_sec_enable,
	.disable_irq = versatile_sec_disable,
	.clear_pending = versatile_sec_clear_pending,
	.set_pending = versatile_sec_set_pending,
	.mask_ack = versatile_sec_mask_ack,
	.unmask = versatile_sec_enable,
};

/* Always return the same irq controller */
struct bathos_irq_controller *bathos_irq_to_ctrl(int irq)
{
	if (irq < 0 || irq > 63)
		return -EINVAL;
	if (irq < 32)
		return &primary_irq_ctrl;
	return &secondary_irq_ctrl;
}

