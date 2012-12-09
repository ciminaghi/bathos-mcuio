#include <bathos/bathos.h>
#include <arch/hw.h>

int bathos_main(void)
{
	struct bathos_task *p;
	unsigned long now;

	puts("Hello, Bathos is speaking\n");

	for (p = __task_begin; p < __task_end; p++) {
		puts("Task: "); puts(p->name); putc('\n');
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
