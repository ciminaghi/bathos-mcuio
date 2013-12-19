SECTIONS
{
	.rodata : {
		*(.rodata);
		. = ALIGN(8);
		romcall_begin = .;
		romcall_end = .;
		. += 8;
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
		. = ALIGN(64);
		evt_sys_timer_tick_handlers_start = .;
		*(.evt_sys_timer_tick_handlers);
		evt_sys_timer_tick_handlers_end = .;
		evt_input_pipe_opened_handlers_start = .;
		*(.evt_input_pipe_opened_handlers);
		evt_input_pipe_opened_handlers_end = .;
		evt_output_pipe_opened_handlers_start = .;
		*(.evt_output_pipe_opened_handlers);
		evt_output_pipe_opened_handlers_end = .;
		evt_pipe_output_ready_handlers_start = .;
		*(.evt_pipe_output_ready_handlers);
		evt_pipe_output_ready_handlers_end = .;
		evt_pipe_input_ready_handlers_start = .;
		*(.evt_pipe_input_ready_handlers);
		evt_pipe_input_ready_handlers_end = .;
		bathos_devices_start = .;
		*(.bathos_devices);
		bathos_devices_end = .;
	}
}

PROVIDE(printf = pp_printf);
PROVIDE(sprintf = pp_sprintf);
PROVIDE(vprintf = pp_sprintf);
PROVIDE(vsprintf = pp_vsprintf);
