# Default architecture. We have "lpc1343" and "lpc2104" (look for arch-*)
ARCH ?= lpc1343

# Task choice. Follow the -y convention, to allow use of $(CONFIG_STH) 
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

all: bathos.bin

# Files that depend on the architecture
ADIR = arch-$(ARCH)
include $(ADIR)/Makefile
AOBJ  = $(ADIR)/boot.o $(ADIR)/io.o
LDS   = $(ADIR)/bathos.lds

# Lib objects and flags
LOBJ = pp_printf/printf.o pp_printf/vsprintf-xint.o
CFLAGS  += -Ipp_printf -DCONFIG_PRINT_BUFSIZE=256

# Use our own linker script
LDFLAGS = -T $(LDS)

# Each architecture can have specific drivers
LDFLAGS += $(LIBARCH)

# As the system goes larger, we need libgcc to resolve missing symbols
LDFLAGS += $(shell $(CC) --print-libgcc-file-name)


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

bathos: main.o $(AOBJ) $(TOBJ) $(LOBJ) $(LIBARCH)
	$(LD) $^ $(LDFLAGS) -o $@

clean:
	rm -f bathos.bin bathos *.o *~
	find . -name '*.o' -o -name '*~' | xargs rm -f
