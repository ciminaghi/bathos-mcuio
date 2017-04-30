# Main Makefile, sets up a build directory and uses it
BUILD_DIR ?= $(shell pwd)/build
SRC_DIR = $(shell pwd)
VPATH = $(SRC_DIR)
SCRIPTS = $(SRC_DIR)/scripts

ifneq ($(USER_SRC_DIR),)
VPATH += $(USER_SRC_DIR)
endif

export VPATH SRC_DIR BUILD_DIR obj-y USER_SRC_DIR

ifeq ($(BUILD_DIR), $(shell pwd))
error "BUILD_DIR MUST NOT BE THE SOURCE DIRECTORY"
endif

.PHONY : all build_tree

all clean distclean: build_tree
	make -C $(BUILD_DIR) $@

%config: build_tree
	make -C $(BUILD_DIR) $@

build_tree: FORCE
	$(SCRIPTS)/copy_build_tree $(SRC_DIR) $(BUILD_DIR)

FORCE:
