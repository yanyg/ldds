#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int fd, x, y, len;
	char buf[16];

	fd = open("/sys/devices/platform/vms/coordinates", O_RDWR);
	if (fd < 0) {
		perror("open vms failed");
		exit(1);
	}

	srandom((unsigned int)time(NULL));

	while (1) {
		x = random()%20;
		y = random()%20;

		if (x%2) x = -x;
		if (y%2) y = -y;

		len = sprintf(buf, "%d %d %d", x, y, 0);
		if (len < 0) {
			perror("sprintf failed");
			exit(1);
		}
		printf("len = %d, %zu, %s\n", len, strlen(buf), buf);
		write(fd, buf, len);
		fsync(fd);
		sleep(1);
	}

	close(fd);

	return 0;
}
