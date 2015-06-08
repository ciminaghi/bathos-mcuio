OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_bathos_start)

regs = 0;
_jiffies = 0x101e3004;

SECTIONS
{
	/* . = 0x00000400; */
	. = 0x10000;
	.text : {
		*(.boot)
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
	.stack : {
	       . += 0x2000;
	       __stack_end = .;
	}
}
