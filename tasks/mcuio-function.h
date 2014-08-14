#ifndef __MCUIO_FUNCTION_H__
#define __MCUIO_FUNCTION_H__

#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

struct mcuio_range;
struct mcuio_function;

typedef int (*mcuio_read)(const struct mcuio_range *,
			  unsigned offset, uint32_t *out, int fill_data);
typedef int (*mcuio_write)(const struct mcuio_range *, unsigned offset,
			   const uint32_t *in, int fill_data);

struct mcuio_range_ops {
	/* Pointers to read operations */
	mcuio_read rd[4];
	/* Pointers to write operations */
	mcuio_write wr[4];
};

struct mcuio_range {
	unsigned int start;
	const unsigned int * PROGMEM length;
	const void * PROGMEM rd_target;
	void *wr_target;
	const struct mcuio_range_ops * PROGMEM ops;
};

struct mcuio_function_runtime {
	struct mcuio_base_packet to_host;
};

extern struct mcuio_function_runtime mcuio_func_common_runtime;

struct mcuio_function_ops {
	/* reply/error notification */
	void (*reply)(struct mcuio_function *f, int err);
};

struct mcuio_function {
	int nranges;
	const struct mcuio_range * PROGMEM ranges;
	const struct mcuio_function_ops * PROGMEM ops;
	struct mcuio_function_runtime *runtime;
};

struct mcuio_function_irq_data {
	unsigned int func;
	unsigned int active;
};

#define __mcuio_func __attribute__((section(".mcuio_functions")))

#define declare_mcuio_function(name, r, th, o, rt)		\
	struct mcuio_function name				\
	__mcuio_func  = {					\
		.nranges = ARRAY_SIZE(r),			\
		.ranges = r,					\
		.ops = o,					\
		.runtime = rt,					\
	};

extern struct mcuio_function mcuio_functions_start[], mcuio_functions_end[];

declare_extern_event(mcuio_function_request);
declare_extern_event(mcuio_function_reply);
declare_extern_event(mcuio_function_irq);


extern int mcuio_rdb(const struct mcuio_range *, unsigned offset,
		     uint32_t *out, int fill);
#ifdef ARCH_IS_HARVARD
extern int mcuio_rdb_ram(const struct mcuio_range *, unsigned offset,
			 uint32_t *out, int fill);
#else
#define mcuio_rdb_ram mcuio_rdb
#endif

extern int mcuio_rdw(const struct mcuio_range *, unsigned offset,
		     uint32_t *out, int fill);
#ifdef ARCH_IS_HARVARD
extern int mcuio_rdw_ram(const struct mcuio_range *, unsigned offset,
			 uint32_t *out, int fill);
#else
#define mcuio_rdw_ram mcuio_rdw
#endif

extern int mcuio_rddw(const struct mcuio_range *, unsigned offset,
		      uint32_t *out, int fill);
#ifdef ARCH_IS_HARVARD
extern int mcuio_rddw_ram(const struct mcuio_range *, unsigned offset,
			  uint32_t *out, int fill);
#else
#define mcuio_rddw_ram mcuio_rddw
#endif

extern int mcuio_rdq(const struct mcuio_range *, unsigned offset,
		     uint32_t *out, int fill);
#ifdef ARCH_IS_HARVARD
extern int mcuio_rdq_ram(const struct mcuio_range *, unsigned offset,
			 uint32_t *out, int fill);
#else
#define mcuio_rdq_ram mcuio_rdq
#endif

extern int mcuio_wrb(const struct mcuio_range *, unsigned offset,
		     const uint32_t *in, int fill);
extern int mcuio_wrw(const struct mcuio_range *, unsigned offset,
		     const uint32_t *in, int fill);
extern int mcuio_wrdw(const struct mcuio_range *, unsigned offset,
		      const uint32_t *in, int fill);
extern int mcuio_wrq(const struct mcuio_range *, unsigned offset,
		     const uint32_t *in, int fill);

extern const struct mcuio_range_ops default_mcuio_range_ops;
extern const struct mcuio_range_ops default_mcuio_range_ro_ops;
extern const struct mcuio_range_ops default_mcuio_range_ram_ops;
extern const struct mcuio_range_ops default_mcuio_range_ro_ram_ops;



#endif /* __MCUIO_FUNCTION_H__ */
