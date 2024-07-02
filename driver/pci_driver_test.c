#include <linux/pci.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <asm-generic/io.h>

#include "pci_driver_test.h"

#define PCI_TEST_DEVICE_NAME "pci_driver_test"
#define PCI_VENDOR_ID_QEMU 0x1234
#define TEST_PCI_DEVICE_ID 0x0721
#define TEST_PCI_BUFF_SIZE 0x100
#define TEST_PCI_BAR_NUM 0
static void __iomem *mmio_addr = NULL;
static struct test_pci *test_pci_priv = NULL;

static int test_pci_fopen(struct inode *inode, struct file *file)
{
    pr_info("%s : pci device cdev open \n", PCI_TEST_DEVICE_NAME);
    return 0;
}

static int test_pci_frelease(struct inode *inode, struct file *file)
{
    pr_info("%s : pci device cdev release \n", PCI_TEST_DEVICE_NAME);
    return 0;
}

ssize_t test_pci_fread(struct file *file, char __user *data, size_t len, loff_t *l)
{
    pr_info("%s : read start \n", PCI_TEST_DEVICE_NAME);
    if (len > TEST_PCI_BUFF_SIZE || len < 0)
    {
        pr_err("%s : data length so long \n", PCI_TEST_DEVICE_NAME);
        return 0;
    }

    for (ssize_t i = 0; i < len; i++)
    {
        data[i] = ioread8(mmio_addr + i);
    }
    pr_info("%s : kernel receive data \n", PCI_TEST_DEVICE_NAME);
    return 0;
}

ssize_t test_pci_fwrite(struct file *file, const char __user *data, size_t len, loff_t *l)
{
    pr_info("%s : write start \n", PCI_TEST_DEVICE_NAME);
    if (len > TEST_PCI_BUFF_SIZE || len < 0)
    {
        pr_err("%s : data length so long \n", PCI_TEST_DEVICE_NAME);
        return 0;
    }

    for (ssize_t i = 0; i < len; i++)
    {
        iowrite8(data[i], mmio_addr + i);
    }
    iowrite8('\0', mmio_addr + len);

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
    pr_info("%s : pci device enable done\n", PCI_TEST_DEVICE_NAME);

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
    pr_info("%s , pci device request mmio addr done : %p \n", PCI_TEST_DEVICE_NAME, mmio_addr);

    // 测试用
    // 实际需要修改为mac80211子系统相关

    // io测试：
    iowrite32(0x00, mmio_addr + 0x20);
    u32 val = ('a' << 24) + ('b' << 16) + ('c' << 8) + 'd';
    pr_info("%s : val : %08x \n", PCI_TEST_DEVICE_NAME, val);
    iowrite32(val, mmio_addr + 0x10);
    val = ioread32(mmio_addr + 0x20);
    pr_info("%s : val lenth write : %08x \n", PCI_TEST_DEVICE_NAME, val);
    val = ioread32(mmio_addr + 0x10);
    pr_info("%s : val value write : %08x \n", PCI_TEST_DEVICE_NAME, val);

    size_t priv_size = 0;
    test_pci_priv = kzalloc(sizeof(*test_pci_priv) + priv_size, GFP_KERNEL);
    test_pci_priv->mmio_addr = mmio_addr;
    pci_set_drvdata(pdev, test_pci_priv);
    pr_info("%s : pci device probe done \n", PCI_TEST_DEVICE_NAME);
    return 0;
}

static void test_pci_remove(struct pci_dev *pdev)
{
    pr_info("%s : pci device remove start \n", PCI_TEST_DEVICE_NAME);
    struct test_pci *test_pci_priv = pci_get_drvdata(pdev);
    if (test_pci_priv)
    {
        iowrite32(0x00, test_pci_priv->mmio_addr + 0x20);
        pci_iounmap(pdev, test_pci_priv->mmio_addr);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        kfree(test_pci_priv);
    }
}

static void test_pci_shutdown(struct pci_dev *pdev)
{
    test_pci_remove(pdev);
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
    pr_info("%s : pci device init \n", PCI_TEST_DEVICE_NAME);
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
    pr_info("%s : pci device exit \n", PCI_TEST_DEVICE_NAME);
    pci_unregister_driver(&test_pci_driver);
}
module_exit(test_pci_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PolarisZg");
MODULE_DESCRIPTION("driver for pci device to test work");