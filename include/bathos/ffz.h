#ifndef __FFZ_H__
#define __FFZ_H__

#include <bathos/ffs.h>

/*
 * ffz - find first zero in word.
 * @word: The word to search
 *
 */
#define ffz(x) ffs(~(x))

#endif /* __FFZ_H__ */
