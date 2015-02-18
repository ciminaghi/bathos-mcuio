
#include <stdint.h>
#include <arch/hw.h>


const uint32_t __attribute__((section(".flash_config")))
flash_config[] = {
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffff7e,
};

void mach_ll_init(void)
{
	clocks_init();
}
