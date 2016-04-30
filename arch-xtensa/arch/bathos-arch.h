#ifndef __ARM_BATHOS_ARCH_H__
#define __ARM_BATHOS_ARCH_H__

#define PROGMEM

#define interrupt_disable(a) a=0
#define interrupt_restore(a) a*=2

#endif /* __BATHOS_ARCH_H__ */

