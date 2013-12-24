#ifndef __MCUIO_FUNCTION_H__
#define __MCUIO_FUNCTION_H__

#include <stdint.h>

struct mcuio_range;

struct mcuio_range_ops {
	int (*readb)(struct mcuio_range *r, unsigned offset, uint8_t *dst);
	int (*readw)(struct mcuio_range *r, unsigned offset, uint16_t *dst);
	int (*readdw)(struct mcuio_range *r, unsigned offset, uint32_t *dst);
	int (*readq)(struct mcuio_range *r, unsigned offset, uint64_t *dst);
	int (*writeb)(struct mcuio_range *r, unsigned offset, uint8_t *dst);
	int (*writew)(struct mcuio_range *r, unsigned offset, uint16_t *dst);
	int (*writedw)(struct mcuio_range *r, unsigned offset, uint32_t *dst);
	int (*writeq)(struct mcuio_range *r, unsigned offset, uint64_t *dst);
};

struct mcuio_range {
	unsigned int start;
	unsigned int length;
	void *target;
	struct mcuio_range_ops *ops;
};

struct mcuio_function_runtime {
	int last_error;
	void *priv;
};

struct mcuio_function_ops {
	/* reply/error notification */
	void (*reply)(struct mcuio_function *f, int err);
};

struct mcuio_function {
	int nranges;
	const struct mcuio_range *ranges;
	const struct mcuio_base_packet *to_host;
	/* This is written on host replies */
	struct mcuio_base_packet *from_host;
	const struct mcuio_function_ops *ops;
	struct mcuio_function_runtime *runtime;
};

#if 0
#define declare_mcuio_function(name, descr)			\
	struct mcuio_function name				\
	__attribute__((section(".mcuio_functions"))) = {	\
		.nranges = nr					\
	};
#endif

extern struct mcuio_function mcuio_functions_start[], mcuio_functions_end[];

declare_extern_event(mcuio_function_request);
declare_extern_event(mcuio_function_reply);


#endif /* __MCUIO_FUNCTION_H__ */
