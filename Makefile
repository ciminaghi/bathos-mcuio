# Read .config first (if any) and use those values
# But we must remove the quotes from these Kconfig values
-include $(CURDIR)/.config
ARCH ?= $(patsubst "%",%,$(CONFIG_ARCH))
CROSS_COMPILE ?= $(patsubst "%",%,$(CONFIG_CROSS_COMPILE))
MODE ?= $(patsubst "%",%,$(CONFIG_MEMORY_MODE))

# if no .config is there, ARCH is still empty, this would prevent a simple
# "make config"
ifeq ($(ARCH),)
  ARCH = lpc1343
endif

# Any tasks coming from configuration ?
ifndef TASK-y
  ifeq ($(CONFIG_TASK_MCUIO),y)
    TASK-y+=task-mcuio.o
  endif
  ifeq ($(CONFIG_MCUIO_ZERO),y)
    TASK-y+=mcuio_zero_func.o
  endif
  ifeq ($(CONFIG_MCUIO_GPIO),y)
    TASK-y+=mcuio_gpio_func.o
  endif
endif

# First: the target. After that, we can include the arch Makefile
all: bathos.bin

ADIR = arch-$(ARCH)
include $(ADIR)/Makefile

# Task choice. This follows the -y convention, to allow use of $(CONFIG_STH) 
# The arch may have its choice, or you can override on the command line
TASK-y ?= task-uart.o

# Cross compiling:
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
CPP             = $(CC) -E
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump

export CC OBJDUMP

# Files that depend on the architecture (bathos.lds may be missing)
AOBJ  += $(ADIR)/boot.o $(ADIR)/io.o

# The user can pass USER_CFLAGS if needed
CFLAGS += $(USER_CFLAGS) -DMODULE_NAME=$(subst -,_,$(subst /,_,$(subst .o,,$@)))

# There may or may not be a linker script (arch-unix doesn't)
LDS   = $(wildcard $(ADIR)/bathos$(MODE).lds)

# Lib objects and flags
LOBJ = pp_printf/printf.o pp_printf/vsprintf-xint.o
CFLAGS  += -Ipp_printf -DCONFIG_PRINT_BUFSIZE=256

# Use our own linker script, if it exists
LDFLAGS += $(patsubst %.lds, -T %.lds, $(LDS))

# Each architecture can have specific drivers
LDFLAGS += $(LIBARCH)

# We have drivers too, let its Makefile do it all
include drivers/Makefile

# Same for the generic library
include lib/Makefile

# As the system goes larger, we need libgcc to resolve missing symbols
LDFLAGS += $(shell $(CC) $(CFLAGS) --print-libgcc-file-name)


# Task object files. All objects are placed in tasks/ but the source may
# live in tasks-$(ARCH), to allow similar but different implementations
TOBJ := $(patsubst %, tasks/%, $(TASK-y))
TOBJ := $(patsubst tasks/arch/%, tasks-$(ARCH)/%, $(TOBJ))
VPATH := tasks-$(ARCH)

ifeq ($(CONFIG_MCUIO_GPIO),y)
  ifndef MCUIO_GPIO_CONFIG_FILE
    ifeq ($(CONFIG_MCUIO_GPIO_MAP_YUN),y)
      MCUIO_GPIO_CONFIG_FILE=yun-gpios.cfg
    else
	ifeq ($(CONFIG_MCUIO_GPIO_MAP_ATMEGA32U4),y)
          MCUIO_GPIO_CONFIG_FILE=generic-atmega32u4-gpios.cfg
        endif
    endif
  endif
  GPIOS_NAMES_FILE = $(patsubst %.cfg, tasks/%-names.o,\
	$(MCUIO_GPIO_CONFIG_FILE))
  GPIOS_CAPS_FILE = $(patsubst %.cfg, tasks/%-caps.o, $(MCUIO_GPIO_CONFIG_FILE))
  TOBJ += $(GPIOS_NAMES_FILE) $(GPIOS_CAPS_FILE)
  MCUIO_NGPIO := $(shell scripts/get_ngpios tasks/$(MCUIO_GPIO_CONFIG_FILE))
  CFLAGS += -DMCUIO_NGPIO=$(MCUIO_NGPIO)
endif

# Generic flags
CFLAGS  += -Iinclude -I$(ADIR)
CFLAGS  += -g -Wall -ffreestanding -Os
ASFLAGS += -g -Wall

# Our target
bathos.bin: bathos
	$(OBJCOPY) -O binary $^ $@

bathos: bathos.o
	$(CC) bathos.o $(LDFLAGS) -o $@

obj-y =  main.o sys_timer.o periodic_scheduler.o pipe.o \
$(AOBJ) $(TOBJ) $(LOBJ) $(LIBARCH) $(LIBS)

bathos.o: silentoldconfig $(obj-y)
	$(LD) -r -T bigobj.lds $(obj-y) -o $@

$(GPIOS_NAMES_FILE): tasks/%-names.o: tasks/%.cfg
	export CC=$(CC) OBJDUMP=$(OBJDUMP) OBJCOPY=$(OBJCOPY) ; \
	scripts/gen_gpios_names $$(scripts/get_bin_format) $< $@

$(GPIOS_CAPS_FILE): tasks/%-caps.o: tasks/%.cfg
	export CC=$(CC) LD=${LD} OBJDUMP=$(OBJDUMP) OBJCOPY=$(OBJCOPY) ; \
	scripts/gen_gpios_capabilities $$(scripts/get_bin_format) $< $@

clean:
	rm -f bathos.bin bathos *.o *~
	find . -name '*.o' -o -name '*~' -o -name '*.a' | \
		grep -v scripts/kconfig | xargs rm -f

# following targets from Makefile.kconfig
silentoldconfig:
	@mkdir -p include/config
	$(MAKE) -f Makefile.kconfig $@

scripts_basic config %config:
	$(MAKE) -f Makefile.kconfig $@

defconfig:
	@echo "Using lpc1343_defconfig"
	@test -f .config || touch .config
	@$(MAKE) -f Makefile.kconfig lpc1343_defconfig

.config: silentoldconfig
