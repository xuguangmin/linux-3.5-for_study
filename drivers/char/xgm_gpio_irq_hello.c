#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>


static struct gpio xgm_gpio_btn[] = {
	{EXYNOS4_GPX3(2), 0, "xgm_btn1"},
	{EXYNOS4_GPX3(3), 1, "xgm_btn2"},
	{EXYNOS4_GPX3(4), 2, "xgm_btn3"},
	{EXYNOS4_GPX3(5), 3, "xgm_btn4"},
};

struct timer_list timer[ARRAY_SIZE(xgm_gpio_btn)];
void timer_fn(unsigned long data)
{
	struct gpio *pdev = data;
	printk("key:%d  name:%s\n", (int)pdev->flags+1, pdev->label);
}

static irqreturn_t xgm_irq_handle(int irq, void *dev)
{
	struct gpio *bdata = (struct gpio *)dev;

	mod_timer(&timer[bdata->flags], jiffies + msecs_to_jiffies(40));

	return IRQ_HANDLED;
}
static int __init xgm_gpio_init(void)
{
	int i, ret, irq;
	for(i = 0; i < ARRAY_SIZE(xgm_gpio_btn); i++) {
		setup_timer(&timer[i], timer_fn, (unsigned long)&xgm_gpio_btn[i]);

		irq = gpio_to_irq(xgm_gpio_btn[i].gpio);
		ret = request_irq(irq, xgm_irq_handle, IRQ_TYPE_EDGE_BOTH, xgm_gpio_btn[i].label, (void *)&xgm_gpio_btn[i]);
		if(ret)
			free_irq(irq, &xgm_gpio_btn[i]);

	}
	return 0;
}
static void __exit xgm_gpio_exit(void)
{
	int i;
	for(i = 0; i < ARRAY_SIZE(xgm_gpio_btn); i++) {
		free_irq(gpio_to_irq(xgm_gpio_btn[i].gpio), &xgm_gpio_btn[i]);
	}
}

module_init(xgm_gpio_init);
module_exit(xgm_gpio_exit);

MODULE_LICENSE("GPL");
