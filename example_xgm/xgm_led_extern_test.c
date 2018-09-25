#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/types.h>


#if 1
#define XGM_IOCTL_BASE	'I'
#define XGM_LED_ON	    _IOW(XGM_IOCTL_BASE, 1, int)
#define XGM_LED_OFF 	_IOW(XGM_IOCTL_BASE, 2, int)
#define XGM_LED_STATE	_IOR(XGM_IOCTL_BASE, 3, int)
#endif


#define LED_DEV_NAME	"/dev/xgm_led"
int main()
{
	int ret, state = 0;
	int fd;
	unsigned flag = XGM_LED_ON;

	fd = open(LED_DEV_NAME, O_RDWR);
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
		ret = ioctl(fd, XGM_LED_STATE, &state);
		if(state == 0)
			printf("LED ON\n");
		else if(state == 1)
			printf("LED OFF\n");

		if(flag == XGM_LED_ON) flag = XGM_LED_OFF;
		else if(flag == XGM_LED_OFF) flag = XGM_LED_ON;

		sleep(3);

	}

	
	return 0;
}



