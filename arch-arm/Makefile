
ifeq ($(CONFIG_CPU),"cortex-m0")
# Using thumb for version 7 ARM core:
CFLAGS = -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
ASFLAGS = $(CFLAGS)
LDFLAGS = -nostdlib -nostartfiles
endif

CPU_DIR = cpu-$(shell echo $(CONFIG_CPU))
MACH_DIR = mach-$(shell echo $(CONFIG_MACH))

CFLAGS += -I$(ADIR)/$(MACH_DIR)

BOOT_OBJ = $(ADIR)/cpu-$(shell echo $(CONFIG_CPU))/boot.o
# No io.o for this arch
IO_OBJ =
MACH_SUBDIR = $(ADIR)/mach-$(shell echo $(CONFIG_MACH))

# Library
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/init.o

ifeq ($(CONFIG_MACH_KL25Z),y)
LIBARCH_OBJS += $(MACH_SUBDIR)/init.o $(MACH_SUBDIR)/clock.o
LDS = $(MACH_SUBDIR)/bathos.lds
endif

# FIXME: add lpc1343 and 2104 files here

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)
