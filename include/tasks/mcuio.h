#ifndef __MCUIO_H__
#define __MCUIO_H__

#include <stdint.h>

#define mcuio_type_rdb	 0
#define mcuio_type_wrb	 1
#define mcuio_type_rdw	 2
#define mcuio_type_wrw	 3
#define mcuio_type_rddw	 4
#define mcuio_type_wrdw	 5
#define mcuio_type_rdq	 6
#define mcuio_type_wrq	 7

struct mcuio_base_packet {
	uint32_t offset:12;
	uint32_t func:5;
	uint32_t dev:4;
	uint32_t bus:3;
	uint32_t type:8;
	uint32_t data[2];
	uint16_t crc;
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

static inline void mcuio_packet_set_reply(struct mcuio_base_packet *p)
{
	p->type |= (1 << 6);
}

static inline int mcuio_packet_is_error(struct mcuio_base_packet *p)
{
	return p->type & (1 << 5);
}

static inline const char *mcuio_packet_type_to_str(int t)
{
	switch(t & 0x7) {
	case mcuio_type_rdb:
		return "mcuio_type_rdb";
	case mcuio_type_wrb:
		return "mcuio_type_wrb";
	case mcuio_type_rdw:
		return "mcuio_type_rdw";
	case mcuio_type_wrw:
		return "mcuio_type_wrw";
	case mcuio_type_rdq:
		return "mcuio_type_rdq";
	case mcuio_type_wrq:
		return "mcuio_type_wrq";
	}
	return "unknown";
}


struct mcuio_func_descriptor {
	uint32_t device:16;
	uint32_t vendor:16;
	uint32_t rev:8;
	uint32_t class:24;
} __attribute__((packed));

struct mcuio_func_data {
	struct mcuio_func_descriptor descr;
	uint32_t registers[0];
} __attribute__((packed));

declare_extern_event(mcuio_host_request);

#endif
