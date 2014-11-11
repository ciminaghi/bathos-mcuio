# The first item is for CFLAGS, the second for avrdude
ifeq ($(CONFIG_MACH),"atmega8")
BATHOS_MCU = atmega8 m8
else ifeq ($(CONFIG_MACH),"atmega32u4")
BATHOS_MCU = atmega32u4 m32u4
else
BATHOS_MCU = atmega8 m8
endif


# Similary, the user can set TTY= when running "make load"
TTY ?= /dev/ttyUSB0

# By default, we only have the uart tasks
TASK-y ?= task-uart.o

# Following stuff is calculated, no further customization expected

MCU=$(word 1, $(BATHOS_MCU))
CFLAGS  = -mmcu=$(MCU) -DMCU_$(MCU)=1 -funsigned-char -funsigned-bitfields \
-ffunction-sections -fpack-struct -fshort-enums -DHAVE_GPIO_EVENTS
LDFLAGS = -nostdlib -Wl,-Map=bathos.map -lc
ASFLAGS = -mmcu=$(word 1, $(BATHOS_MCU)) -Iinclude -I$(ADIR)

# Library: we have one assembly source file
LIBARCH = $(ADIR)/libarch.a

ifeq ($(MCU),atmega8)
LIBARCH_OBJS += $(ADIR)/atmega-a8.o
else ifeq ($(MCU),atmega32u4)
LIBARCH_OBJS += $(ADIR)/atmega-32u4.o
endif

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)

ifeq ($(MCU),atmega32u4)
AOBJ = $(ADIR)/main.o $(ADIR)/idle.o $(ADIR)/uart-mux-yun.o $(ADIR)/gpio.o \
	$(ADIR)/adc.o $(ADIR)/pwm.o
endif

ifeq ($(CONFIG_ATMEGA_USB_UART),y)
AOBJ += $(ADIR)/usb-uart.o
endif

ifeq ($(CONFIG_ATMEGA_UART),y)
AOBJ += $(ADIR)/uart.o
endif

ifeq ($(CONFIG_ATMEGA_SPI),y)
AOBJ += $(ADIR)/spi.o
endif

# and the load target, not there in other architectures
load: bathos.hex
	avrdude -p $(word 2, $(BATHOS_MCU)) \
		-c stk500v2 -P $(TTY) -e -U flash:w:bathos.hex

%.hex: %
	$(OBJCOPY) -O ihex $^ $@

clean: cleanhex

cleanhex:
	rm -f bathos.hex
