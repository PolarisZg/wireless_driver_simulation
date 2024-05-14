#include <linux/platform_device.h>
#include <linux/module.h>

#define MY_DEVICE_NAME "hellowd_dev"
static char *my_device_name = MY_DEVICE_NAME;

static int hellowd_dev_probe(struct platform_device *pdev)
{
    printk("hello world platform driver \n");
    return 0;

err_free_dev:

    return 1;
}

static int hellowd_dev_remove(struct platform_device *pdev)
{
    printk("bye world platform driver \n");
    return 0;
}

static struct platform_driver hellowd_dev_driver = {
    .driver = {
        .name = MY_DEVICE_NAME,
        .owner = THIS_MODULE},
    .probe = hellowd_dev_probe,
    .remove = hellowd_dev_remove};

module_platform_driver(hellowd_dev_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yukikaze");
MODULE_DESCRIPTION("hello world platform");