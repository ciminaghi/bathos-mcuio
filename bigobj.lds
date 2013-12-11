SECTIONS
{
	.rodata : {
		*(.rodata);
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
		events_start = .;
		*(.events);
		events_end = .;
	}
}

PROVIDE(printf = pp_printf);
PROVIDE(sprintf = pp_sprintf);
PROVIDE(vprintf = pp_sprintf);
PROVIDE(vsprintf = pp_vsprintf);
