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
	.rodata : { *(.rodata) }
	.data : { *(.data) }

	. = ALIGN(16);
	__task_begin = .;
	.task : {*(.task) }
	__task_end = .;

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss);
		. = ALIGN(16);
		__bss_end = .;
	}
	/DISCARD/ : { *(.text.romboot); *(.vectors); }
}

PROVIDE(printf = pp_printf);
PROVIDE(sprintf = pp_sprintf);
PROVIDE(vprintf = pp_sprintf);
PROVIDE(vsprintf = pp_vsprintf);
