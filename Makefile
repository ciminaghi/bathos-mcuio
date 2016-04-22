# Read .config first (if any) and use those values
# But we must remove the quotes from these Kconfig values
-include $(CURDIR)/.config
ARCH ?= $(patsubst "%",%,$(CONFIG_ARCH))
MACH ?= $(patsubst "%",%,$(CONFIG_MACH))
BOARD ?= $(patsubst "%",%,$(CONFIG_BOARD))
FAMILY ?= $(patsubst "%",%,$(CONFIG_FAMILY))
PACKAGE = $(patsubst "%",%,$(CONFIG_MACH_PACKAGE))
CROSS_COMPILE ?= $(patsubst "%",%,$(CONFIG_CROSS_COMPILE))
MODE ?= $(patsubst "%",%,$(CONFIG_MEMORY_MODE))
BATHOS_GIT=$(shell ./scripts/get_version)
HZ ?= $(patsubst "%",%,$(CONFIG_HZ))
THOS_QUARTZ ?= $(patsubst "%",%,$(CONFIG_THOS_QUARTZ))
BUILD_DIR ?= $(shell pwd)

# if no .config is there, ARCH is still empty, this would prevent a simple
# "make config"
ifeq ($(ARCH),)
  ARCH = atmega
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
  ifeq ($(CONFIG_MCUIO_ADC), y)
    TASK-y+= mcuio_adc_func.o
  endif
  ifeq ($(CONFIG_MCUIO_PWM), y)
    TASK-y+= mcuio_pwm_func.o
  endif
  ifeq ($(CONFIG_MCUIO_IRQ_CONTROLLER_MSG),y)
    TASK-y+=mcuio_irq_controller_msg.o
  endif
  ifeq ($(CONFIG_MCUIO_SHIELD), y)
    TASK-y+=mcuio_shield_func.o
  endif
  ifeq ($(CONFIG_MCUIO_BITBANG_I2C), y)
    TASK-y+=mcuio_bitbang_i2c_func.o
  endif
  ifeq ($(CONFIG_MCUIO_IRQ_TEST), y)
    TASK-y+=mcuio_irq_test_func.o
  endif
  ifeq ($(CONFIG_TASK_LOOPBACK_PIPE), y)
    TASK-y+=task-loopback-pipe.o
  endif
endif

ifeq ($(CONFIG_NR_INTERRUPTS),)
NR_INTERRUPTS=0
INT_EVENTS_OBJS=
INT_EVENTS_OBJ=
else
NR_INTERRUPTS:=$(CONFIG_NR_INTERRUPTS)
INT_EVENTS_OBJS=$(foreach i,$(shell seq 0 $$(($(NR_INTERRUPTS) - 1))),\
		  interrupt_event_$i.o)
INT_EVENTS_OBJ=interrupt_events.o
endif


# First: the target. After that, we can include the arch Makefile
all: bathos.bin bathos.hex

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

export CC OBJDUMP OBJCOPY LD ARCH BUILD_DIR

# host gcc
HOSTCC ?= gcc
HOST_CFLAGS ?= -Iinclude

BOOT_OBJ ?= $(ADIR)/boot.o
IO_OBJ ?= $(ADIR)/io.o

# Files that depend on the architecture (bathos.lds may be missing)
AOBJ  += $(BOOT_OBJ) $(IO_OBJ)

# The user can pass USER_CFLAGS if needed
CFLAGS += $(USER_CFLAGS) -DMACH=$(MACH) -DBATHOS_GIT=\"$(BATHOS_GIT)\" -DMODULE_NAME=$(subst -,_,$(subst /,_,$(subst .o,,$@))) -DHZ=$(HZ) -DTHOS_QUARTZ=$(THOS_QUARTZ)

# There may or may not be a linker script (arch-unix doesn't)
LDS   ?= $(wildcard $(ADIR)/bathos$(MODE).lds)

# Lib objects and flags
LOBJ = pp_printf/printf.o pp_printf/vsprintf-xint.o
CFLAGS  += -Ipp_printf -DCONFIG_PRINT_BUFSIZE=100

# Use our own linker script, if it exists
LDFLAGS += $(patsubst %.lds, -T %.lds, $(LDS))

# Each architecture can have specific drivers
LDFLAGS += $(LIBARCH)

# Add $(LIBDRIVERS) because console_init() console_putc() could be stored there
LDFLAGS += $(LIBDRIVERS)

