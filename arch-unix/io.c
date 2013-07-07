/*
 * Arch-dependent initialization and I/O functions
 * Alessandro Rubini, 2012 GNU GPL2 or later
 */
#include <stdint.h>
#include <bathos/bathos.h>
#include <arch/hw.h>

/* We need a main function, called by libc initialization */
int main(int argc, char **argv)
{
	bathos_main();
	return 0;
}

/* And this trivially uses stdout */
void putc(int c)
{
	uint8_t ch = c;

	write(1, &ch, 1);
}
