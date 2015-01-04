/*
 * Copyright (c) dog hunter AG - Zug - CH
 * General Public License version 2 (GPLv2)
 * Author: Davide Ciminaghi <ciminaghi@gnudd.com>
 */

#include <bathos/stdio.h>
#include <bathos/event.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/shell.h>
#include <bathos/allocator.h>
#include <bathos/string.h>

#define MAX_ARGV 5

#define PROMPT "lininoIO> "

extern const struct shell_cmd PROGMEM cmds_start[], cmds_end[];

declare_event(shell_input_ready);
declare_event(shell_termination);
declare_event(shell_start);

/* 32 bytes total to avoid wasting allocated memory */
struct shell_data {
	char *curr_ptr;
	char buf[32 - sizeof(char *)];
};

struct shell_data *shell_data;

#ifdef ARCH_IS_HARVARD

static const struct shell_cmd *__get_cmd(const struct shell_cmd * PROGMEM _cmd,
					 struct shell_cmd *__cmd)
{
	memcpy_p(__cmd, _cmd, sizeof(*_cmd));
	return __cmd;
}

#else

static const struct shell_cmd *__get_cmd(const struct shell_cmd * PROGMEM _cmd,
					 struct shell_cmd *__cmd)
{
	return _cmd;
}

#endif

static int __do_cmd(char *buf)
{
	const struct shell_cmd *cmd, * PROGMEM _cmd;
	struct shell_cmd __cmd;
	char *argv[MAX_ARGV];
	char *ptr;
	int i;

	/* find args */
	for (i = 1, argv[0] = buf, ptr = buf; *ptr && *ptr != ';'; ptr++) {
		if ((*ptr == ' ' || *ptr == '\0') && i < MAX_ARGV) {
			/* zero terminate arg */
			*ptr = 0;
			/* Skip multiple spaces */
			for (ptr++; *ptr == ' '; ptr++);
			if (*ptr)
				argv[i++] = ptr;
		}
	}
	if (*ptr == ';')
		*ptr++ = 0;

	for (_cmd = cmds_start; _cmd != cmds_end; _cmd++) {
		cmd = __get_cmd(_cmd, &__cmd);
		if (!strcmp(argv[0], cmd->str))
			break;
	}
	if (_cmd == cmds_end) {
		printf("unknown cmd %s\n", argv[0]);
		goto end;
	}
	if (cmd->handler(i, argv) < 0)
		printf("error exec cmd %s\n", buf);
end:
	return ptr - buf;
}

static void __shell_exit(struct shell_data *data)
{
	printf("\nbye\n");
	bathos_free_buffer(data, sizeof(*data));
	data = NULL;
	trigger_event(&evt_shell_termination, NULL);
}

static void __shell_input_handle(struct event_handler_data *ed)
{
	struct shell_data **pdata = ed->priv;
	struct shell_data *data = *pdata;

	int stat, i;
	char *newline = NULL, *cmdstr;

	if (!data)
		return;

	stat = pipe_read(bathos_stdin, data->curr_ptr,
			 sizeof(data->buf) - (data->curr_ptr - data->buf));
	if (stat < 0) {
		pr_debug("shell: error reading from stdin\n");
		return;
	}
	/* find newline */
	for (i = 0; i < stat ; i++) {
		if (data->curr_ptr[i] == 0x4) {
			__shell_exit(data);
			return;
		}
		/* echo back chars */
		putc(data->curr_ptr[i]);
		if (data->curr_ptr[i] == '\r') {
			newline = &data->curr_ptr[i];
			data->curr_ptr[i] = 0;
			break;
		}
	}
	data->curr_ptr += stat;
	if (!newline) {
		if (data->curr_ptr - data->buf >= sizeof(data->buf)) {
			printf("\ninvalid cmd\n");
			goto next_cmd;
		}
		return;
	}
	printf("\n");
	/* one or more ; separated cmds have been received, run them */
	for (cmdstr = data->buf; cmdstr[0]; )
		cmdstr += __do_cmd(cmdstr);

next_cmd:
	/* get ready for next cmd */
	data->curr_ptr = data->buf;
	/* Empty pipe (if pending chars are there) and reset buffer */
	while (pipe_read(bathos_stdin, data->curr_ptr,
		sizeof(data->buf)) > 0);
	memset(data->buf, 0, sizeof(data->buf));
	printf(PROMPT);
}

static void __shell_start_handle(struct event_handler_data *ed)
{
	struct shell_data **pdata = ed->priv;
	struct shell_data *data = *pdata;

	*pdata = bathos_alloc_buffer(sizeof(struct shell_data));
	data = *pdata;
	if (!data) {
		pr_debug("ERROR: shell_init(): not enough memory\n");
		return;
	}
	printf("\nshell started\n");
	memset(data->buf, 0, sizeof(data->buf));
	data->curr_ptr = data->buf;
	printf(PROMPT);
}

declare_event_handler_with_priv(shell_start, NULL, __shell_start_handle,
				NULL, &shell_data);

#if !CONFIG_CONSOLE_NULL
static int shell_init()
{
	printf("bathos shell initialized\n");
	return 0;
}

rom_initcall(shell_init);
#endif

declare_event_handler_with_priv(shell_input_ready, NULL, __shell_input_handle,
				NULL, &shell_data);

/* a couple of embedded shell commands */

/* help command */
static int help_handler(int argc, char *argv[])
{
	const struct shell_cmd *cmd, * PROGMEM _cmd;
	struct shell_cmd __cmd;

	for (_cmd = cmds_start; _cmd != cmds_end; _cmd++) {
		cmd = __get_cmd(_cmd, &__cmd);
		printf("%s: ", cmd->str);
		if (cmd->help)
			cmd->help(argc, argv);
	}
	return 0;
}

static void help_help(int argc, char *argv[])
{
	printf("print help\n");
}

declare_shell_cmd(help, help_handler, help_help);


/* version command */

extern const char version_string;

static int version_handler(int argc, char *argv[])
{
	/*
	 * printf is actually a macro, which does not expand into correct C
	 * if used with a pointer as first arg instead of a string literal.
	 * We can't use printf("%s", &version_string) either, because the
	 * second arg must point to ram, not flash on the atmega.
	 */
	__printf(&version_string);
	return 0;
}

static void version_help(int argc, char *argv[])
{
	printf("print version\n");
}

declare_shell_cmd(version, version_handler, version_help);


/* license command */

static int license_handler(int argc, char *argv[])
{
	printf("lininoIO, MCU implementation of the mcuio protocol\n");
	printf("See source files for individual Copyright terms\n");
	printf("lininoIO is distributed under the GPL license version 2 "
		"or later, see source files for details\n");
	return 0;
}

static void license_help(int argc, char *argv[])
{
	printf("print licensing terms\n");
}

declare_shell_cmd(license, license_handler, license_help);

/* exit command */

static int exit_handler(int argc, char *argv[])
{
	__shell_exit(shell_data);
	return 0;
}

static void exit_help(int argc, char *argv[])
{
	printf("terminate\n");
}

declare_shell_cmd(exit, exit_handler, exit_help);
