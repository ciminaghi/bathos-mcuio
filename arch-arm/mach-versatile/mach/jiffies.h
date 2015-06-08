#ifndef __MACH_JIFFIES_H__
#define __MACH_JIFFIES_H__

/* Code from old arch-versatile by Alessandro Rubini */

/* Our counter counts downwards, so trigger the "#ifdef __get_jiffies" */
extern volatile unsigned long _jiffies;

static inline unsigned long get_jiffies(void)
{
	return ~_jiffies;
}

#define __get_jiffies  get_jiffies


#endif /* __MACH_JIFFIES_H__ */
