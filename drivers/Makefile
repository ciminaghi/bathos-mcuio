
# Drivers Makefile: just add the objects and names

LIBDRIVERS = drivers/libdrivers.a
DRV_OBJS += drivers/enc28j60.o

DRV_OBJS += drivers/w1.o
DRV_OBJS += drivers/w1-gpio.o
DRV_OBJS += drivers/w1-temp.o
DRV_OBJS += drivers/w1-eeprom.o
DRV_OBJS += drivers/dev.o

ifeq ($(CONFIG_HAVE_USB_GENERIC_STACK),y)
DRV_OBJS += drivers/usb-cdc-acm.o
DRV_OBJS += drivers/usb-device-controller.o drivers/usb-device-ep.o
DRV_OBJS += drivers/usb-descriptors.o drivers/usb-data.o
endif

ifeq ($(CONFIG_DEBUG_BITBANG),y)
DRV_OBJS += drivers/debug_bitbang.o
endif

ifeq ($(CONFIG_FREESCALE_DRIVERS),y)
  DRV_OBJS += drivers/freescale-uart.o
  DRV_OBJS += drivers/usb-device-freescale.o
endif

$(LIBDRIVERS): $(DRV_OBJS)
	$(AR) r $@ $(DRV_OBJS)

LIBS += $(LIBDRIVERS)
LDFLAGSS += -Ldrivers -ldrivers

drivers/usb-descriptors.o: drivers/usb-descriptors.c

drivers/usb-descriptors.c: $(patsubst "%",%,$(CONFIG_USB_CONFIG_FILE))
	scripts/gen_usb_descr $< $@

drivers/usb-data.o: drivers/usb-data.c


drivers/usb-data.c: $(patsubst "%",%,$(CONFIG_USB_CONFIG_FILE))
	CONFIG_MACH=$(CONFIG_MACH) scripts/gen_usb_data $< $@
