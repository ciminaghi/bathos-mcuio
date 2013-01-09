/*
 * Main header for Bathos
 *  Alessandro Rubini, 2009-2012 GNU GPL2 or later
 */
#ifndef __BATHOS_H__
#define __BATHOS_H__

/*
 * stdio-related stuff is related to bathos/stdio.h, but users are not
 * expected to include it (because it sounds very ugly)
 */
#include <bathos/stdio.h>

/* Ever-needed definitions */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})


/* Other misc bathos stuff */
extern int bathos_main(void);
extern int bathos_setup(void);

/* And finally the task definition */
struct bathos_task {
	char *name;
	void *(*job)(void *);
	int (*init)(void *);
	void *arg;
	unsigned long period;
	unsigned long release;
};

#define __task __attribute__((section(".task"),__used__))

extern struct bathos_task __task_begin[], __task_end[];

#endif /* __BATHOS_H__ */
