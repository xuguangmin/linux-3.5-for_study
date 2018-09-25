#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/types.h>

#define XGM_IOCTL_BASE	'I'
#define XGM_PWM_ON	    _IOW(XGM_IOCTL_BASE, 1, int)
#define XGM_PWM_OFF 	_IOW(XGM_IOCTL_BASE, 2, int)
#define XGM_PWM_STATE	_IOR(XGM_IOCTL_BASE, 3, int)

#define PWM_DEV_NAME	"/dev/xgm_pwm"
int main()
{
	int ret, state = 0;
	int fd;
	unsigned flag = XGM_PWM_ON;

	fd = open(PWM_DEV_NAME, O_RDWR);
	if(fd < 0) {
		printf("open device error %d\n", ret);
		perror("open:");
		return -1;
	}

	int i = 1;
	while(1) {
		ret = ioctl(fd, flag, 0);
		if(ret != 0) {
			printf("ioctl error %d\n", ret);
			perror("ioctl:");
			return -1;
		}
		ret = ioctl(fd, XGM_PWM_STATE, &state);
		if(state == 0)
			printf("LED ON\n");
		else if(state == 1)
			printf("LED OFF\n");

		if(flag == XGM_PWM_ON) flag = XGM_PWM_OFF;
		else if(flag == XGM_PWM_OFF) flag = XGM_PWM_ON;

		sleep(1);

	}

	
	return 0;
}



