#ifndef __MCUIO_FUNCTION_H__
#define __MCUIO_FUNCTION_H__

#include <stdint.h>

struct mcuio_range;
struct mcuio_function;

typedef int (*mcuio_read)(const struct mcuio_range *,
			  unsigned offset, uint32_t *out);
typedef int (*mcuio_write)(const struct mcuio_range *, unsigned offset,
			   const uint32_t *in);

struct mcuio_range_ops {
	/* Pointers to read operations */
	mcuio_read rd[4];
	/* Pointers to write operations */
	mcuio_write wr[4];
};

struct mcuio_range {
	unsigned int start;
	unsigned int length;
	void *target;
	const struct mcuio_range_ops *ops;
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

#define declare_mcuio_function(name, r, th, o, rt)		\
	struct mcuio_function name				\
	__attribute__((section(".mcuio_functions"))) = {	\
		.nranges = ARRAY_SIZE(r),			\
		.ranges = r,					\
		.to_host = th,					\
		.ops = o,					\
		.runtime = r,					\
	};

extern struct mcuio_function mcuio_functions_start[], mcuio_functions_end[];

declare_extern_event(mcuio_function_request);
declare_extern_event(mcuio_function_reply);


extern int mcuio_rdb(const struct mcuio_range *, unsigned offset,
		     uint32_t *out);
extern int mcuio_rdw(const struct mcuio_range *, unsigned offset,
		     uint32_t *out);
extern int mcuio_rddw(const struct mcuio_range *, unsigned offset,
		      uint32_t *out);
extern int mcuio_rdq(const struct mcuio_range *, unsigned offset,
		     uint32_t *out);
extern int mcuio_wrb(const struct mcuio_range *, unsigned offset,
		     const uint32_t *in);
extern int mcuio_wrw(const struct mcuio_range *, unsigned offset,
		     const uint32_t *in);
extern int mcuio_wrdw(const struct mcuio_range *, unsigned offset,
		      const uint32_t *in);
extern int mcuio_wrq(const struct mcuio_range *, unsigned offset,
		     const uint32_t *in);

extern const struct mcuio_range_ops default_mcuio_range_ops;
extern const struct mcuio_range_ops default_mcuio_range_ro_ops;



#endif /* __MCUIO_FUNCTION_H__ */
