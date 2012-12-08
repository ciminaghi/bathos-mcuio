#include <bathos/bathos.h>
#include <arch/hw.h>

int bathos_main(void)
{
	struct bathos_task *p;
	unsigned long now;

	pp_printf("Hello, Bathos is speaking (built on %s)\n", __DATE__);

	for (p = __task_begin; p < __task_end; p++) {
		pp_printf("Task: %s\n", p->name);
		if (p->init) p->init(p->arg);
	}

	now = jiffies;
	for (p = __task_begin; p < __task_end; p++)
		p->release += now + 2;

	while (1) {
		struct bathos_task *t;

		for (t = p = __task_begin; p < __task_end; p++)
			if (p->release < t->release)
				t = p;
		while ((signed)(t->release - jiffies) > 0)
			;
		t->arg = t->job(t->arg);
		t->release += t->period;
	}
}
