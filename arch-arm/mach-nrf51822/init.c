
#include <arch/hw.h>
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

static int mach_ll_init(void)
{
	while(1);
}
rom_initcall(mach_ll_init);
