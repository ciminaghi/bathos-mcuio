
#include <bathos/stdio.h>
#include <bathos/event.h>
#include <bathos/pipe.h>
#include <bathos/init.h>
#include <bathos/shell.h>
#include <bathos/allocator.h>

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

static struct shell_cmd *__get_cmd(const struct shell_cmd * PROGMEM _cmd,
				   struct shell_cmd *__cmd)
{
	memcpy_p(__cmd, _cmd, sizeof(*_cmd));
	return __cmd;
}

#else

static struct shell_cmd *__get_cmd(const struct shell_cmd * PROGMEM _cmd,
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
	for (i = 1, argv[0] = buf, ptr = buf; *ptr && *ptr != ';';) {
		if (*ptr++ == ' ' && i < MAX_ARGV) {
			/* zero terminate arg */
			*(ptr - 1) = 0;
			/* Skip multiple spaces */
			for (; *ptr == ' '; ptr++);
			if (*ptr)
				argv[i++] = ptr - 1;
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
		printf("Unknown command %s\n", argv[0]);
		goto end;
	}
	if (cmd->handler(i, argv) < 0)
		printf("shell: error executing cmd %s\n", buf);
end:
	return ptr - buf;
}

static void __shell_exit(struct shell_data *data)
{
	printf("lininoIO shell terminated, bye\n");
	bathos_free_buffer(data, sizeof(*data));
	data = NULL;
	trigger_event(&evt_shell_termination, NULL, EVT_PRIO_MAX);
}

static void __shell_input_handle(struct event_handler_data *ed)
{
	struct shell_data **pdata = ed->priv;
	struct shell_data *data = *pdata;

	int stat, i;
	char *newline = NULL, *cmdstr;

	if (!data)
		return;

	if (data->curr_ptr - data->buf >= sizeof(data->buf)) {
		printf("invalid cmd\n");
		data->curr_ptr = data->buf;
		return;
	}

	stat = pipe_read(bathos_stdin, data->curr_ptr,
			 sizeof(data->buf) - (data->curr_ptr - data->buf));
	if (stat < 0) {
		printf("shell: error reading from stdin\n");
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
	if (!newline)
		return;
	printf("\n");
	/* one or more ; separated cmds have been received, run them */
	for (cmdstr = data->buf; cmdstr[0]; )
		cmdstr += __do_cmd(cmdstr);
	
	/* get ready for next cmd */
	data->curr_ptr = data->buf;
	memset(data->buf, 0, sizeof(data->buf));
	printf(PROMPT);
}

static void __shell_start_handle(struct event_handler_data *ed)
{
	struct shell_data **pdata = ed->priv;
	struct shell_data *data = *pdata;

	if (data)
		printf("WARN: shell_init(): shell_data is not NULL\n");
	*pdata = bathos_alloc_buffer(sizeof(struct shell_data));
	data = *pdata;
	if (!data) {
		printf("ERROR: shell_init(): not enough memory\n");
		return;
	}
	printf("\nlininoIO shell started\n");
	memset(data->buf, 0, sizeof(data->buf));
	data->curr_ptr = data->buf;
	printf(PROMPT);
}

declare_event_handler_with_priv(shell_start, NULL, __shell_start_handle,
				NULL, &shell_data);

static int shell_init()
{
	printf("bathos shell initialized\n");
	return 0;
}

rom_initcall(shell_init);

declare_event_handler_with_priv(shell_input_ready, NULL, __shell_input_handle,
				NULL, &shell_data);

/* a couple of embedded shell commands */

/* help command */
static int help_handler(int argc, char *argv[])
{
	const struct shell_cmd *cmd, * PROGMEM _cmd;
	struct shell_cmd __cmd;

	printf("lininoIO shell, available commands:\n");
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
	printf("print shell help and return\n");
}

declare_shell_cmd(help, help_handler, help_help);


/* version command */

static int version_handler(int argc, char *argv[])
{
	printf("lininoIO git: %s, built %s %s\n", BATHOS_GIT, __DATE__,
		__TIME__);
	return 0;
}

static void version_help(int argc, char *argv[])
{
	printf("prints out lininoIO version and returns\n");
}

declare_shell_cmd(version, version_handler, version_help);


/* license command */

static int license_handler(int argc, char *argv[])
{
	printf("lininoIO, MCU implementation of the mcuio protocol\n");
	printf("See source files for individual Copyright terms\n");
	printf("lininoIO is distributed under the GPL license version 2 or later, see source files for details\n");
	return 0;
}

static void license_help(int argc, char *argv[])
{
	printf("prints out lininoIO licensing terms\n");
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
	printf("terminates the shell\n");
}

declare_shell_cmd(exit, exit_handler, exit_help);

