
#include <bathos/init.h>
#include <stdint.h>
#include <arch/hw.h>
#include <mach/hw.h>


const uint32_t __attribute__((section(".flash_config")))
flash_config[] = {
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffff7e,
};

int mach_ll_init(void)
{
	clocks_init();
	return 0;
}
rom_initcall(mach_ll_init);
