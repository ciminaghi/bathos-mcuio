#ifndef __ARCH_UNIX_PIPE_H__
#define __ARCH_UNIX_PIPE_H__

struct bathos_pipe;

struct arch_unix_pipe_data {
	int fd;
	struct bathos_pipe *pipe;
};

extern struct bathos_pipe *unix_fd_to_pipe(int fd);

#endif /* __ARCH_UNIX_PIPE_H__ */
