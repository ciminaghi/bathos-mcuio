
# Drivers Makefile: just add the objects and names

LIBDRIVERS = drivers/libdrivers.a
DRV_OBJS += drivers/enc28j60.o

DRV_OBJS += drivers/w1.o
DRV_OBJS += drivers/w1-gpio.o
DRV_OBJS += drivers/w1-temp.o
DRV_OBJS += drivers/w1-eeprom.o
DRV_OBJS += drivers/dev.o
DRV_OBJS += drivers/usb-cdc-acm.o
DRV_OBJS += drivers/radio-bridge-master.o
DRV_OBJS += drivers/radio-bridge-slave.o

ifeq ($(CONFIG_DEBUG_BITBANG),y)
DRV_OBJS += drivers/debug_bitbang.o
endif

ifeq ($(CONFIG_MBED_DRIVERS),y)
  DRV_OBJS += drivers/mbed/adc.o
endif

ifeq ($(CONFIG_FREESCALE_DRIVERS),y)
  DRV_OBJS += drivers/freescale-uart.o
endif

ifeq ($(CONFIG_NRF51_DRIVERS),y)
  DRV_OBJS += drivers/nrf51-rtc.o
  DRV_OBJS += drivers/nrf51-uart.o
  DRV_OBJS += drivers/nrf51-radio.o
endif

$(LIBDRIVERS): $(DRV_OBJS)
	$(AR) r $@ $(DRV_OBJS)

LIBS += $(LIBDRIVERS)
LDFLAGSS += -Ldrivers -ldrivers
