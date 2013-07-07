
#include <bathos/bathos.h>
#include <arch/hw.h>
#include <arch/bathos-arch.h>

#include <time.h>
#include <errno.h>

unsigned long get_jiffies(void)
{
	/* Lazily, let's waste processing power */
	unsigned long long totalns;
	static unsigned long jiffies;
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		printf("Error in clock_gettime(): %i\n", -errno);
		return jiffies;
	}
	totalns = ts.tv_sec;
	totalns = totalns * 1000 * 1000 * 1000 + ts.tv_nsec;
	jiffies = totalns / (1000 * 1000 * 1000 / HZ);
	return jiffies;
}
