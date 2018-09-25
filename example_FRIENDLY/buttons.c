#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
int main(void)
{
	int buttons_fd;
	char buttons[6] = {'0', '0', '0', '0', '0', '0'};
	char current_buttons[6];
	int i;

	buttons_fd = open("/dev/buttons", 0);
	if (buttons_fd < 0) {
		perror("open device buttons");
		exit(1);
	}

	for (;;) {
		if (read(buttons_fd, current_buttons,sizeof(current_buttons)) != sizeof(current_buttons)) {
			perror("read buttons:");
			exit(1);
		}

		for (i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
			if (buttons[i] != current_buttons[i]) {
				buttons[i] = current_buttons[i];
				printf("key %d is %s\n", i+1, buttons[i] == '0' ? "up" : "down");
			}
		}
	}

	close(buttons_fd);
	return 0;
}
