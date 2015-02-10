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
		. = ALIGN(8);
		bathos_devices_start = .;
		*(.bathos_devices);
		bathos_devices_end = .;
		. = ALIGN(4);
		__task_begin = .;
		*(.task);
		__task_end = .;
	}
	.text : {
		. = ALIGN(2);
		events_start = .;
		*(.events);
		events_end = .;
		interrupt_events_start = .;
		*(.interrupt_events);
		interrupt_events_end = .;
		cmds_start = .;
		*(.shell_cmds);
		cmds_end = .;
		*(.text.version);
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
		evt_mcuio_irq_handlers_start = .;
		*(.evt_mcuio_irq_handlers);
		evt_mcuio_irq_handlers_end = .;
		evt_gpio_evt_handlers_start = .;
		*(.evt_gpio_evt_handlers);
		evt_gpio_evt_handlers_end = .;
		evt_i2c_transaction_handlers_start = .;
		*(.evt_i2c_transaction_handlers);
		evt_i2c_transaction_handlers_end = .;
		evt_shell_input_ready_handlers_start = .;
		*(.evt_shell_input_ready_handlers);
		evt_shell_input_ready_handlers_end = .;
		evt_shell_start_handlers_start = .;
		*(.evt_shell_start_handlers);
		evt_shell_start_handlers_end = .;
		evt_shell_termination_handlers_start = .;
		*(.evt_shell_termination_handlers);
		evt_shell_termination_handlers_end = .;
		mcuio_functions_start = .;
		*(.mcuio_functions);
		mcuio_functions_end = .;
	}
}

PROVIDE(__printf = pp_printf);
PROVIDE(__sprintf = pp_sprintf);
PROVIDE(__vprintf = pp_sprintf);
PROVIDE(__vsprintf = pp_vsprintf);
