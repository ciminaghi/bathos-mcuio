OUTPUT_FORMAT("elf32-avr")
OUTPUT_ARCH(avr:4)
ENTRY(_bathos_start)

regs = 0;
/* jiffies is maintained in software */

SECTIONS
{
	. = 0; /* mandatory: we can only run in flash */
	.text : {
		*(.vectors)
		*(.boot)
		*(.text.ISR)
		*(.text)
		*(.text.*)
		/* For data that needs to reside in the lower 64k of progmem. */
		*(.progmem.gcc*)
    		*(.progmem*)
	}
	__end_text = .;
	INCLUDE arch-atmega/bootloader.lds
	/* By conventions, RAM is thought to be at address 0x80.0000 */
	.data 0x800100 : AT (__end_text) {
		__data_start = .;      
		*(.rodata);
		. = ALIGN(2);
		romcall_begin = .;
		*(.init0);
		romcall_end = .;
		*(.data);
		__data = .;
	}
	__end_data = .;

	/* We need to copy data, these are the start and end addresses */
	__data_load_start = LOADADDR(.data);
	__data_load_end = __data_load_start + SIZEOF(.data);

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss);
		*(COMMON);
		. = ALIGN(16);
		__bss_end = .;
	}
}
