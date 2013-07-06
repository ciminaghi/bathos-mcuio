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
	romcall_begin = .;
	romcall_end = .;
	.data : { *(.data) }

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss);
		. = ALIGN(16);
		__bss_end = .;
	}
	/DISCARD/ : { *(.text.romboot); *(.vectors); *(.init0); }
}
