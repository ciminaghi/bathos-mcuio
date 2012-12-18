
/* Our counter counts downwards, so trigger the "#ifdef __get_jiffies" */
extern volatile unsigned long _jiffies;

static inline unsigned long get_jiffies(void)
{
	return ~_jiffies;
}

#define __get_jiffies  get_jiffies

