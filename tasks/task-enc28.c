#include <bathos/bathos.h>
#include <bathos/enc28j60.h>
#include <arch/spi.h>
#include <arch/gpio.h>


#define GPIOCS 10

/*
 * SPI configuration: we need to declare both the config and the
 * device since we have no malloc in this crappy OS
 */
static const struct spi_cfg spi_cfg = {
	.gpio_cs = GPIOCS,
	.pol = 0,
	.phase = 0,
	.devn = 0,
};

static struct spi_dev spi_dev = {
	.cfg = &spi_cfg,
};

/* ENC28 configuration */
static const struct enc28_cfg enc28_cfg = {
	/* This is an IP address in my network */
	.ipaddr = {192, 168,16, 201},
	/* And this is in the private range */
	.macaddr = {0x02, 0x33, 0x44, 0x55, 0x66, 0x77},
};

static struct enc28_dev enc28_dev = {
	.spi_dev = &spi_dev,
	.cfg = &enc28_cfg,
};

static int enc28_task_init(void *unused)
{
	/* My board has another device on SPI: disable its CS on gpio 9*/
	gpio_dir_af(9, 1, 1, 0);
	gpio_set(9, 1);
	/* This is our gpio 10 */
	gpio_dir_af(10, 1, 1, 0);
	gpio_set(10, 1);

	enc28_create(&enc28_dev);
	return 0;
}


/* the packet we send out is a simple count of runs */
static unsigned char packet[] = {
	/* hardwired eth header */
	[0] = 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* dest: broadcast */
	[6] = 0x02, 0x33, 0x44, 0x55, 0x66, 0x77,	/* source: us */
	[12] = 0x12, /* was 8 */ 0x00,			/* proto: crap */

	[14] = 0x41, 0x42, 0x43, 0x44, 0x45, 0x46	/* temporary hack */
};

/* The task sends out one packet and reads pending ones */
static void *enc28_task_run(void *arg)
{
	struct enc28_dev *dev = arg;
	static u8 inpacket[50];
	int i;

	i = enc28_send(dev, packet, sizeof(packet));
	i = enc28_recv(dev, inpacket, sizeof(inpacket));

	return dev;
}

static struct bathos_task __task t_enc28 = {
	.name = "enc28j60", .period = HZ,
	.arg = &enc28_dev,
	.init = enc28_task_init, .job = enc28_task_run,
	.release = 15
};
