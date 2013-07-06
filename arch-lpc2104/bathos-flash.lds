OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry_rom)

regs = 0;
jiffies = 0xe0004008;

SECTIONS
{
	. = 0x0;
        .text.boot : {
                    *(.vectors);
                    *(.text.romboot);
                    *(.text.anyboot);
        }
	.text : {
		*(.text)
	}
	.rodata : {
		*(.rodata)
		romcall_begin = .;
		*(.init0);
		romcall_end = .;
	}
	. = ALIGN(16);
	_erom = .;

	.data 0x40000400: AT(_erom) {
		_sdata = .;
		*(.data);
		_edata = .;
	}

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss);
		. = ALIGN(16);
		__bss_end = .;
	}
	/DISCARD/ : { *(.text.ramboot); }
}
