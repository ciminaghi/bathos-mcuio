
extern int bathos_main(void);
extern int bathos_setup(void);

extern void putc(int c);
extern int puts(const char *s);

extern volatile unsigned long jiffies;

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
