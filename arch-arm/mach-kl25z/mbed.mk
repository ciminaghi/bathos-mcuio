# mbed library CFLAGS and LDFLAGS

CFLAGS += -I$(MBED_DIR)/libraries/mbed/targets/hal/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z
CFLAGS += -I$(MBED_DIR)/libraries/mbed/targets/hal/TARGET_Freescale/TARGET_KLXX
CFLAGS += -I$(MBED_DIR)/libraries/mbed/targets/cmsis/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z

LDFLAGS += $(MBED_DIR)/build/mbed/TARGET_KL25Z/TOOLCHAIN_GCC_ARM/libmbed.a
LDFLAGS += $(MBED_DIR)/build/mbed/TARGET_KL25Z/TOOLCHAIN_GCC_ARM/system_MKL25Z4.o
LDFLAGS += $(MBED_DIR)/build/mbed/TARGET_KL25Z/TOOLCHAIN_GCC_ARM/cmsis_nvic.o
