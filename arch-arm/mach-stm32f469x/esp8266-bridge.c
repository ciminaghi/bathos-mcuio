#include <bathos/bathos.h>
#include <bathos/pipe.h>
#include <bathos/string.h>
#include <bathos/errno.h>
#include <tasks/mcuio.h>
#include <bathos/jiffies.h>
#include <bathos/stdio.h>
#include <generated/autoconf.h>

struct esp8266_bridge_data {
	/* Host connection */
	struct bathos_pipe *upstream_pipe;
	/* ESP8266 connection */
	struct bathos_pipe *downstream_pipe;
};

static struct esp8266_bridge_data global_data;

declare_event(upstream_input_ready);
declare_event(downstream_input_ready);

#define HELLO_STR "esp8266 bridge ready\n"

static int esp8266_bridge_init(void *arg)
{
	struct esp8266_bridge_data *data = arg;
	data->upstream_pipe = pipe_open("uart6",
					BATHOS_MODE_INPUT_OUTPUT,
					data);
	if (!data->upstream_pipe) {
		printf("esp8266-bridge: upstream pipe err\n");
		return -1;
	}
	pipe_remap_input_ready_event(data->upstream_pipe,
				     &evt_upstream_input_ready);
	data->downstream_pipe = pipe_open("uart5",
					  BATHOS_MODE_INPUT_OUTPUT,
					  data);
	if (!data->downstream_pipe) {
		printf("esp8266-bridge: downstream pipe err\n");
		return -1;
	}
	pipe_remap_input_ready_event(data->downstream_pipe,
				     &evt_downstream_input_ready);
	pipe_write(data->upstream_pipe, HELLO_STR, strlen(HELLO_STR));
	return 0;
}

static void upstream_input_ready_handle(struct event_handler_data *ed)
{
	char c;
	int stat;
	static struct esp8266_bridge_data *data = &global_data;

	stat = pipe_read(data->upstream_pipe, &c, 1);
	if (stat < 0) {
	    //printf("%s: error reading from upstream pipe\n", __func__);
		return;
	}
	stat = pipe_write(data->downstream_pipe, &c, 1);
	if (stat < 0)
	    printf("%s: error writing to downstream pipe\n",
		   __func__);
}
declare_event_handler(upstream_input_ready, NULL, upstream_input_ready_handle,
		      NULL);

static void downstream_input_ready_handle(struct event_handler_data *ed)
{
	char c;
	int stat;
	static struct esp8266_bridge_data *data = &global_data;

	stat = pipe_read(data->downstream_pipe, &c, 1);
	if (stat < 0) {
		printf("%s: error %d reading from downstream pipe\n", __func__,
		       stat);
		return;
	}
	stat = pipe_write(data->upstream_pipe, &c, 1);
	if (stat < 0)
	    printf("%s: error writing to upstream pipe\n", __func__);
}
declare_event_handler(downstream_input_ready, NULL,
		      downstream_input_ready_handle, NULL);


static void *esp8266_bridge_alive(void *arg)
{
	return arg;
}

static struct bathos_task __task t_mcuio = {
	.name = "esp8266-bridge", .period = 60 * HZ,
	.job = esp8266_bridge_alive, .arg = &global_data,
	.init = esp8266_bridge_init,
	.release = 3,
};
