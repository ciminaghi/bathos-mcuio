/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/event.h>

#ifndef INTNO
#error "Please compile this file with -DINTNO=interrupt_number"
#endif

#define ISR_NAME bathos_int_handler_name(INTNO)
#define LL_ISR_NAME bathos_ll_int_handler_name(INTNO)
#define ISR_PRIV bathos_int_handler_priv(INTNO)
#define EVT_NAME xcat(int_evt_,INTNO)

/* Dummy interrupt event handlers and relevant data, can be overridden */
void  __attribute__((weak)) ISR_NAME(struct event_handler_data *d)
{
}

void __attribute__((weak)) LL_ISR_NAME(struct event_handler_data *d)
{
}

struct { } ISR_PRIV __attribute__((weak));

static const struct event_handler_ops PROGMEM
event_handler_ops_struct(ISR_NAME) = {
	.init = NULL,
	.handle_ll = LL_ISR_NAME,
	.handle = ISR_NAME,
	.exit = NULL,
};

static struct event_handler_data PROGMEM event_handler_struct(ISR_NAME) = {
	.ops = &event_handler_ops_struct(ISR_NAME),
	.priv = &ISR_PRIV,
};

/*
 * Interrupts can have just one handler
 */
const struct event __attribute__((section(".interrupt_events"))) EVT_NAME = {
	.handlers_start = &event_handler_struct(ISR_NAME),
	.handlers_end = &(&event_handler_struct(ISR_NAME))[1],
};


