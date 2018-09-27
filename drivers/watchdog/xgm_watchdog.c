#include <linux/module.h>
#include <linux/types.h>

#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/watchdog.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <plat/regs-watchdog.h>
#include <plat/map-base.h>

#undef S3C_VA_WATCHDOG
#define S3C_VA_WATCHDOG (0)
#define CONFIG_S3C2410_WATCHDOG_DEFAULT_TIME	(8)

#define XGM_WATCHDOG_DEV	"s3c2410-wdt"
//#define XGM_WATCHDOG_DEV	"xgm_dev_watchdog"
static bool nowayout	= WATCHDOG_NOWAYOUT;

static struct resource * xgm_wdt_mem = NULL;
static struct resource * xgm_wdt_irq = NULL;
static void __iomem *wdt_base = NULL;
static struct clk	*wdt_clock = NULL;
static int wdt_count = CONFIG_S3C2410_WATCHDOG_DEFAULT_TIME;

static int s3c2410wdt_set_heartbeat(struct watchdog_device *wdd, unsigned timeout);
static int xgm_start(struct watchdog_device *wdd)
{
	unsigned long wtcon;
	//stop
#if 0
	wtcon = readl((unsigned int)wdt_base + S3C2410_WTCON);
	wtcon &= ~(S3C2410_WTCON_ENABLE | S3C2410_WTCON_RSTEN);
	writel(wtcon, (unsigned int)wdt_base + S3C2410_WTCON);
#endif

	wtcon = readl((unsigned int)wdt_base + S3C2410_WTCON);
	wtcon |= S3C2410_WTCON_ENABLE | S3C2410_WTCON_DIV128;	

	wtcon &= ~S3C2410_WTCON_INTEN;
	wtcon |= S3C2410_WTCON_RSTEN;

	writel(wdt_count, (unsigned int)wdt_base + S3C2410_WTDAT);
	writel(wdt_count, (unsigned int)wdt_base + S3C2410_WTCNT);
	writel(wtcon, (unsigned int)wdt_base + S3C2410_WTCON);
	return 0;
}
static int xgm_stop(struct watchdog_device *wdd)
{
	return 0;
}
static int xgm_ping(struct watchdog_device *wdd)
{
	writel(wdt_count, (unsigned int)wdt_base + S3C2410_WTCNT);
	return 0;
}

#define OPTIONS (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE)
static struct watchdog_info xgm_wdt_info = {
	.options = OPTIONS,
	.firmware_version =  0,
	.identity = "Watchdog",
};
static struct watchdog_ops xgm_wdt_ops = {
	.owner = THIS_MODULE,
	.start = xgm_start,
	.stop = xgm_stop,
	.ping = xgm_ping,
};

struct watchdog_device wdd = {
	.info = &xgm_wdt_info,
	.ops = &xgm_wdt_ops,
};

static int xgm_watchdog_probe(struct platform_device *pdev)
{
	int size;
	xgm_wdt_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(xgm_wdt_mem == NULL) {
		printk("no mem for get source\n");
		return ENOENT;
	}

	xgm_wdt_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(xgm_wdt_irq == NULL) {
		printk("no irq for get source\n");
		return ENOENT;
	}
	
	size = resource_size(xgm_wdt_mem);
	if (!request_mem_region(xgm_wdt_mem->start, size, pdev->name)) {
		printk("failed to get memory region\n");
		return -EBUSY;
	}

	wdt_base = ioremap(xgm_wdt_mem->start, size);
	if (wdt_base == NULL) {
		printk("failed to ioremap() region\n");
		return -EINVAL;
	}

	wdt_clock = clk_get(&pdev->dev, "watchdog");
	if (IS_ERR(wdt_clock)) {
		printk("failed to find watchdog clock source\n");
		return PTR_ERR(wdt_clock);
	}

	clk_enable(wdt_clock);

	s3c2410wdt_set_heartbeat(&wdd, wdt_count);
	watchdog_set_nowayout(&wdd, nowayout);

	watchdog_register_device(&wdd);
	xgm_start(&wdd);
	return 0;
}
static int xgm_watchdog_remove(struct platform_device *platform_device)
{
	return 0;
}

struct platform_driver platform_drv = {
	.remove = xgm_watchdog_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name  = XGM_WATCHDOG_DEV,
	},
};

static int __init xgm_watchdog_init(void)
{
	int ret;
	
	ret = platform_driver_probe(&platform_drv, xgm_watchdog_probe);
	if(ret)
		printk("platform_device_register error\n");

	return 0;
}

static void __exit xgm_watchdog_exit(void)
{
	platform_driver_unregister(&platform_drv);
}

module_init(xgm_watchdog_init);
module_exit(xgm_watchdog_exit);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:s3c2410-wdt");



/* -------------- test ----------------------*/
static int s3c2410wdt_set_heartbeat(struct watchdog_device *wdd, unsigned timeout)
{
	unsigned long freq = clk_get_rate(wdt_clock);
	unsigned int count;
	unsigned int divisor = 1;
	unsigned long wtcon;

	if (timeout < 1)
		return -EINVAL;

	freq /= 128;
	count = timeout * freq;


	if (count >= 0x10000) {
		for (divisor = 1; divisor <= 0x100; divisor++) {
			if ((count / divisor) < 0x10000)
				break;
		}

		if ((count / divisor) >= 0x10000) {
			printk( "timeout %d too big\n", timeout);
			return -EINVAL;
		}
	}

	count /= divisor;
	wdt_count = count;

	/* update the pre-scaler */
	wtcon = readl((unsigned int)wdt_base + S3C2410_WTCON);
	wtcon &= ~S3C2410_WTCON_PRESCALE_MASK;
	wtcon |= S3C2410_WTCON_PRESCALE(divisor-1);

	writel(count, (unsigned int)wdt_base + S3C2410_WTDAT);
	writel(wtcon, (unsigned int)wdt_base + S3C2410_WTCON);

	wdd->timeout = (count * divisor) / freq;

	return 0;
}


