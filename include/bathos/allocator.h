
#ifndef __BATHOS_ALLOC_H__
#define __BATHOS_ALLOC_H__

#define BATHOS_NORDERS_LOG	3
#define BATHOS_NORDERS		(1 << BATHOS_NORDERS_LOG)
#define BATHOS_MAX_ORDER	((BATHOS_NORDERS) - 1)
/* How many bytes are managed by the allocator */
#define BATHOS_ALLOCATOR_MEMORY 512

/* Returns number of buffers for order o */
#define BATHOS_NBUFS(o) (1 << ((BATHOS_NORDERS - (o)) - 1))
/* Returns size of buffer for order o */
#define BATHOS_BUF_SIZE(o) \
	(BATHOS_ALLOCATOR_MEMORY / BATHOS_NBUFS(o))

void *bathos_alloc_buffer(int size);

/*
 * Pass size again to avoid storing allocation size somewhere
 * bathos_alloc_buffer is not a malloc replacement, it is used in contexts
 * where allocation size is constant and known at build time.b
 */
void bathos_free_buffer(void *, int size);

int bathos_alloc_init(void);

/*
 * This is also used by scripts/allocator_aux_gen.c
 */
static inline int __index_to_bit(int index, int order)
{
	int out;
	/*
	 * First part:
	 * sum of nbufs(0), nbufs(1), ..., nbufs(order - 1)
	 *
	 * 
	 */
	out = (1 << BATHOS_NORDERS) - (1 << (BATHOS_NORDERS - order));
	/*
	 * Second part:
	 * Add index / (1 << order) (see diagram on top of file allocator.c)
	 * which is equal to index >> order
	 */
	return out + (index >> order);
}


#endif /* __BATHOS_ALLOC_H__ */
