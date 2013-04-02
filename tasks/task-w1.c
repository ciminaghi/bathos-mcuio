#include <bathos/bathos.h>
#include <bathos/w1.h>
#include <arch/gpio.h>
#include <arch/hw.h>

static int w1_init(void *arg)
{
	struct w1_bus *bus = arg;
	struct w1_dev *dev;
	int i, ndev;

	ndev = w1_scan_bus(bus);
	printf("%s: scan result: %i devices\n", __func__, ndev);
	for (i = 0; i < ndev; i++) {
		dev = bus->devs + i;
		printf("  %08x%08x\n", (int)(dev->rom >> 32), (int)(dev->rom));
	}
	return 0;
}

static void *w1_job(void *arg)
{
	struct w1_bus *bus = arg;
	static int nrun;
	int i, sign, offset;
	uint32_t t;
	unsigned char buf[8];

	nrun++;

	if (nrun & 1) {
		/* odd: start reading temperature */
		w1_read_temp_bus(bus, W1_FLAG_NOWAIT);
		return arg;
	}

	/* even: collect tempertarure or error */
	t = w1_read_temp_bus(bus, W1_FLAG_COLLECT);
	if (t != 0x8000 << 16) {
		sign = t & 0x80000000;
		if (sign)
			t = -(int32_t)t;
		printf("temp 0x%08x: %s%i.%04i\n", (unsigned int)t,
		       sign ? "-" : "",
		       (int)(t >> 16),
		       (int)((t & 0xffff) * 10 * 1000 >> 16));
	}

	/* read 8 bytes from an eeprom, if any */
	offset = (nrun -2) * 4; /* we only run at even times, starting from 2 */
	i = w1_read_eeprom_bus(bus, offset, buf, 8);
	if (i == 8) {
		printf("eeprom data @ 0x%08x: ", offset);
		for (i = 0; i < 8; i++)
			printf("%02x%c", buf[i], i == 7 ? '\n' : ' ');
	}
	return arg;
}

static struct w1_bus b = {
	.detail = GPIO_NR(0, 8),
};

static struct bathos_task __task t_w1 = {
	.name = "w1", .period = 2 * HZ,
	.init = w1_init,
	.job = w1_job,
	.arg = &b,
};
