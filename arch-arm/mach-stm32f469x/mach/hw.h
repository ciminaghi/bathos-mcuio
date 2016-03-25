/*
 * Header file for STM32F469/479x
 *
 * Copyright Dog Hunter SA 2016
 * GPLv2 or later
 * Author: Davide Ciminaghi (comes from kl25z hw.h)
 */
#ifndef __STM32F469X_H__
#define __STM32F469X_H__

#include <family/clocks.h>

#define GPIO_BASE 0x40020000
#define RCC_BASE 0x40023800

#define IRQ_UART5       53
#define IRQ_USART6      71

#define USART1_ID	4
#define USART1_BUS	APB2
#define USART1_BASE	0x40011000
#define UART2_ID	17
#define USART2_BUS	APB1
#define USART2_BASE	0x40004400
#define USART3_ID	18
#define USART3_BUS	APB1
#define USART3_BASE	0x40004800
#define UART4_ID	19
#define UART4_BUS	APB1
#define UART4_BASE	0x40004C00
#define UART5_ID	20
#define UART5_BUS	APB1
#define UART5_BASE	0x40005000
#define USART6_ID	5
#define USART6_BUS	APB2
#define USART6_BASE	0x40011400
#define UART7_ID	30
#define UART7_BUS	APB1
#define UART7_BASE	0x40007800
#define UART8_ID	31
#define UART8_BUS	APB1
#define UART8_BASE	0x40007C00

#endif /* __STM32F469X_H__ */
