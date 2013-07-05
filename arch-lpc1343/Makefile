
# Using thumb for version 7 ARM core:
CFLAGS  = -march=armv7-m -mthumb
ASFLAGS = -march=armv7-m -mthumb

# default is run in RAM memory
BATHOS_MODE = -ram

# Library
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/gpio.o

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)
