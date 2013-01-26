
# Library Makefile: just add the objects and names

LIBBATHOS = lib/libbathos.a
LIB_OBJS += lib/string.o
LIB_OBJS += lib/stdio.o
LIB_OBJS += lib/ctype.o

$(LIBBATHOS): $(LIB_OBJS)
	$(AR) r $@ $(LIB_OBJS)

LIBS += lib/libbathos.a
LDFLAGSS += -Llib -lbathos
