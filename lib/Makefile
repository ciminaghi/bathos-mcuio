
# Library Makefile: just add the objects and names

LIBBATHOS = lib/libbathos.a
LIB_OBJS += lib/string.o
LIB_OBJS += lib/stdio.o
LIB_OBJS += lib/stdlib.o
LIB_OBJS += lib/ctype.o
LIB_OBJS += lib/setup.o
LIB_OBJS += lib/udelay.o
LIB_OBJS += lib/event.o
LIB_OBJS += lib/statemachine.o
LIB_OBJS += lib/allocator.o
LIB_OBJS += lib/allocator-tables.o
LIB_OBJS += lib/shell.o

$(LIBBATHOS): $(LIB_OBJS)
	$(AR) r $@ $(LIB_OBJS)

lib/allocator-tables.c : scripts/allocator_aux_gen
	$< > $@

scripts/allocator_aux_gen: scripts/allocator_aux_gen.c
	$(HOSTCC) $(HOST_CFLAGS) -DBITS_PER_LONG=$(BITS_PER_LONG) \
	-o $@ $<

LIBS += lib/libbathos.a
LDFLAGSS += -Llib -lbathos
