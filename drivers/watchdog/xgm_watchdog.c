#include <linux/module.h>
#include <linux/platfor_device.h>

struct platform_driver platform_drv = {
	.probe = ;
	.remove = ;
	void (*shutdown)(struct platform_device *);
	int (*suspend)(struct platform_device *, pm_message_t state);
	int (*resume)(struct platform_device *);
	struct device_driver driver;
	const struct platform_device_id *id_table;
};

static void __init xgm_watchdog_init(void)
{
	int ret;
	
	ret = platform_driver_register(&platform_drv);
	if(ret)
		printk("platform_device_register error\n");
}

static void __exit xgm_watchdog_exit(void)
{
	platform_driver_unregister(&platform_drv);
}

module_init();
module_exit();
MODULE_LICENSE("GPL");
