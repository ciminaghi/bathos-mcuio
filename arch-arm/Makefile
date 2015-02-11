
ifeq ($(CONFIG_CPU),"cortex-m0")
# Using thumb for version 7 ARM core:
CFLAGS = -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
endif
ASFLAGS = $(CFLAGS)
LDFLAGS = -nostdlib -nostartfiles -Wl,--defsym -Wl,regs=0

ifeq ($(CONFIG_OPENSDA_DEBUG),y)
ASFLAGS += -DOPENSDA_DEBUG
endif

CPU_DIR = cpu-$(shell echo $(CONFIG_CPU))
MACH_DIR = $(ADIR)/mach-$(shell echo $(CONFIG_MACH))

CFLAGS += -I$(ADIR)/$(MACH_DIR) -DHZ=100

ifneq ($(CONFIG_MACH),"none")
BOOT_OBJ = $(MACH_DIR)/boot.o $(MACH_DIR)/init.o
else
BOOT_OBJ =
endif

# No io.o for this arch
IO_OBJ =

# Library
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/init.o

AOBJ += $(ADIR)/vectable.o $(ADIR)/nvic.o

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)
