// 因为没有设备树中的结点供该moudle注册，因此需要手动向内核注册一个设备
// because that there is no device tree node to auto register this driver,
// we should add a device to kernel
#include <linux/platform_device.h>
#include <linux/module.h>

#define MY_DEVICE_NAME "hellowd_dev"
static char *my_device_name = MY_DEVICE_NAME;
static struct platform_device *my_pdev;

static int __init hellowd_dev_init(void)
{
    printk("hellowd platform device init\n");
    int ret = 0;
    struct platform_device *pdev = NULL;

    pdev = platform_device_alloc(my_device_name, -1);
    if (!pdev)
    {
        printk(KERN_ERR "failed to alloc platform device : %s \n", my_device_name);
    }
    my_pdev = pdev;

    ret = platform_device_add(pdev);
    if (ret)
    {
        printk(KERN_ERR "failed to add platform device  : %s \n", my_device_name);
        platform_device_put(pdev);
        return ret;
    }

    printk(KERN_INFO "device init %s : success \n", my_device_name);
    return 0;
}

static void __exit hellowd_dev_exit(void)
{
    printk(KERN_INFO "device exit %s \n", my_device_name);
    platform_device_unregister(my_pdev);
}

module_init(hellowd_dev_init);
module_exit(hellowd_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PolarisZg");
MODULE_DESCRIPTION("hello world platform dev");