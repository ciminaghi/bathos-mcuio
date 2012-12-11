/* basic types used in several places */
#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#endif /* __TYPES_H__ */
