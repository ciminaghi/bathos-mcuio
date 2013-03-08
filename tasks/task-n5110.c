#include <bathos/bathos.h>
#include <bathos/spi.h>
#include <arch/gpio.h>

#define GPIODC 12
#define GPIOCS 11

#define GPIOCS_NO0 10
#define GPIOCS_NO1  9

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



static int lcd_cmd(struct spi_dev *dev, int cmd)
{
	uint8_t buf[1] = {cmd};
	struct spi_obuf sob = {.len = 1, .buf = buf};
	int ret;

	gpio_set(GPIODC, 0);
	ret = spi_write(dev, SPI_F_DEFAULT, &sob);
	return ret;
}

static int lcd_data(struct spi_dev *dev, int cmd)
{
	uint8_t buf[1] = {cmd};
	struct spi_obuf sob = {.len = 1, .buf = buf};
	int ret;

	gpio_set(GPIODC, 1);
	ret = spi_write(dev, SPI_F_DEFAULT, &sob);
	return ret;
}

static int lcd_task_init(void *arg)
{
	struct spi_dev *dev = arg;

	printf("%s: %i\n", __func__, __LINE__);
	/* My board other devices on SPI: disable their CS */
	gpio_dir_af(GPIOCS_NO0, 1, 1, 0);
	gpio_set(GPIOCS_NO0, 1);
	gpio_dir_af(GPIOCS_NO1, 1, 1, 0);
	gpio_set(GPIOCS_NO1, 1);
	/* This is our gpio and the reset */
	gpio_dir_af(GPIOCS, 1, 1, 0);
	gpio_set(GPIOCS, 1);
	gpio_dir_af(13, 1, 1, 0);
	gpio_set(13, 1);

	/* d/c */
	gpio_dir_af(GPIODC, 1, 1, 0);
	gpio_set(GPIODC, 1);

	spi_create(&spi_dev);

	lcd_cmd(dev, 0x21);
	lcd_cmd(dev, 0x90);
	lcd_cmd(dev, 0x20);
	lcd_cmd(dev, 0x0c);

	return 0;
}

/* The task sends out one packet and reads pending ones */
static void *lcd_task_run(void *arg)
{
	struct spi_dev *dev = arg;
	static u8 inpacket[50];
	int i, j;

	printf("%s: %i\n", __func__, __LINE__);
	printf("cmd: %i\n", i);

	for (j = 0; j < 0x20; j++)
		i = lcd_data(dev, j);

	return dev;
}

static struct bathos_task __task t_lcd = {
	.name = "n5110", .period = HZ,
	.arg = &spi_dev,
	.init = lcd_task_init, .job = lcd_task_run,
	.release = 15
};
