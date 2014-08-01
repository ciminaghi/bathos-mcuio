#ifndef __MCUIO_H__
#define __MCUIO_H__

#define mcuio_htonl(a) a
#define mcuio_ntohl(a) a
#define mcuio_ntohs(a) a
#define mcuio_htons(a) a

#include <stdint.h>
#include <bathos/mcuio-proto.h>

struct mcuio_func_data {
	struct mcuio_func_descriptor descr;
	uint32_t registers[0];
} __attribute__((packed));

#define INIT_MCUIO_FUNC_DESCR(d,v,r,c) {				\
	.device_vendor = ((uint32_t)(d)) | (((uint32_t)(v)) << 16),	\
	.rev_class = ((uint32_t)(r)) | (((uint32_t)(c)) << 8),		\
}

#endif
