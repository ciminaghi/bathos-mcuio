#include <bathos/init.h>
#include <stdint.h>
#include <arch/hw.h>
#include <mach/hw.h>

int timer_setup(void)
{
	/* First configure as 32-bit, but keep it disabled */
	regs[REG_TIMER_CTRL] = TIMER_CTRL_32BIT | TIMER_CTRL_DIV16 \
		| TIMER_CTRL_PERIODIC;
	/* counts down to zero, so start at the maximum */
	regs[REG_TIMER_LOAD] = ~0;
	regs[REG_TIMER_BGLOAD] = ~0;
	/* Finally enable it */
	regs[REG_TIMER_CTRL] = TIMER_CTRL_32BIT | TIMER_CTRL_DIV16 \
		| TIMER_CTRL_PERIODIC | TIMER_CTRL_ENABLE;
	return 0;
}
core_initcall(timer_setup);
