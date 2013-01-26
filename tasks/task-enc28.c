#include <ssw/task.h>
#include <ssw/init.h>
#include <ssw/module.h>
#include <ssw/gpio.h>
#include <ssw/spi.h>
#include <ssw/enc28j60.h>

/* SPI configuration */
#define GPIOCS 10
const struct spi_cfg spi_cfg = {
	.gpio_cs = GPIOCS,
	.pol = 0,
	.phase = 0,
	.devn = 0,
};

/* ENC28 configuration */
const struct enc28_cfg enc28_cfg = {
	.spi_cfg = &spi_cfg,
	/* This is an IP address in my network */
	.ipaddr = {192, 168,16, 201},
	/* And this is in the private range, with today's date */
	.macaddr = {0x02, 0x27, 0x07, 0x20, 0x10, 0x0},
};

/* the packet we send out is a simple count of runs */
static unsigned char packet[] = {
	/* hardwired eth header */
	[0] = 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* dest: broadcast */
	[6] = 0x02, 0x27, 0x07, 0x20, 0x10, 0x0,	/* source: us */
	[12] = 0x12, /* was 8 */ 0x00,			/* proto: IPV4 */
	/* hardwired IP header */

	/* hardwired UDP header */

	/* data placeholder */
	[14] = 0x41, 0x42, 0x43, 0x44, 0x45, 0x46	/* temporary hack */
};

/* The task sends out one packet and reads pending ones */
static void *task_enc28test(void *arg)
{
	struct enc28_dev *dev = arg;
	static char inpacket[500];
	int i;

#if 0
	/* FIXME: temporarily just send out this packet */
	i = enc28_send(dev, packet, sizeof(packet));
	printk("send: %i\n", i);
#else
	i = enc28_recv(dev, inpacket, sizeof(packet));
	printk("recv: %i\n", i);
#endif
	return dev;
}

int __init init_app_enc28(void)
{
	struct enc28_dev *dev;

	/* The board has anothe device on SPI: disable its CS */
	gpio_set(9, 1);
	gpio_dir_af(9, SSW_GPIO_OUT, SSW_GPIO_AF_GPIO);

	dev = enc28_create(&enc28_cfg);
	if (!dev) return -1;

	task_register_p(task_enc28test, dev, 100, 10);
	return 0;
}

task_initcall(init_app_enc28);

module_request(spi_21xxx);
module_request(vsprintf_full);
