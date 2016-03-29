/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */
/* Instantiate stm32f4x devices for the Arduino STAR board */

#include <bathos/init.h>
#include <bathos/event.h>
#include <bathos/stm32f4x-uart.h>
#include <bathos/dev_ops.h>
#include <bathos/pipe.h>
#include <bathos/gpio.h>
#include <mach/hw.h>

#define HSE_FREQ (8000000UL)
#define VCO_FREQ ((HSE_FREQ/CONFIG_STM32F4X_PLL_M) * CONFIG_STM32F4X_PLL_N)
#define PLL_FREQ (VCO_FREQ / CONFIG_STM32F4X_PLL_P)
#define AHB_FREQ (PLL_FREQ / CONFIG_STM32F4X_AHB_PRESC)
#define APB1_FREQ (PLL_FREQ / CONFIG_STM32F4X_APB1_PRESC)
#define APB2_FREQ (PLL_FREQ / CONFIG_STM32F4X_APB2_PRESC)

char pippo[200];

struct stm32f4x_uart_platform_data uart5_pdata = {
	/* PC12, GPIO44 */
	.tx_pin = 44,
	.tx_af = AF08,
	/* PD2, GPIO50 */
	.rx_pin = 50,
	.rx_af = AF08,
	.base = UART5_BASE,
	.clock = APB1_FREQ,
	.bus = UART5_BUS,
	.irq = IRQ_UART5,
	.periph_id = UART5_ID,
};

static struct bathos_dev __udev0
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart5",
	.ops = &stm32f4x_uart_dev_ops,
	.platform_data = &uart5_pdata,
};

void bathos_ll_int_handler_name(IRQ_UART5)(struct event_handler_data *data)
{
	stm32f4x_uart_irq_handler(&__udev0);
}

#ifdef CONFIG_CONSOLE_UART5
# error "UART5 CANNOT BE USED AS A SERIAL CONSOLE ON THE ARDUINO STAR BOARD"
#endif

struct stm32f4x_uart_platform_data uart6_pdata = {
	/* PC6, GPIO38 */
	.tx_pin = 38,
	.tx_af = AF08,
	/* PC7, GPIO39 */
	.rx_pin = 39,
	.rx_af = AF08,
	.base = USART6_BASE,
	.clock = APB2_FREQ,
	.bus = USART6_BUS,
	.periph_id = USART6_ID,
	.irq = IRQ_USART6,
};

static struct bathos_dev __udev1
	__attribute__((section(".bathos_devices"), aligned(4))) = {
	.name = "uart6",
	.ops = &stm32f4x_uart_dev_ops,
	.platform_data = &uart6_pdata,
};

void bathos_ll_int_handler_name(IRQ_USART6)(struct event_handler_data *data)
{
	stm32f4x_uart_irq_handler(&__udev1);
}

#ifdef CONFIG_CONSOLE_UART6
int console_init(void)
{
	return stm32f4x_uart_console_init(&uart6_pdata);
}

void console_putc(int c)
{
	stm32f4x_uart_console_putc(&uart6_pdata, c);
}
#endif
