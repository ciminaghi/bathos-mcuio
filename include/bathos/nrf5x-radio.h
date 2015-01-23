/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#ifndef __NRF51_RADIO_H__
#define __NRF51_RADIO_H__

#include <stdint.h>

struct nrf5x_radio_packet {
	uint8_t len;
	char payload[0];
};

struct nrf5x_radio_platform_data {
	int irq;
	uint32_t base;
	/* Radio addresses */
	uint8_t	addr_length;
	const uint8_t * PROGMEM my_addr;
	const uint8_t * PROGMEM dst_addr;
	/* Pointer to packet area */
	struct nrf5x_radio_packet *packet_area;
	/* sizeof(packet_area->payload) */
	int packet_size;
};

extern const struct bathos_dev_ops PROGMEM nrf5x_radio_dev_ops;
extern void nrf5x_radio_irq_handler(struct bathos_dev *);

declare_extern_event(radio_empty_packet_received);

#endif /* __NRF51_RADIO_H__ */
