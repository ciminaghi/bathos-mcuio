
#include <avr/io.h>

void timer_init(void)
{
#if 0
	/* FIX THIS !!! */
	/* set up the timer: use 256 as a prescaler, and irq */
	TCCR0 = REG_TCCR0_P256;
	TIMSK = REG_TIMSK_TOIE0;
#endif
}

