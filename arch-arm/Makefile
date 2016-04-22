
ifeq ($(CONFIG_CPU),"cortex-m0")
# Using thumb for version 7 ARM core:
CFLAGS = -mcpu=cortex-m0 -mthumb -mfloat-abi=soft
CPU_DIR = $(ADIR)/cpu-cortex-m/
endif

ifeq ($(CONFIG_CPU),"cortex-m4")
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CPU_DIR = $(ADIR)/cpu-cortex-m/
endif

ifeq ($(CONFIG_CPU),"cortex-m4f")
CFLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard
CPU_DIR = $(ADIR)/cpu-cortex-m/
endif

ifeq ($(CONFIG_CPU),"arm926ej-s")
CPU_DIR = $(ADIR)/cpu-arm926ej-s
endif

ASFLAGS = $(CFLAGS)
LDFLAGS = -nostdlib -nostartfiles -Wl,--defsym -Wl,regs=0

ifeq ($(CONFIG_OPENSDA_DEBUG),y)
ASFLAGS += -DOPENSDA_DEBUG
endif

MACH_DIR = $(ADIR)/mach-$(shell echo $(CONFIG_MACH))

CFLAGS += -I$(MACH_DIR) -DARCH_HAS_BUILTIN_FFS

ifneq ($(FAMILY),)
FAMILY_DIR = $(ADIR)/family-$(FAMILY)
CFLAGS += -I$(FAMILY_DIR)/
endif

ifneq ($(CONFIG_MACH),"none")
BOOT_OBJ = $(MACH_DIR)/init.o
endif
BOOT_OBJ += $(CPU_DIR)/boot.o $(CPU_DIR)/idle.o

# No io.o for this arch
IO_OBJ =

# Library
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/init.o

AOBJ += $(CPU_DIR)/vectable.o

ifeq ($(CONFIG_HAS_CORTEXM_NVIC),y)
AOBJ += $(CPU_DIR)/nvic.o
endif

ifeq ($(CONFIG_HAS_CORTEXM_SYSTICKTMR),y)
CFLAGS += -DARCH_NEEDS_INTERRUPT_FOR_JIFFIES=1
AOBJ += $(CPU_DIR)/systicktmr.o
endif

ifeq ($(CONFIG_MACH),"kl25z")
AOBJ += $(MACH_DIR)/kl25.o $(MACH_DIR)/clock.o $(MACH_DIR)/gpio.o \
	$(MACH_DIR)/usb-device.o
endif

ifeq ($(CONFIG_MACH),"versatile")
AOBJ += $(MACH_DIR)/irq.o $(MACH_DIR)/timer.o $(MACH_DIR)/versatile-pb.o
endif

ifeq ($(CONFIG_FAMILY),"nrf5x")
AOBJ += $(FAMILY_DIR)/clock.o
endif

ifeq ($(CONFIG_MACH),"nrf51822")
CFLAGS += -I$(FAMILY_SUBDIR)/family
AOBJ += $(MACH_DIR)/nrf51822.o $(MACH_DIR)/quirks.o
endif

ifeq ($(CONFIG_FAMILY),"stm32f4x")
CFLAGS += -I$(FAMILY_SUBDIR)/family
AOBJ += $(FAMILY_DIR)/clocks.o
endif

ifeq ($(CONFIG_BOARD_STAR),y)
AOBJ += $(MACH_DIR)/board-star.o
endif

LDS = $(wildcard $(MACH_DIR)/bathos$(MODE).lds)

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)

run_versatile: bathos.bin
	qemu-system-arm -M versatilepb -m 128M -kernel bathos.bin -nographic
