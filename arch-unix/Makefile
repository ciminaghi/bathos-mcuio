
# The default task list is very limited...
TASK-y ?= task-uart.o

# Library
LIBARCH = $(ADIR)/libarch.a
LIBARCH_OBJS = $(ADIR)/jiffies.o

$(ADIR)/libarch.a: $(LIBARCH_OBJS)
	$(AR) r $@ $(LIBARCH_OBJS)

# We need librt for clock_gettime. Bah.
LDFLAGS += -lrt -lpthread -lc