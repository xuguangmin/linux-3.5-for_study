#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define MAX_DEVS (10)
#define XGM_CLASS_NAME   "xgm_class_name"


static dev_t devno;
static dev_t dev_baseid = 250;
static struct class *xgm_class  = NULL;
//static dev_t data;
static int s32_dev_num = 0;

struct file_operations xgm_fops = {
	.owner = THIS_MODULE,
};

static int __devinit xgm_platform_test_probe(struct platform_device *pdev){
	struct device *cls_dev;
	dev_t minor_id = pdev->id;
	
	devno = MKDEV(MAJOR(dev_baseid), MINOR(minor_id));
	pdev->dev.devt = devno;
	
#if 0
	pcdev = cdev_alloc();
	cdev_init(pcdev, &xgm_fops);
	cdev_add(pcdev, devno, 1);
	platform_set_drvdata(pdev, pcdev);
#endif

	cls_dev = device_create(xgm_class, NULL, devno, NULL, "xgm_platform_hello_name%d", minor_id);
	if(IS_ERR(cls_dev))
		printk("xgm platform device create error !\n");
	
	printk("hello probe  xgm_platform_hello_name%d \n", pdev->id);
	s32_dev_num++;
	return 0;
}

static int __devexit xgm_platform_test_remove(struct platform_device *pdev){	
	device_destroy(xgm_class, pdev->dev.devt);
#if 0
	pcdev = platform_get_drvdata(pdev);
	cdev_del(pcdev);
#endif
	s32_dev_num--;
	printk("hello remove  xgm_platform_hello_name%d  %d %d\n", pdev->id, MAJOR(pdev->dev.devt), MINOR(pdev->dev.devt));

	return 0;
}

static struct platform_driver xgm_platform_test = {
	.probe = xgm_platform_test_probe,
	.remove = xgm_platform_test_remove,
	.driver = {
		.name = "xgm_platform",
		.owner =THIS_MODULE, 
	},
};

static int __init xgm_platform_helloworld_init(void)
{
	int ret;
	printk("Helllo, Platform insmod !\n");
	
	ret = alloc_chrdev_region(&dev_baseid, 0, MAX_DEVS, XGM_CLASS_NAME);
	if (ret)
		printk("alloc_chrdev_region error !\n");

	xgm_class = class_create(THIS_MODULE, XGM_CLASS_NAME);
	if(IS_ERR(xgm_class))
		printk("class_create error !\n");

	ret = platform_driver_register(&xgm_platform_test);
	if(ret != 0)
		printk("xgm platform init error !\n");
	
	
	return ret;
}

static void __exit xgm_platform_helloworld_exit(void)
{
	platform_driver_unregister(&xgm_platform_test);
	class_destroy(xgm_class);
	//unregister_chrdev_region(0, s32_dev_num);
		
	printk("Good-bye, Platform removed !\n");
}

module_init(xgm_platform_helloworld_init);
module_exit(xgm_platform_helloworld_exit);

MODULE_LICENSE("GPL");
