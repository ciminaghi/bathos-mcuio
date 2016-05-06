
#include <user_interface.h>
#include <c_types.h>
#include <ets_sys.h>
#include <osapi.h>
#include <bathos/bathos.h>
#include <bathos/init.h>
#include <bathos/jiffies.h>
#include <bathos/event.h>

#define BATHOS_QUEUE_LEN 16

static os_event_t bathos_queue[BATHOS_QUEUE_LEN];

static void _bathos_main(os_event_t *events)
{
	while(pending_events())
		handle_events();
}

/*
 * The bathos is just a task in the esp8266's os.
 * In this case, bathos loop is not actually a loop. We just spawn a task
 * here and wake it up for the first time. Then trigger_event() will restart
 * _bathos_main via events_notify();
 */
void bathos_loop(void)
{
	system_os_task(_bathos_main, 0, bathos_queue, BATHOS_QUEUE_LEN);
	system_os_post(0, 0, 0);
}

/*
 * This is called by trigger_event(): we need to wake up _bathos_main() when
 * bathos events are available to be processed
 */
void events_notify(void)
{
	system_os_post(0, 0, 0);
}

void user_init(void)
{
	int i;

	console_early_init();
	/* printf seems to start working after a while ! */
	for (i = 0; i < 11500; i++)
		console_putc('+');
	bathos_setup();
	bathos_main();
}
