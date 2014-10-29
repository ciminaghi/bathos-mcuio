/*
 * Hardware registers we use, defined for use in the regs[] abstraction
 * Alessandro Rubini, 2009-2012 GNU GPL2 or later
 */
#ifndef __ARM_HW_H__
#define __ARM_HW_H__

#include <stdint.h>
#include <bathos/io.h>

#include <mach/hw.h>

extern void clocks_init(void);
extern void machine_ll_init(void);


#endif /* __ARM_HW_H__ */
