SECTIONS
{
	.rodata : {
		*(.rodata);
		*(.rodata.str1.1);
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
		. = ALIGN(2);
		events_start = .;
		*(.events);
		events_end = .;
		. = ALIGN(64);
		evt_sys_timer_tick_handlers_start = .;
		*(.evt_sys_timer_tick_handlers);
		evt_sys_timer_tick_handlers_end = .;
		evt_hw_timer_tick_handlers_start = .;
		*(.evt_hw_timer_tick_handlers);
		evt_hw_timer_tick_handlers_end = .;
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
		evt_mcuio_function_request_handlers_start = .;
		*(.evt_mcuio_function_request_handlers);
		evt_mcuio_function_request_handlers_end = .;
		evt_input_pipe_closed_handlers_start = .;
		*(.evt_input_pipe_closed_handlers);
		evt_input_pipe_closed_handlers_end = .;
		evt_output_pipe_closed_handlers_start = .;
		*(.evt_output_pipe_closed_handlers);
		evt_output_pipe_closed_handlers_end = .;
		evt_pipe_really_closed_handlers_start = .;
		*(.evt_pipe_really_closed_handlers);
		evt_pipe_really_closed_handlers_end = .;
		evt_mcuio_data_ready_handlers_start = .;
		*(.evt_mcuio_data_ready_handlers);
		evt_mcuio_data_ready_handlers_end = .;
		mcuio_functions_start = .;
		*(.mcuio_functions);
		mcuio_functions_end = .;
	}
}

PROVIDE(printf = pp_printf);
PROVIDE(sprintf = pp_sprintf);
PROVIDE(vprintf = pp_sprintf);
PROVIDE(vsprintf = pp_vsprintf);
