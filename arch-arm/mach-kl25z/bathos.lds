ENTRY(_romboot_start)

regs = 0;
ram_vectors = 0x1ffff000;

/* Warning: linker script valid for flash boot only */
SECTIONS
{
	.text 0x00000000: {
		rom_vectors = .;
		*(.vectors);
		. = 0x400;
		*(.flash_config)
		. = 0x410;
		*(.text.romboot)
		*(.text)
		mcuio_functions_start = .;
		*(.mcuio_functions);
		mcuio_functions_end = .;
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
		_edata = .;
	}

	.bss : {
	        . = ALIGN(16);
		__bss_start__ = .;
		*(.bss);
		*(.bss.*);
		*(COMMON);
		. = ALIGN(16);
		__bss_end__ = .;
	}
}
