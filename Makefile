# Architecture choice
ARCH ?= lpc1343

# Task choice
TSRC ?= task-uart.c task-led.c task-pwm.c

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
ADIR = arch-$(ARCH)
include $(ADIR)/Makefile
AOBJ  = $(ADIR)/boot.o $(ADIR)/io.o
LDS   = $(ADIR)/bathos.lds

# Use our own linker script
LDFLAGS = -T $(LDS)

# Task object files
TOBJ = $(TSRC:.c=.o)

# Generic flags
CFLAGS  += -Iinclude -I$(ADIR)
CFLAGS  += -g -Wall -ffreestanding -Os
ASFLAGS += -g -Wall

# Our target
bathos.bin: bathos
	$(OBJCOPY) -O binary $^ $@

bathos: main.o $(AOBJ) $(TOBJ)
	$(LD) $^ $(LDFLAGS) -o $@

clean:
	rm -f bathos.bin bathos *.o *~
	find . -name '*.o' -o -name '*~' | xargs rm -f
