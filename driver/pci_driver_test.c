#include <linux/pci.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/msi.h>
#include <asm-generic/io.h>

#include "pci_driver_test.h"

#define PCI_TEST_DEVICE_NAME "pci_driver_test"
#define PCI_VENDOR_ID_QEMU 0x1234
#define TEST_PCI_DEVICE_ID 0x0721
#define TEST_PCI_BUFF_SIZE 0x100
#define TEST_PCI_BAR_NUM 0
#define TEST_PCI_DMA_MASK 36
static void __iomem *mmio_addr = NULL;
static struct test_pci *test_pci_priv = NULL;

static irqreturn_t test_pci_irq_handler(int irq, void *dev)
{
    struct pci_dev *pdev = dev;
    struct test_pci *priv = pci_get_drvdata(pdev);
    pr_info("%s : interrupt start , ioaddr : %p \n", PCI_TEST_DEVICE_NAME, priv->mmio_addr);
    u32 val = ioread32(priv->mmio_addr + 0x30);
    pr_info("%s : interrupt type %08x \n", PCI_TEST_DEVICE_NAME, val);
    iowrite32(0, priv->mmio_addr + 0x30);
    return IRQ_HANDLED;
}

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

    int ret = 0;

    ret = pci_assign_resource(pdev, 0);
    if(ret){
        pr_err("%s : fail to assign pci resource \n", PCI_TEST_DEVICE_NAME);
        return ret;
    }
    // 启用pci设备
    ret = pci_enable_device(pdev);
    if (ret)
    {
        dev_err(&pdev->dev, "%s : pci_ebable_device failed\n", PCI_TEST_DEVICE_NAME);
        return ret;
    }
    pr_info("%s : pci device enable done\n", PCI_TEST_DEVICE_NAME);

    // 申请mmio资源
    ret = pci_request_regions(pdev, PCI_TEST_DEVICE_NAME);
    if (ret)
    {
        dev_err(&pdev->dev, "%s : pci_request_regions failed \n", PCI_TEST_DEVICE_NAME);
        pci_disable_device(pdev);
        return ret;
    }

    // 设置dma地址长度
    ret = dma_set_mask(&pdev->dev, DMA_BIT_MASK(TEST_PCI_DMA_MASK));
    if(ret){
        pr_err("%s : set dma mask err : %d \n", PCI_TEST_DEVICE_NAME, TEST_PCI_DMA_MASK);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return ret;
    }
    pr_info("%s : pci dma mask set %016llx \n", PCI_TEST_DEVICE_NAME, DMA_BIT_MASK(TEST_PCI_DMA_MASK));

    pci_set_master(pdev);

    // 申请 mmio 地址, 对应于pci设备中的 BAR 0 
    mmio_addr = pci_iomap(pdev, TEST_PCI_BAR_NUM, 0);
    if (!mmio_addr)
    {
        dev_err(&pdev->dev, "%s : pci_iomap failed \n", PCI_TEST_DEVICE_NAME);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return -EIO;
    }
    pr_info("%s , pci device request mmio addr done : %p \n", PCI_TEST_DEVICE_NAME, mmio_addr);

    int num_vectors = 0;
    num_vectors = pci_alloc_irq_vectors(pdev, 1, 10, PCI_IRQ_ALL_TYPES); // 最少的向量数要大于 1
    if (num_vectors < 0)
    {
        pr_err("%s : msi malloc error \n", PCI_TEST_DEVICE_NAME);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return num_vectors;
    }
    pr_info("%s : msi vectors : %d \n", PCI_TEST_DEVICE_NAME, num_vectors);
    // 此时应该将中断disable, 以免在驱动程序初始化的过程中发生中断导致出错

    // struct msi_desc *msi_desc;
    // msi_desc = irq_get_msi_desc(pdev->irq);
    // if (!msi_desc)
    // {
    //     pr_err("%s : msi desc null \n", PCI_TEST_DEVICE_NAME);
    //     pci_free_irq_vectors(pdev);
    //     pci_release_regions(pdev);
    //     pci_disable_device(pdev);
    //     return -1;
    // }

    // 添加中断处理函数

    int irq = pci_irq_vector(pdev, 0);
    pr_info("%s : pci irq vector num is %d \n", PCI_TEST_DEVICE_NAME, irq);
    ret = request_irq(irq, test_pci_irq_handler, IRQF_SHARED, "test_pci_irq", pdev);
    // 测试用
    // 实际需要修改为mac80211子系统相关

    // io测试：
    iowrite32(0x00, mmio_addr + 0x20); // 清空写入数据长度
    u32 val = ('a' << 24) + ('b' << 16) + ('c' << 8) + 'd';
    pr_info("%s : val : %08x \n", PCI_TEST_DEVICE_NAME, val);
    iowrite32(val, mmio_addr + 0x10); // 写入32bit数据
    val = ioread32(mmio_addr + 0x20); // 读取写入数据长度 以 char 为单位
    pr_info("%s : val lenth write : %08x \n", PCI_TEST_DEVICE_NAME, val);
    val = ioread32(mmio_addr + 0x10); // 读取写入数据 以 32bit = 4 char 为单位
    pr_info("%s : val value write : %08x \n", PCI_TEST_DEVICE_NAME, val);
    iowrite32(3, mmio_addr + 0x30);
    val = ioread32(mmio_addr + 0x30);
    pr_info("%s : interrupt type val : %08x \n", PCI_TEST_DEVICE_NAME, val);

    size_t priv_size = 0;
    test_pci_priv = kzalloc(sizeof(*test_pci_priv) + priv_size, GFP_KERNEL);
    test_pci_priv->mmio_addr = mmio_addr;
    // test_pci_priv->msi_desc = msi_desc;
    test_pci_priv->irq_vectors_num = num_vectors;
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
        pci_free_irq_vectors(pdev);
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