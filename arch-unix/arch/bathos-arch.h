#ifndef __POSIX_ARCH_H__
#define __POSIX_ARCH_H__

/* This "jiffies" is fake. We use the monotinic time instead, in a lib file */
extern unsigned long get_jiffies(void);

#define __get_jiffies  get_jiffies

#define PROGMEM

#endif /* __POSIX_ARCH_H__ */
