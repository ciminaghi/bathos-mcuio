# MCUIO Makefile

ifeq ($(CONFIG_TASK_LININOIO),y)
CFLAGS += -I$(patsubst "%",%,$(CONFIG_LININOIO_EXT_PACKAGE))/include
endif

obj-$(CONFIG_TASK_LININOIO) += lininoio/core.o
