#include <linux/platform_device.h>
#include <linux/module.h>

static char* my_device_name = "hellowd_dev";

static int hellowd_dev_probe(struct platform_device *pdev)
{
    printk("hello world platform device \n");
    return 0;

err_free_dev:

    return 1;
}

static int hellowd_dev_remove(struct platform_device *pdev)
{
    printk("bye world platform device \n");
    return 0;
}

static struct platform_driver hellowd_dev_driver = {
    .driver = {
        .name = my_device_name,
        .owner = THIS_MODULE},
    .probe = hellowd_dev_probe,
    .remove = hellowd_dev_remove};

module_platform_driver(hellowd_dev_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("yukikaze");
MODULE_DESCRIPTION("hello world platform");

// 因为没有设备树中的结点供该moudle注册，因此需要手动向内核注册一个设备
// because that there is no device tree node to auto register this driver,
// we should add a device to kernel

static int __init hellowd_dev_init(void)
{
    printk("hellowd platform device init\n");
    int ret = 0;
    struct platform_device *pdev = NULL;

    pdev = platform_device_alloc(my_device_name, -1);
    if(!pdev){
        printk(KERN_ERR "failed to alloc platform device : %s \n", my_device_name);
    }

    ret = platform_device_add(pdev);
    if(ret) {
        printk(KERN_ERR "failed to add platform device  : %s \n", my_device_name);
    }
}

static void __exit hellowd_dev_exit(void)
{
    printk("hellowd platform exit\n");
    platform_driver_unregister(&hellowd_dev_driver);
}

module_init(hellowd_dev_init);
module_exit(hellowd_dev_exit);