OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry_rom)

regs = 0;
jiffies = 0x40018008;

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
		. = ALIGN(4);
		initcall_begin = .;
		*(.init0);
		*(.init1);
		*(.init2);
		*(.init3);
		*(.init4);
		*(.init5);
		*(.init6);
		*(.init7);
		*(.init8);
		initcall_end = .;
	}
	. = ALIGN(16);
	_erom = .;

	.data 0x10000400: AT(_erom) {
		_sdata = .;
		*(.data);

		. = ALIGN(16);
		__task_begin = .;
		*(.task);
		__task_end = .;
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

PROVIDE(printf = pp_printf);
PROVIDE(sprintf = pp_sprintf);
PROVIDE(vprintf = pp_sprintf);
PROVIDE(vsprintf = pp_vsprintf);
