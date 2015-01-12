
#include <arch/hw.h>
#include <mach/hw.h>
#include <bathos/gpio.h>
#include <bathos/init.h>
#include <bathos/event.h>

unsigned long jiffies;

#ifdef CONFIG_CONSOLE_DEBUG_BITBANG
static void console_gpio_init(void)
{
	gpio_dir_af(CONFIG_BB_CLOCK_GPIO, 1, 1, 0);
	gpio_dir_af(CONFIG_BB_DATA_GPIO, 1, 1, 0);
}
#else
static void console_gpio_init(void)
{
}
#endif

static int is_manual_peripheral_setup_needed(void)
{
	if ((((*(uint32_t *)0xF0000FE0) & 0x000000FF) != 0x1) ||
	    (((*(uint32_t *)0xF0000FE4) & 0x0000000F) != 0x0))
		return 0;
	if ((((*(uint32_t *)0xF0000FE8) & 0x000000F0) == 0x00) &&
	    (((*(uint32_t *)0xF0000FEC) & 0x000000F0) == 0x0)) {
		return 1;
	}
	if ((((*(uint32_t *)0xF0000FE8) & 0x000000F0) == 0x10) &&
	    (((*(uint32_t *)0xF0000FEC) & 0x000000F0) == 0x0)) {
		return 1;
	}
	if ((((*(uint32_t *)0xF0000FE8) & 0x000000F0) == 0x30) &&
	    (((*(uint32_t *)0xF0000FEC) & 0x000000F0) == 0x0)) {
		return 1;
	}
	return 0;
}

static int is_disabled_in_debug_needed(void)
{
	if ((((*(uint32_t *)0xF0000FE0) & 0x000000FF) != 0x1) ||
	    (((*(uint32_t *)0xF0000FE4) & 0x0000000F) != 0x0))
		return 0;
	if ((((*(uint32_t *)0xF0000FE8) & 0x000000F0) == 0x40) &&
	    (((*(uint32_t *)0xF0000FEC) & 0x000000F0) == 0x0))
		return 1;
	return 0;
}


static void do_machine_quirks(void)
{
	if (is_manual_peripheral_setup_needed()) {
		*(uint32_t volatile *)0x40000504 = 0xC007FFDF;
		*(uint32_t volatile *)0x40006C18 = 0x00008000;
	}
	if (is_disabled_in_debug_needed())
		regs[REG_MPU_DISDBG] = 1;
}


static int mach_ll_init(void)
{
	do_machine_quirks();
	console_gpio_init();
	console_putc('.');
	return 0;
}
rom_initcall(mach_ll_init);
