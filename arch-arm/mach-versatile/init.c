
#include <bathos/init.h>
#include <stdint.h>
#include <arch/hw.h>


int mach_ll_init(void)
{
	return 0;
}
rom_initcall(mach_ll_init);
