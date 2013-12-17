#ifndef __PIPE_H__
#define __PIPE_H__

#include <bathos/event.h>

#define MAX_BATHOS_PIPES 32

#define BATHOS_MODE_INPUT  1
#define BATHOS_MODE_OUTPUT 2
#define BATHOS_MODE_INPUT_OUTPUT (BATHOS_MODE_INPUT|BATHOS_MODE_OUTPUT)

struct bathos_pipe;

struct bathos_ioctl_data {
	int code;
	void *data;
};

struct bathos_dev_ops {
	int (*open)(struct bathos_pipe *);
	int (*read)(struct bathos_pipe *, char *buf, int len);
	int (*write)(struct bathos_pipe *, const char *buf, int len);
	void (*close)(struct bathos_pipe *);
	int (*ioctl)(struct bathos_pipe *, struct bathos_ioctl_data *);
};

struct bathos_dev {
	const char *name;
	struct bathos_dev_ops *ops;
	void *priv;
};

struct bathos_pipe {
	const char *n;
	int mode;
	void *data;
	struct bathos_dev *dev;
};

/*
 * pipe_open()
 * @n : pipe name (can be NULL, in which case a new descriptor is just returned)
 * @data : data passed on to input_ready/output_ready event callback
 */
struct bathos_pipe *pipe_open(const char *n, int mode, void *data);
int pipe_close(struct bathos_pipe *);
int pipe_read(struct bathos_pipe *, char *buf, int len);
int pipe_write(struct bathos_pipe *, const char *buf, int len);
int pipe_ioctl(struct bathos_pipe *, struct bathos_ioctl_data *data);

declare_extern_event(pipe_opened);
declare_extern_event(pipe_input_ready);
declare_extern_event(pipe_output_ready);
declare_extern_event(pipe_error);

#endif /* __PIPE_H__ */
