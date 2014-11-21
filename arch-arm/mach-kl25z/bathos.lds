ENTRY(_romboot_start)

regs = 0;

/* Warning: linker script valid for flash boot only */
SECTIONS
{
	.text 0x00000000: {
		*(.vectors);
		. = 0x400;
		*(.flash_config)
		. = 0x410;
		*(.text.romboot)
		*(.text)
	}

	. = ALIGN(16);
	.etext = .;

	.rodata : {
		*(.rodata)
		romcall_begin = .;
		*(.init0);
		romcall_end = .;
	       . = ALIGN(16);
	       _erom = .;
	}

	. = ALIGN(16);
	_erom = .;

	.data 0x1ffff0c0: AT(_erom) {
		_sdata = .;
		*(.data);
		bathos_devices_start =  .;
		*(.bathos_devices);
		bathos_devices_end = .;
		_edata = .;
	}

	.bss : {
	        . = ALIGN(16);
		__bss_start__ = .;
		*(.bss);
		*(COMMON);
		. = ALIGN(16);
		__bss_end__ = .;
	}
}
