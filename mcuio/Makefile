# MCUIO Makefile

obj-$(CONFIG_TASK_MCUIO) += mcuio/task-mcuio.o
obj-$(CONFIG_MCUIO_ZERO) += mcuio/mcuio_zero_func.o
obj-$(CONFIG_MCUIO_GPIO) += mcuio/mcuio_gpio_func.o
obj-$(CONFIG_MCUIO_ADC)  += mcuio/mcuio_adc_func.o
obj-$(CONFIG_MCUIO_PWM)  += mcuio/mcuio_pwm_func.o
obj-$(CONFIG_MCUIO_IRQ_CONTROLLER_MSG) += mcuio/mcuio_irq_controller_msg.o
obj-$(CONFIG_MCUIO_SHIELD) += mcuio/mcuio_shield_func.o
obj-$(CONFIG_MCUIO_BITBANG_I2C) += mcuio/mcuio_bitbang_i2c_func.o
obj-$(CONFIG_MCUIO_IRQ_TEST) += mcuio/mcuio_irq_test_func.o


ifeq ($(CONFIG_MCUIO_GPIO),y)
  # Find out name for gpio config file
  # For generic boards (any), pick a cfg file based on package variant
  __PACKAGE=$(shell echo $(BOARD) | grep "any" && echo $(PACKAGE)-)
  MCUIO_GPIO_CONFIG_FILE=$(ARCH)-$(BOARD)-$(__PACKAGE)gpios.cfg
  GPIOS_NAMES_FILE = $(patsubst %.cfg, mcuio/%-names.o,\
	$(MCUIO_GPIO_CONFIG_FILE))
  GPIOS_CAPS_FILE = $(patsubst %.cfg, mcuio/%-caps.o, $(MCUIO_GPIO_CONFIG_FILE))
  obj-y += $(GPIOS_NAMES_FILE) $(GPIOS_CAPS_FILE)
  MCUIO_TOT_NGPIO = $(shell $(SCRIPTS)/get_ngpios mcuio/configs/$(MCUIO_GPIO_CONFIG_FILE))
  MCUIO_GPIO_NPORTS = $(shell $(SCRIPTS)/get_ngpio_ports $(MCUIO_TOT_NGPIO) 64)
  MCUIO_TABLES_OBJS = $(foreach i, \
			 $(shell seq 0 $$(($(MCUIO_GPIO_NPORTS) - 1))),\
			 mcuio/mcuio_gpio_table_$i.o)
  obj-y += $(MCUIO_TABLES_OBJS)
endif

$(GPIOS_NAMES_FILE): mcuio/%-names.o: mcuio/configs/%.cfg main.o
	for p in $$(seq 0 $$(($(MCUIO_GPIO_NPORTS) - 1))) ; do \
		offs=$$(($$p * 64)) \
		ngpios=$$($(SCRIPTS)/get_port_ngpios $$p 64 $(MCUIO_TOT_NGPIO)); \
		$(SCRIPTS)/gen_gpios_names $$($(SCRIPTS)/get_bin_format) $< \
		$$p $$offs $$ngpios $(basename $@)_$$p.o ; \
	done
	$(LD) -r $(foreach p,$(shell seq 0 $$(($(MCUIO_GPIO_NPORTS)-1))), \
		$(basename $@)_$(p).o) -o $@
	if [ "$(ARCH)" = "arm" ] ; then $(SCRIPTS)/arm_fix_elf $@ main.o ; fi

$(GPIOS_CAPS_FILE): mcuio/%-caps.o: mcuio/configs/%.cfg main.o
	for p in $$(seq 0 $$(($(MCUIO_GPIO_NPORTS) - 1))) ; do \
		offs=$$(($$p * 64)) \
		ngpios=$$($(SCRIPTS)/get_port_ngpios $$p 64 $(MCUIO_TOT_NGPIO)); \
		$(SCRIPTS)/gen_gpios_capabilities $$($(SCRIPTS)/get_bin_format) $< \
		$$p $$offs $$ngpios $(basename $@)_$$p.o ; \
	done
	$(LD) -r $(foreach p,$(shell seq 0 $$(($(MCUIO_GPIO_NPORTS)-1))), \
		$(basename $@)_$(p).o) -o $@
	if [ "$(ARCH)" = "arm" ] ; then $(SCRIPTS)/arm_fix_elf $@ main.o ; fi



# This target is needed to generate a default version of the gpio config file
# Will be removed when all boards have their gpio config file.
mcuio/$(MCUIO_GPIO_CONFIG_FILE):
	$(SCRIPTS)/gen_default_gpio_config_file $(ARCH) $(BOARD) $@

$(MCUIO_TABLES_OBJS): mcuio/mcuio_gpio_table_%.o : mcuio/mcuio_gpio_table.c mcuio/configs/$(MCUIO_GPIO_CONFIG_FILE)
	$(CC) $(CFLAGS) -DMCUIO_GPIO_PORT=$* \
	-DMCUIO_NGPIO=$$($(SCRIPTS)/get_port_ngpios $* 64 $(MCUIO_TOT_NGPIO)) \
	-c -o $@ $<
