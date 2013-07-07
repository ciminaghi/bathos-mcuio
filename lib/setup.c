/*
 * Trivial, generic, setup function, running initcalls
 * Architectures will pick this if they don't offer their own.
 */
#include <bathos/bathos.h>
#include <bathos/init.h>

static int do_one_initcall(initcall_t *p)
{
	int error = (*p)();

	if (error)
		printf("initcall at %p: error %i\n", p, error);
	return error;
}

int do_initcalls(void)
{
	initcall_t *p;

	/* This list is empty for RAM boot */
	for (p = romcall_begin; p < romcall_end; p++)
		do_one_initcall(p);

	for (p = initcall_begin; p < initcall_end; p++)
		do_one_initcall(p);
	return 0;
}

int bathos_setup(void) __attribute__((weak, alias("do_initcalls")));
