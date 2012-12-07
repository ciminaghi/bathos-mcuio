# Architecture choice
ARCH ?= lpc1343

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

# Using thumb for version 7 ARM core:
CFLAGS  = -march=armv7-m -mthumb -g -Wall -ffreestanding -O2
ASFLAGS = -march=armv7-m -mthumb -g -Wall

# Files that depend on the architecture
ADIR = arch-$(ARCH)
AOBJ = $(ADIR)/boot.o $(ADIR)/io.o
LDS = $(ADIR)/bathos.lds

# Use our own linker script
LDFLAGS = -T $(LDS)

# Task source files and objects
TSRC = $(wildcard task-*.c)
TOBJ = $(TSRC:.c=.o)

# Temporary
CFLAGS += -I.

# Our target
bathos.bin: bathos
	$(OBJCOPY) -O binary $^ $@

bathos: main.o $(AOBJ) $(TOBJ)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -f bathos.bin bathos *.o *~
	find . -name '*.o' -o -name '*~' | xargs rm -f
