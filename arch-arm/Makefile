
ifeq ($(CONFIG_CPU),"cortex-m0")
# Using thumb for version 7 ARM core:
CFLAGS = -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
ASFLAGS = $(CFLAGS)
LDFLAGS = -nostdlib -nostartfiles
endif

ifeq ($(CONFIG_OPENSDA_DEBUG),y)
ASFLAGS += -DOPENSDA_DEBUG
endif

CPU_DIR = cpu-$(shell echo $(CONFIG_CPU))
MACH_DIR = mach-$(shell echo $(CONFIG_MACH))

CFLAGS += -I$(ADIR)/$(MACH_DIR)

BOOT_OBJ = $(ADIR)/$(MACH_DIR)/boot.o $(MACH_SUBDIR)/init.o
# No io.o for this arch
IO_OBJ =
MACH_SUBDIR = $(ADIR)/mach-$(shell echo $(CONFIG_MACH))

# Library
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/init.o

ifeq ($(CONFIG_MACH_KL25Z),y)
LIBARCH_OBJS += $(MACH_SUBDIR)/clock.o $(MACH_SUBDIR)/jiffies.o
AOBJ += $(MACH_SUBDIR)/idle.o
LDS = $(MACH_SUBDIR)/bathos.lds
endif

# FIXME: add lpc1343 and 2104 files here

ifeq ($(CONFIG_MBED_DRIVERS),y)
  MBED_DIR = $(shell echo $(CONFIG_MBED_PATH))

  CFLAGS += -I$(MBED_DIR)/libraries/mbed/hal \
    -I$(MBED_DIR)/libraries/mbed/targets/cmsis \
    -I$(MBED_DIR)/libraries/mbed/api

  include $(ADIR)/$(MACH_DIR)/mbed.mk
endif

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)
