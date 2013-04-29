/*
 * Arch-dependent initialization and I/O functions
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/init.h>
#include <arch/hw.h>

static int timer_setup(void)
{
	/* enable timer 0, and count at HZ Hz */
	regs[REG_T0TCR] = 1;
	regs[REG_T0PR] = (CPU_FREQ / HZ) - 1;
	regs[REG_T0TCR] = 3;
	regs[REG_T0TCR] = 1;
	return 0;
}

core_initcall(timer_setup);

void putc(int c)
{
	if (c == '\n')
		putc('\r');
	while ( !(regs[REG_U0LSR] & REG_U0LSR_THRE) )
		;
	regs[REG_U0THR] = c;
}
