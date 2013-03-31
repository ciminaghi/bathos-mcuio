/*
 * Trivial, generic, setup function, running initcalls
 * Architectures will pick this if they don't offer their own.
 */
#include <bathos/bathos.h>
#include <bathos/init.h>

int do_initcalls(void)
{
	initcall_t *p;
	int error;

	for (p = initcall_begin; p < initcall_end; p++)
		if ( (error = (*p)()) )
			printf("initcall at %p: error %i\n", p, error);
	return 0;
}

int bathos_setup(void) __attribute__((weak, alias("do_initcalls")));
