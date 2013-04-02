
# Drivers Makefile: just add the objects and names

LIBDRIVERS = drivers/libdrivers.a
DRV_OBJS += drivers/enc28j60.o

DRV_OBJS += drivers/w1.o
DRV_OBJS += drivers/w1-gpio.o
DRV_OBJS += drivers/w1-temp.o

$(LIBDRIVERS): $(DRV_OBJS)
	$(AR) r $@ $(DRV_OBJS)

LIBS += $(LIBDRIVERS)
LDFLAGSS += -Ldrivers -ldrivers
