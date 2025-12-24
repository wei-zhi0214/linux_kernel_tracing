#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEV "/dev/wq_echo"

int main(int argc, char **argv)
{
	const char *msg = (argc >= 2) ? argv[1] : "hello workqueue";

	int fd = open(DEV, O_WRONLY);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	ssize_t w = write(fd, msg, strlen(msg));
	if (w < 0) {
		printf("write failed: %s\n", strerror(errno));
		close(fd);
		return 1;
	}

	printf("wrote %zd bytes: \"%s\"\n", w, msg);
	close(fd);
	return 0;
}

