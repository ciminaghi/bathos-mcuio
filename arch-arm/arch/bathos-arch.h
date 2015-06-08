#include <generated/autoconf.h>

#define PROGMEM

#if defined CONFIG_CPU_CORTEX_M0 || defined CONFIG_CPU_CORTEX_M0_PLUS || \
    defined CONFIG_CPU_CORTEX_M3
#include <arch/bathos-arch-cortex-m.h>
#endif

#if defined CONFIG_CPU_ARM926EJS
#include <arch/bathos-arch-arm926ej-s.h>
#endif

#if defined CONFIG_MACH_VERSATILE
#include <mach/jiffies.h>
#endif
