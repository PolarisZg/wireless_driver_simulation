#include <linux/pci.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>

#include "pci_driver_test.h"

#define PCI_TEST_DEVICE_NAME "test_pci"
#define PCI_TEST_CDEVICE_NAME "test_pci_cdev"
#define PCI_VENDOR_ID_QEMU 0x1234
#define TEST_PCI_DEVICE_ID 0x0721
#define TEST_PCI_BUFF_SIZE 0x100
#define TEST_PCI_BAR_NUM 0
static void __iomem *mmio_addr;
static struct test_pci *test_pci_priv;

static int test_pci_fopen(struct inode *inode, struct file *file){
    return 0;
}

static int test_pci_frelease(struct inode *inode, struct file *file){
    return 0;
}

ssize_t test_pci_fread(struct file *file, char __user * data, size_t len, loff_t *l){
    return 0;
}

ssize_t test_pci_fwrite(struct file *file, const char __user *data, size_t len, loff_t *l){
    int err = 0;
    if(len > TEST_PCI_BUFF_SIZE || len < 0){
        pr_err("%s : data length so long \n", PCI_TEST_DEVICE_NAME);
        return 0;
    }
        
    err = copy_from_user(mmio_addr, data, len);
    if(err){
        pr_err("%s : device received data failed \n", PCI_TEST_DEVICE_NAME);
        return 0;
    }

    pr_info("%s : device receive data \n", PCI_TEST_DEVICE_NAME);
    return 0;
}

static const struct file_operations test_pci_fops = {
    .owner = THIS_MODULE,
    .open = test_pci_fopen,
    .release = test_pci_frelease,
    .read = test_pci_fread,
    .write = test_pci_fwrite,
};

static const struct pci_device_id test_pci_id_table[] = {
    {PCI_DEVICE(PCI_VENDOR_ID_QEMU, TEST_PCI_DEVICE_ID)},
    {0},
};
MODULE_DEVICE_TABLE(pci, test_pci_id_table);

static int test_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id_dev)
{
    pr_info("%s : pci probe info from pci_device : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            PCI_TEST_DEVICE_NAME, pdev->vendor, pdev->device, pdev->subsystem_vendor, pdev->subsystem_device);
    pr_info("%s : pci probe info from pci_dev_id : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            PCI_TEST_DEVICE_NAME, pci_id_dev->vendor, pci_id_dev->device, pci_id_dev->subvendor, pci_id_dev->subdevice);

    int err = 0;

    // 启用pci设备
    err = pci_enable_device(pdev);
    if (err)
    {
        dev_err(&pdev->dev, "%s : pci_ebable_device failed\n", PCI_TEST_DEVICE_NAME);
        return err;
    }

    // 申请mmio / pmio资源
    err = pci_request_regions(pdev, PCI_TEST_DEVICE_NAME); //
    if (err)
    {
        dev_err(&pdev->dev, "%s : pci_request_regions failed \n", PCI_TEST_DEVICE_NAME);
        pci_disable_device(pdev);
        return err;
    }

    mmio_addr = pcim_iomap(pdev, TEST_PCI_BAR_NUM, 0);
    if (!mmio_addr)
    {
        dev_err(&pdev->dev, "%s : pci_iomap failed \n", PCI_TEST_DEVICE_NAME);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return -EIO;
    }

    // 测试用，字符设备
    // 实际需要修改为mac80211子系统相关

    u32 test_pci_cdev_num;
    err = alloc_chrdev_region(&test_pci_cdev_num, 0, 1, PCI_TEST_CDEVICE_NAME);
    if (err < 0)
    {
        dev_err(&pdev->dev, "%s : char dev alloc failed \n", PCI_TEST_DEVICE_NAME);
        pcim_iounmap(pdev, mmio_addr);
        mmio_addr = NULL;
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return err;
    }

    struct cdev *test_pci_cdev;
    cdev_init(test_pci_cdev, &test_pci_fops);
    err = cdev_add(test_pci_cdev, test_pci_cdev_num, 1);
    if (err < 0)
    {
        dev_err(&pdev->dev, "%s : char dev add kernel map failed \n", PCI_TEST_DEVICE_NAME);
        unregister_chrdev_region(test_pci_cdev_num, 1);
        test_pci_cdev_num = 0;
        pcim_iounmap(pdev, mmio_addr);
        mmio_addr = NULL;
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return err;
    }

    size_t priv_size = 0;
    test_pci_priv = kzalloc(sizeof(*test_pci_priv) + priv_size, GFP_KERNEL);
    test_pci_priv->mmio_addr = mmio_addr;
    test_pci_priv->cdev_num = test_pci_cdev_num;
    test_pci_priv->pci_dev = pdev;
    test_pci_priv->char_dev = test_pci_cdev;
    pci_set_drvdata(pdev, test_pci_priv);
    return 0;
}

static void test_pci_remove(struct pci_dev *pdev)
{
}

static void test_pci_shutdown(struct pci_dev *pdev)
{
}

static struct pci_driver test_pci_driver = {
    .name = PCI_TEST_DEVICE_NAME,
    .id_table = test_pci_id_table,
    .probe = test_pci_probe,
    .remove = test_pci_remove,
    .shutdown = test_pci_shutdown,
};

static int __init test_pci_init(void)
{
    int err = 0;
    err = pci_register_driver(&test_pci_driver);
    if (err)
    {
        pr_err("failed to register test pci driver: %d\n", err);
    }
    return err;
}
module_init(test_pci_init);

static void __exit test_pci_exit(void)
{
    pci_unregister_driver(&test_pci_driver);
}
module_exit(test_pci_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PolarisZg");
MODULE_DESCRIPTION("driver for pci device to test work");