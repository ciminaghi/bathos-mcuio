
#include <arch/hw.h>
#include <bathos/gpio.h>
#include <bathos/init.h>
#include <bathos/event.h>
#include <cpu-cortex-m0/nvic.h>

unsigned long jiffies;
volatile uint32_t *vectors = rom_vectors;

#ifdef CONFIG_CONSOLE_DEBUG_BITBANG
static void console_gpio_init(void)
{
	gpio_dir_af(CONFIG_BB_CLOCK_GPIO, 1, 1, 1);
	gpio_dir_af(CONFIG_BB_DATA_GPIO, 1, 1, 1);
}
#else
static void console_gpio_init(void)
{
}
#endif

static int mach_ll_init(void)
{
	clocks_init();
	nvic_init_ram();
	jiffies_init();
	console_gpio_init();
	events_init();
	return 0;
}
rom_initcall(mach_ll_init);