# This is currently needed by the bathos allocator
# Default value of BITS_PER_LONG is 32, can be overridden in arch makefile
BITS_PER_LONG ?= 32
CFLAGS += -DBITS_PER_LONG=$(BITS_PER_LONG)

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
  # Find out name for gpio config file
  # For generic boards (any), pick a cfg file based on package variant
  __PACKAGE=$(shell echo $(BOARD) | grep "any" && echo $(PACKAGE)-)
  MCUIO_GPIO_CONFIG_FILE=$(ARCH)-$(BOARD)-$(__PACKAGE)gpios.cfg
  GPIOS_NAMES_FILE = $(patsubst %.cfg, tasks/%-names.o,\
	$(MCUIO_GPIO_CONFIG_FILE))
  GPIOS_CAPS_FILE = $(patsubst %.cfg, tasks/%-caps.o, $(MCUIO_GPIO_CONFIG_FILE))
  TOBJ += $(GPIOS_NAMES_FILE) $(GPIOS_CAPS_FILE)
  MCUIO_TOT_NGPIO = $(shell scripts/get_ngpios tasks/$(MCUIO_GPIO_CONFIG_FILE))
  MCUIO_GPIO_NPORTS = $(shell scripts/get_ngpio_ports $(MCUIO_TOT_NGPIO) 64)
  MCUIO_TABLES_OBJS = $(foreach i, \
			 $(shell seq 0 $$(($(MCUIO_GPIO_NPORTS) - 1))),\
			 tasks/mcuio_gpio_table_$i.o)
  TOBJ += $(MCUIO_TABLES_OBJS)
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

%.hex: %
	$(OBJCOPY) -O ihex $^ $@

# This target is needed to generate a default version of the gpio config file
# Will be removed when all boards have their gpio config file.
tasks/$(MCUIO_GPIO_CONFIG_FILE):
	scripts/gen_default_gpio_config_file $(ARCH) $(BOARD) $@

$(MCUIO_TABLES_OBJS): tasks/mcuio_gpio_table_%.o : tasks/mcuio_gpio_table.c tasks/$(MCUIO_GPIO_CONFIG_FILE)
	$(CC) $(CFLAGS) -DMCUIO_GPIO_PORT=$* \
	-DMCUIO_NGPIO=$$(scripts/get_port_ngpios $* 64 $(MCUIO_TOT_NGPIO)) \
	-c -o $@ $<

obj-y =  main.o sys_timer.o periodic_scheduler.o pipe.o version_data.o \
$(INT_EVENTS_OBJ) $(AOBJ) $(TOBJ) $(LOBJ) $(LIBARCH) $(LIBS)

$(INT_EVENTS_OBJ): $(INT_EVENTS_OBJS)
	$(LD) -r -o $@ $+

interrupt_event_%.o: interrupt_event.c
	$(CC) $(CFLAGS) -DINTNO=$* -c -o $@ $<

bathos.o: silentoldconfig $(obj-y) $(ARCH_EXTRA)
	$(LD) -r -T bigobj.lds $(obj-y) -o $@

version_data.o:
	export CC=$(CC) OBJDUMP=$(OBJDUMP) OBJCOPY=$(OBJCOPY) ; \
	scripts/gen_version_data $(BATHOS_GIT) $$(scripts/get_bin_format) \
	$@

$(GPIOS_NAMES_FILE): tasks/%-names.o: tasks/%.cfg main.o
	for p in $$(seq 0 $$(($(MCUIO_GPIO_NPORTS) - 1))) ; do \
		offs=$$(($$p * 64)) \
		ngpios=$$(scripts/get_port_ngpios $$p 64 $(MCUIO_TOT_NGPIO)); \
		scripts/gen_gpios_names $$(scripts/get_bin_format) $< \
		$$p $$offs $$ngpios $(basename $@)_$$p.o ; \
	done
	$(LD) -r $(foreach p,$(shell seq 0 $$(($(MCUIO_GPIO_NPORTS)-1))), \
		$(basename $@)_$(p).o) -o $@
	if [ "$(ARCH)" = "arm" ] ; then scripts/arm_fix_elf $@ main.o ; fi

$(GPIOS_CAPS_FILE): tasks/%-caps.o: tasks/%.cfg main.o
	for p in $$(seq 0 $$(($(MCUIO_GPIO_NPORTS) - 1))) ; do \
		offs=$$(($$p * 64)) \
		ngpios=$$(scripts/get_port_ngpios $$p 64 $(MCUIO_TOT_NGPIO)); \
		scripts/gen_gpios_capabilities $$(scripts/get_bin_format) $< \
		$$p $$offs $$ngpios $(basename $@)_$$p.o ; \
	done
	$(LD) -r $(foreach p,$(shell seq 0 $$(($(MCUIO_GPIO_NPORTS)-1))), \
		$(basename $@)_$(p).o) -o $@
	if [ "$(ARCH)" = "arm" ] ; then scripts/arm_fix_elf $@ main.o ; fi

clean:
	rm -f bathos.bin bathos *.o *~
	rm -f drivers/usb-data.* drivers/usb-descriptors.*
	find . -name '*.o' -o -name '*~' -o -name '*.a' | \
		grep -v scripts/kconfig | xargs rm -f
	rm -f scripts/allocator_aux_gen
	rm -f lib/allocator-tables.o lib/allocator-tables.c

# following targets from Makefile.kconfig
silentoldconfig:
	@mkdir -p include/config
	$(MAKE) -f Makefile.kconfig $@

scripts_basic config:
	$(MAKE) -f Makefile.kconfig $@

%config:
	$(MAKE) -f Makefile.kconfig $@

defconfig:
	@echo "Using yun_defconfig"
	@test -f .config || touch .config
	@$(MAKE) -f Makefile.kconfig yun_defconfig

.config: silentoldconfig

.PHONY: version_data.o
