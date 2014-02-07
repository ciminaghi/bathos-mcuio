
#include <arch/hw.h>
#include <avr/io.h>

void pll_init(void)
{
	/* config PLL, 16 MHz xtal */
	PLLCSR = 0x12;
	/* wait for PLL lock */
	while (!(PLLCSR & (1<<PLOCK))) ;
}

void timer_init(void)
{
}

void usb_enable_freeze(void)
{
	USBCON = (1<<USBE)|(1<<FRZCLK);
}

void usb_config(void)
{
	USBCON = (1<<USBE)|(1<<OTGPADE);
}

void usb_hw_config(void)
{
	UHWCON = 0x81;
}
