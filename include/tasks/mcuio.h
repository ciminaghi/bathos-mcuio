/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#ifndef __MCUIO_H__
#define __MCUIO_H__

#include <stdint.h>

#define mcuio_type_rdb	 0 /* t & ~1 = 0 l = 1   1 << ((t & ~1)/2) = 1 */
#define mcuio_type_wrb	 1 /* t & ~1 = 1 l = 1   1 << ((t & ~1)/2) = 1 */
#define mcuio_type_rdw	 2 /* t & ~1 = 2 l = 2   1 << ((t & ~1)/2) = 2 */
#define mcuio_type_wrw	 3 /* t & ~1 = 2 l = 2   1 << ((t & ~1)/2) = 2 */
#define mcuio_type_rddw	 4 /* t & ~1 = 4 l = 4   1 << ((t & ~1)/2) = 4 */
#define mcuio_type_wrdw	 5 /* t & ~1 = 4 l = 4   1 << ((t & ~1)/2) = 4 */
#define mcuio_type_rdq	 6 /* t & ~1 = 6 l = 8   1 << ((t & ~1)/2) = 8 */
#define mcuio_type_wrq	 7 /* t & ~1 = 6 l = 8   1 << ((t & ~1)/2) = 8 */

struct mcuio_base_packet {
	uint32_t offset:12;
	uint32_t func:5;
	uint32_t dev:4;
	uint32_t bus:3;
	uint32_t type:8;
	uint32_t data[2];
	uint16_t crc;
	uint16_t dummy;
} __attribute__((packed));

static inline int mcuio_packet_is_read(struct mcuio_base_packet *p)
{
	return !(p->type & 0x1);
}

static inline int mcuio_packet_is_write(struct mcuio_base_packet *p)
{
	return (p->type & 0x1);
}

static inline int mcuio_packet_is_reply(struct mcuio_base_packet *p)
{
	return p->type & (1 << 6);
}

static inline int mcuio_packet_is_fill_data(struct mcuio_base_packet *p)
{
	return (p->type & (1 << 7));
}

static inline void mcuio_packet_set_reply(struct mcuio_base_packet *p)
{
	p->type |= (1 << 6);
}

static inline int mcuio_packet_is_error(struct mcuio_base_packet *p)
{
	return p->type & (1 << 5);
}

static inline int mcuio_packet_set_error(struct mcuio_base_packet *p)
{
	return p->type |= (1 << 5);
}

static inline int mcuio_packet_data_len(struct mcuio_base_packet *p)
{
	return 1 << ((p->type & ((1 << 5) - 1)) >> 1);
}

static inline const char *mcuio_packet_type_to_str(int t)
{
	switch(t & 0x7) {
	case mcuio_type_rdb:
		return "rdb";
	case mcuio_type_wrb:
		return "wrb";
	case mcuio_type_rdw:
		return "rdw";
	case mcuio_type_wrw:
		return "wrw";
	case mcuio_type_rddw:
		return "rddw";
	case mcuio_type_wrdw:
		return "wrdw";
	case mcuio_type_rdq:
		return "rdq";
	case mcuio_type_wrq:
		return "wrq";
	}
	return "unknown";
}


struct mcuio_func_descriptor {
	uint32_t device:16;
	uint32_t vendor:16;
	uint32_t class:24;
	uint32_t rev:8;
} __attribute__((packed));

struct mcuio_func_data {
	struct mcuio_func_descriptor descr;
	uint32_t registers[0];
} __attribute__((packed));

#endif
