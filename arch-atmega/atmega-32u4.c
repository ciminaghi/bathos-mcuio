/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/bathos.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>
#include <bathos/idle.h>
#include <bathos/sys_timer.h>

#include <arch/hw.h>
#include <avr/interrupt.h>
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
	/* prescaler: 256, 16MHz / 256 = 62500Hz */
	TCCR0B = 1 << CS02;
	/* 62500Hz / 250 = 250HZ */
	OCR0A = 250;
	TIMSK0 = 1 << TOIE0;
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

volatile unsigned long jiffies;

ISR(TIMER0_OVF_vect, __attribute__((section(".text.ISR"))))
{
	jiffies++;
	trigger_event(&event_name(hw_timer_tick), NULL, EVT_PRIO_MAX);
}
