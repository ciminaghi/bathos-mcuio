#include <bathos/init.h>
#include <stdint.h>
#include <arch/hw.h>
#include <mach/clocks.h>


int mach_ll_init(void)
{
	return stm32f4x_sysclock_init();
}
rom_initcall(mach_ll_init);
