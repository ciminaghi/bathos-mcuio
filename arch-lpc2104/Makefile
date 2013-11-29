
TASK-y = task-uart.o arch/task-led.o arch/task-pwm.o arch/task-spi.o

# Library
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/gpio.o $(ADIR)/spi.o

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)
