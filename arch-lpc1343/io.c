/*
 * Arch-dependent initialization and I/O functions
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#include <bathos/bathos.h>
#include <bathos/init.h>
#include <bathos/io.h>
#include <arch/gpio.h>

static int timer_setup(void)
{
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_CT32B1;

	/* enable timer 1, and count at HZ Hz (currently 100) */
	regs[REG_TMR32B1TCR] = 1;
	regs[REG_TMR32B1PR] = (CPU_FREQ / HZ) - 1;
	return 0;
}

core_initcall(timer_setup);

/* This function is needed to fire up the serial port */
static int uart_setup(void)
{
	unsigned int val;

	/* Turn on the clock for pin configuration, and gpio too */
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_IOCON | REG_AHBCLKCTRL_GPIO;

	/* First fix pin configuration */
	gpio_dir_af(GPIO_NR(1, 6), 0, 0, 1); /* port 1-6: rx */
	gpio_dir_af(GPIO_NR(1, 7), 1, 0, 1); /* port 1-7: tx */

	/*
	 * The clock divider must be set before turning on the clock.
	 * We are running at 12MHz with no PLL, so divide by 6.5
	 * (12M / 115200 / 16 = 6.510416)
	 */
	regs[REG_UARTCLKDIV] = 1; /* Don't divide before internal divisor */

	/* Turn on the clock for the uart */
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_UART;

	/* Disable interrupts and clear pending interrupts */
	regs[REG_U0IER] = 0;
	val = regs[REG_U0IIR];
	val = regs[REG_U0RBR];
	val = regs[REG_U0LSR];

	/* Set bit rate: enable DLAB bit and then divisor */
	regs[REG_U0LCR] = 0x80;

	/*
	 * This calculation is hairy: we need 6.5, but the fractional
	 * divisor register makes (1 + A/B) where B is 1..15
	 * 6.510416 = 4 * 1.627604 = 4 * (1+5/8)   --- better than 1%
	 */
	val = 4;
	regs[REG_U0DLL] = val & 0xff;
	regs[REG_U0DLM] = (val >> 8) & 0xff;
	regs[REG_U0FDR] = (8 << 4) | 5; /* 1 + 5/8 */

	/* clear DLAB and write mode (8bit, no parity) */
	regs[REG_U0LCR] = 0x3;
	regs[REG_U0FCR] = 0x0;
	return 0;
}

rom_initcall(uart_setup);

void putc(int c)
{
	if (c == '\n')
		putc('\r');
	while ( !(regs[REG_U0LSR] & REG_U0LSR_THRE) )
		;
	regs[REG_U0THR] = c;
}
