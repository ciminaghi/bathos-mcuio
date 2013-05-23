# Default architecture. We have several of them (look for arch-*)
ARCH ?= lpc1343

# First: the target. After that, we can include the arch Makefile
all: bathos.bin

ADIR = arch-$(ARCH)
include $(ADIR)/Makefile

# Task choice. This follows the -y convention, to allow use of $(CONFIG_STH) 
# The arch may have its choice, or you can override on the command line
TASK-y ?= task-uart.o arch/task-led.o arch/task-pwm.o

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

# Files that depend on the architecture
AOBJ  = $(ADIR)/boot.o $(ADIR)/io.o

# You can use "MODE=flash" on the command line", or BATHOS_MODE is from arch
ifneq ($(MODE),)
   BATHOS_MODE := -$(MODE)
endif

LDS   = $(ADIR)/bathos$(BATHOS_MODE).lds

# Lib objects and flags
LOBJ = pp_printf/printf.o pp_printf/vsprintf-xint.o
CFLAGS  += -Ipp_printf -DCONFIG_PRINT_BUFSIZE=256

# Use our own linker script
LDFLAGS += -T $(LDS)

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

# Generic flags
CFLAGS  += -Iinclude -I$(ADIR)
CFLAGS  += -g -Wall -ffreestanding -Os
ASFLAGS += -g -Wall

# Our target
bathos.bin: bathos
	$(OBJCOPY) -O binary $^ $@

bathos: bathos.o
	$(LD) $^ $(LDFLAGS) -o $@

bathos.o: main.o $(AOBJ) $(TOBJ) $(LOBJ) $(LIBARCH) $(LIBS)
	$(LD) -r -T bigobj.lds $^ -o $@

clean:
	rm -f bathos.bin bathos *.o *~
	find . -name '*.o' -o -name '*~' -o -name '*.a' | xargs rm -f
