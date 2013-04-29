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
	/* enable timer 0, and count at HZ Hz */
	regs[REG_T0TCR] = 1;
	regs[REG_T0PR] = (CPU_FREQ / HZ) - 1;
	regs[REG_T0TCR] = 3;
	regs[REG_T0TCR] = 1;
	return 0;
}

core_initcall(timer_setup);

/* This function is needed to fire up the serial port */
static int uart_setup(void)
{
	unsigned int val;

	/* First fix pin configuration */
	gpio_dir_af(GPIO_NR(0, 0), 1, 0, 1); /* port 0-0: tx */
	gpio_dir_af(GPIO_NR(0, 1), 0, 0, 1); /* port 0-1: rx */

	/* Disable interrupts and clear pending interrupts */
	regs[REG_U0IER] = 0;
	val = regs[REG_U0IIR];
	val = regs[REG_U0RBR];
	val = regs[REG_U0LSR];

	/* Set bit rate: enable DLAB bit and then divisor */
	regs[REG_U0LCR] = 0x80;
	val = CPU_FREQ / (115200 * 16);
	regs[REG_U0DLL] = val & 0xff;
	regs[REG_U0DLM] = (val >> 8) & 0xff;

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
