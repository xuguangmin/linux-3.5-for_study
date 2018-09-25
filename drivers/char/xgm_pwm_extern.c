#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>


#include <mach/gpio.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>

#define MAX_DEVS (1)
#define XGM_CLASS_NAME   "xgm_pwm_class"
#define XGM_DEV_NAME     "xgm_pwm"
	
static int pwm_gpios[] = {
	EXYNOS4_GPD0(0),
	EXYNOS4_GPD0(1),
};
#define PWM_NUM		ARRAY_SIZE(pwm_gpios)

static dev_t devno;
static dev_t dev_baseid = 520;
static struct class *xgm_class  = NULL;
//static dev_t data;
static int s32_dev_num = 0;

#define XGM_IOCTL_BASE	'I'
#define XGM_PWM_ON	    _IOW(XGM_IOCTL_BASE, 1, int)
#define XGM_PWM_OFF 	_IOW(XGM_IOCTL_BASE, 2, int)
#define XGM_PWM_STATE	_IOR(XGM_IOCTL_BASE, 3, int)

long xgm_unlocked_ioctl(struct file *pFile, unsigned int u32Cmd, unsigned long u64Arg)
{
	switch(u32Cmd)
	{
		case XGM_PWM_ON:
			gpio_set_value(pwm_gpios[0], GPIOF_INIT_HIGH);
			break;
		case XGM_PWM_OFF:
			gpio_set_value(pwm_gpios[0], GPIOF_INIT_LOW);
			break;
		case XGM_PWM_STATE:
			  *(int *)u64Arg = gpio_get_value(pwm_gpios[0]);
			  break;
		default:
			printk("xgm_led: default error\n");
			return -1;
			break;
	}
	return 0;
}

struct file_operations xgm_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = xgm_unlocked_ioctl,
};

static int __devinit xgm_platform_test_probe(struct platform_device *pdev){
	struct device *cls_dev;
	struct cdev * pcdev;
	dev_t minor_id = pdev->id;
	int i, j, ret;

	for(i = 0; i < PWM_NUM; i++) {
		ret = gpio_request(pwm_gpios[i], "xgm-pwm");
		if (ret)
			printk("request gpio error %d\n", i);
		for(j = 0; j < 3; j++){
			gpio_direction_output(pwm_gpios[i], GPIOF_INIT_HIGH);
			msleep(70);
			gpio_direction_output(pwm_gpios[i], GPIOF_INIT_LOW);
		}
		//gpio_set_value(led_gpios[i], S3C_GPIO_OUTPUT);
	}
	
	devno = MKDEV(MAJOR(dev_baseid), MINOR(minor_id));
	pdev->dev.devt = devno;
	
	pcdev = cdev_alloc();
	cdev_init(pcdev, &xgm_fops);
	cdev_add(pcdev, devno, 1);
	platform_set_drvdata(pdev, pcdev);

	cls_dev = device_create(xgm_class, NULL, devno, NULL, "%s", XGM_DEV_NAME);
	if(IS_ERR(cls_dev))
		printk("xgm platform device create error !\n");
	
	printk("hello probe  %s%d \n", XGM_DEV_NAME, pdev->id);
	s32_dev_num++;
	return 0;
}

static int __devexit xgm_platform_test_remove(struct platform_device *pdev){	
	struct cdev * pcdev;
	int i;

	for(i = 0; i < PWM_NUM; i++) {
		gpio_free(pwm_gpios[i]);
		gpio_set_value(pwm_gpios[i], GPIOF_INIT_LOW);
	}
	
	device_destroy(xgm_class, pdev->dev.devt);
#if 1
	pcdev = platform_get_drvdata(pdev);
	cdev_del(pcdev);
#endif
	s32_dev_num--;
	printk("remove  %s%d  %d %d\n", XGM_DEV_NAME, pdev->id, MAJOR(pdev->dev.devt), MINOR(pdev->dev.devt));

	return 0;
}

static struct platform_driver xgm_platform_test = {
	.probe = xgm_platform_test_probe,
	.remove = xgm_platform_test_remove,
	.driver = {
		.name = XGM_DEV_NAME,
		.owner =THIS_MODULE, 
	},
};

static int __init xgm_platform_helloworld_init(void)
{
	int ret;
			
	ret = alloc_chrdev_region(&dev_baseid, 0, MAX_DEVS, XGM_CLASS_NAME);
	if (ret)
		printk("alloc_chrdev_region error !\n");

	xgm_class = class_create(THIS_MODULE, XGM_CLASS_NAME);
	if(IS_ERR(xgm_class))
		printk("class_create error !\n");

	ret = platform_driver_register(&xgm_platform_test);
	if(ret != 0)
		printk("xgm platform init error !\n");
	
	printk("Helllo, Platform insmod !\n");
	return ret;
}



static void __exit xgm_platform_helloworld_exit(void)
{
	platform_driver_unregister(&xgm_platform_test);
	class_destroy(xgm_class);
	unregister_chrdev_region(dev_baseid,MAX_DEVS);
		
	printk("Good-bye, Platform removed !\n");
}

module_init(xgm_platform_helloworld_init);
module_exit(xgm_platform_helloworld_exit);

MODULE_LICENSE("GPL");
