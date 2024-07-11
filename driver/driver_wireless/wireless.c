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

#include "wireless.h"

#define WIRELESS_SIMU_DEVICE_NAME "wirelesssimu"
#define PCI_VENDOR_ID_QEMU 0x1234
#define PCI_DEVICE_ID_WIRELESS_SIMU 0x1145
#define WIRELESS_SIMU_BUFF_SIZE 0x100
#define WIRELESS_SIMU_BAR_NUM 0
#define WIRELESS_SIMU_DMA_MASK 32
static void __iomem *mmio_addr = NULL;
static struct wireless_simu *wireless_simu_priv = NULL;

static irqreturn_t wireless_simu_irq_handler(int irq, void *dev)
{
    struct pci_dev *pdev = dev;
    struct wireless_simu *priv = pci_get_drvdata(pdev);
    pr_info("%s : interrupt start , ioaddr : %p \n", WIRELESS_SIMU_DEVICE_NAME, priv->mmio_addr);
    u32 val = ioread32(priv->mmio_addr + 0x30);
    pr_info("%s : interrupt type %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);
    iowrite32(0, priv->mmio_addr + 0x30);
    return IRQ_HANDLED;
}

static const struct pci_device_id wireless_simu_id_table[] = {
    {PCI_DEVICE(PCI_VENDOR_ID_QEMU, PCI_DEVICE_ID_WIRELESS_SIMU)},
    {0},
};
MODULE_DEVICE_TABLE(pci, wireless_simu_id_table);

static int wireless_simu_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id_dev)
{
    pr_info("%s : pci probe info from pci_device : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            WIRELESS_SIMU_DEVICE_NAME, pdev->vendor, pdev->device, pdev->subsystem_vendor, pdev->subsystem_device);
    pr_info("%s : pci probe info from pci_dev_id : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            WIRELESS_SIMU_DEVICE_NAME, pci_id_dev->vendor, pci_id_dev->device, pci_id_dev->subvendor, pci_id_dev->subdevice);

    int ret = 0;

    ret = pci_assign_resource(pdev, 0);
    if (ret)
    {
        pr_err("%s : fail to assign pci resource \n", WIRELESS_SIMU_DEVICE_NAME);
        return ret;
    }
    // 启用pci设备
    ret = pci_enable_device(pdev);
    if (ret)
    {
        dev_err(&pdev->dev, "%s : pci_ebable_device failed\n", WIRELESS_SIMU_DEVICE_NAME);
        return ret;
    }
    pr_info("%s : pci device enable done\n", WIRELESS_SIMU_DEVICE_NAME);

    // 申请mmio资源
    ret = pci_request_regions(pdev, WIRELESS_SIMU_DEVICE_NAME);
    if (ret)
    {
        dev_err(&pdev->dev, "%s : pci_request_regions failed \n", WIRELESS_SIMU_DEVICE_NAME);
        pci_disable_device(pdev);
        return ret;
    }

    // 设置dma地址长度
    ret = dma_set_mask(&pdev->dev, DMA_BIT_MASK(WIRELESS_SIMU_DMA_MASK));
    if (ret)
    {
        pr_err("%s : set dma mask err : %d \n", WIRELESS_SIMU_DEVICE_NAME, WIRELESS_SIMU_DMA_MASK);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return ret;
    }
    pr_info("%s : pci dma mask set %016llx \n", WIRELESS_SIMU_DEVICE_NAME, DMA_BIT_MASK(WIRELESS_SIMU_DMA_MASK));

    pci_set_master(pdev);

    // 申请 mmio 地址, 对应于pci设备中的 BAR 0
    mmio_addr = pci_iomap(pdev, WIRELESS_SIMU_BAR_NUM, 0);
    if (!mmio_addr)
    {
        dev_err(&pdev->dev, "%s : pci_iomap failed \n", WIRELESS_SIMU_DEVICE_NAME);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return -EIO;
    }
    pr_info("%s , pci device request mmio addr done : %p \n", WIRELESS_SIMU_DEVICE_NAME, mmio_addr);

    int num_vectors = 0;
    num_vectors = pci_alloc_irq_vectors(pdev, 1, 10, PCI_IRQ_MSI); // 最少的向量数要大于 1
    int test_msi_isenable = 0;
    if (num_vectors < 0)
    {
        test_msi_isenable = 0;
        pr_info("%s : this kernel dont support msi \n", WIRELESS_SIMU_DEVICE_NAME);
        num_vectors = pci_alloc_irq_vectors(pdev, 1, 10, PCI_IRQ_ALL_TYPES);
    }
    else
    {
        test_msi_isenable = 1;
        pr_info("%s : msi enabled \n", WIRELESS_SIMU_DEVICE_NAME);
    }
    if (num_vectors < 0)
    {
        pr_err("%s : msi malloc error \n", WIRELESS_SIMU_DEVICE_NAME);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        return num_vectors;
    }
    pr_info("%s : msi vectors : %d \n", WIRELESS_SIMU_DEVICE_NAME, num_vectors);
    // 此时应该将中断disable, 以免在驱动程序初始化的过程中发生中断导致出错

    struct msi_desc *msi_desc;
    if (test_msi_isenable)
    {
        msi_desc = irq_get_msi_desc(pdev->irq);
        if (!msi_desc)
        {
            pr_err("%s : msi desc null \n", WIRELESS_SIMU_DEVICE_NAME);
            pci_free_irq_vectors(pdev);
            pci_release_regions(pdev);
            pci_disable_device(pdev);
            return -1;
        }
    }

    // 添加中断处理函数

    int irq = pci_irq_vector(pdev, 0);
    pr_info("%s : pci irq vector num is %d \n", WIRELESS_SIMU_DEVICE_NAME, irq);
    ret = request_irq(irq, wireless_simu_irq_handler, IRQF_SHARED, "wireless_simu_irq", pdev);
    // 测试用
    // 实际需要修改为mac80211子系统相关

    // io测试：
    u32 val = 0;
    iowrite32(0x1919810, mmio_addr + 0x00); // 清空写入数据长度
    val = ioread32(mmio_addr + 0x00);
    pr_info("%s : read from 0x00 : %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);
    
    size_t priv_size = 0;
    wireless_simu_priv = kzalloc(sizeof(*wireless_simu_priv) + priv_size, GFP_KERNEL);
    wireless_simu_priv->mmio_addr = mmio_addr;
    if(test_msi_isenable){
        wireless_simu_priv->msi_desc = msi_desc;
    }
    wireless_simu_priv->irq_vectors_num = num_vectors;
    pci_set_drvdata(pdev, wireless_simu_priv);
    pr_info("%s : pci device probe done \n", WIRELESS_SIMU_DEVICE_NAME);
    return 0;
}

static void wireless_simu_remove(struct pci_dev *pdev)
{
    pr_info("%s : pci device remove start \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *wireless_simu_priv = pci_get_drvdata(pdev);
    if (wireless_simu_priv)
    {
        iowrite32(0x00, wireless_simu_priv->mmio_addr + 0x20);
        pci_iounmap(pdev, wireless_simu_priv->mmio_addr);
        pci_free_irq_vectors(pdev);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        kfree(wireless_simu_priv);
    }
}

static void wireless_simu_shutdown(struct pci_dev *pdev)
{
    wireless_simu_remove(pdev);
}

static struct pci_driver wireless_simu_driver = {
    .name = WIRELESS_SIMU_DEVICE_NAME,
    .id_table = wireless_simu_id_table,
    .probe = wireless_simu_probe,
    .remove = wireless_simu_remove,
    .shutdown = wireless_simu_shutdown,
};

static int __init wireless_simu_init(void)
{
    pr_info("%s : pci device init \n", WIRELESS_SIMU_DEVICE_NAME);
    int err = 0;
    err = pci_register_driver(&wireless_simu_driver);
    if (err)
    {
        pr_err("failed to register test pci driver: %d\n", err);
    }
    return err;
}
module_init(wireless_simu_init);

static void __exit wireless_simu_exit(void)
{
    pr_info("%s : pci device exit \n", WIRELESS_SIMU_DEVICE_NAME);
    pci_unregister_driver(&wireless_simu_driver);
}
module_exit(wireless_simu_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PolarisZg");
MODULE_DESCRIPTION("driver for simu wireless device");