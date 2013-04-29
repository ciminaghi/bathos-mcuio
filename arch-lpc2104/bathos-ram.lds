OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry_ram)

regs = 0;
jiffies = 0xe0004008;

SECTIONS
{
	. = 0x40000400;
	 .text.boot : {
		*(.text.ramboot);
		*(.text.anyboot);
	 }
	.text : {
		*(.text)
	}
	.rodata : {
		*(.rodata)
		. = ALIGN(4);
		initcall_begin = .;
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

	.data : {
		*(.data);
		. = ALIGN(16);
		__task_begin = .;
		*(.task);
		__task_end = .;
	}

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss);
		. = ALIGN(16);
		__bss_end = .;
	}
	/DISCARD/ : { *(.text.romboot); *(.vectors); *(.init0); }
}

PROVIDE(printf = pp_printf);
PROVIDE(sprintf = pp_sprintf);
PROVIDE(vprintf = pp_sprintf);
PROVIDE(vsprintf = pp_vsprintf);
