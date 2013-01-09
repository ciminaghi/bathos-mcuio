/*
 * Arch-dependent initialization and I/O functions
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <arch/hw.h>

int bathos_setup(void)
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

void putc(int c)
{
	if (c == '\n')
		putc('\r');
	while (regs[REG_UART01x_FR] & 0x8 /* busy */ )
		;
	regs[REG_UART01x_DR] = c;
}
