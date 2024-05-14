// 因为没有设备树中的结点供该moudle注册，因此需要手动向内核注册一个设备
// because that there is no device tree node to auto register this driver,
// we should add a device to kernel

// 仅仅含有设备的加载/卸载过程，无其他处理逻辑
// only has init device and exit device, dont have other logic code
#include <linux/platform_device.h>
#include <linux/module.h>

#define HOST_WIFI_NAME "simulation_wifi"
static char *simuwifi_dev_name = HOST_WIFI_NAME;
static struct platform_device *my_pdev;

static int __init simuwifi_dev_init(void)
{
    printk(KERN_INFO "%s : simuwifi device init start \n", simuwifi_dev_name);
    int ret = 0;
    struct platform_device *pdev = NULL;

    pdev = platform_device_alloc(simuwifi_dev_name, -1);
    if (!pdev)
    {
        printk(KERN_ERR "%s : failed to alloc platform device \n", simuwifi_dev_name);
        return -1;
    }
    my_pdev = pdev;

    ret = platform_device_add(pdev);
    if (ret)
    {
        printk(KERN_ERR "%s : failed to add platform device\n", simuwifi_dev_name);
        platform_device_put(pdev);
        return ret;
    }
    printk(KERN_INFO "%s : device init success \n", simuwifi_dev_name);
    return 0;
}

static void __exit simuwifi_dev_exit(void)
{
    printk(KERN_INFO "%s : device exit \n", simuwifi_dev_name);
    platform_device_unregister(my_pdev);
}

module_init(simuwifi_dev_init);
module_exit(simuwifi_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PolarisZg");
MODULE_DESCRIPTION("add simuwifi device");