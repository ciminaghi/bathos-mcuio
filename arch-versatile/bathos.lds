OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_bathos_start)

regs = 0;
_jiffies = 0x101e3004;

SECTIONS
{
	. = 0x00000400;
	.text : {
		*(.boot)
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
}

PROVIDE(printf = pp_printf);
PROVIDE(sprintf = pp_sprintf);
PROVIDE(vprintf = pp_sprintf);
PROVIDE(vsprintf = pp_vsprintf);
