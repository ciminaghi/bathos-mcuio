# The user can set BATHOS_MCU on the command line or environment
# The first item is for CFLAGS, the second for avrdude
BATHOS_MCU ?= atmega8 m8

# Similary, the user can set TTY= when running "make load"
TTY ?= /dev/ttyUSB0

# By default, we only have the uart tasks
TASK-y ?= task-uart.o

# Following stuff is calculated, no further customization expected

CFLAGS  = -mmcu=$(word 1, $(BATHOS_MCU))
LDFLAGS = -nostdlib
ASFLAGS = -mmcu=$(word 1, $(BATHOS_MCU))

# Library: we have one assembly source file
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/jiffies.o

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)

# and the load target, not there in other architectures
load: bathos.hex
	avrdude -p $(word 2, $(BATHOS_MCU)) \
		-c stk500v2 -P $(TTY) -e -U flash:w:bathos.hex

%.hex: %
	$(OBJCOPY) -O ihex $^ $@

clean: cleanhex

cleanhex:
	rm -f bathos.hex