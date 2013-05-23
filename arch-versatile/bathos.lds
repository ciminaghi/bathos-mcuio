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
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss);
		. = ALIGN(16);
		__bss_end = .;
	}
}
