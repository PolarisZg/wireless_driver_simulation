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
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/dmaengine.h>

#include <linux/skbuff.h>
#include <linux/mutex.h>

#include "wireless.h"
#include "wireless_dma.h"

static void wireless_tx_end(struct wireless_simu *priv)
{
    struct pci_dev *pdev = priv->pci_dev;
    u32 dma_tx_ring_id = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_OUT_BUFF_ID);
    u32 dma_tx_flag = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_OUT_FLAG);
    pr_info("%s : mem to device dma end ; ring id %08x , flag %08x\n", WIRELESS_SIMU_DEVICE_NAME, dma_tx_ring_id, dma_tx_flag);
    priv->tx_ring.tx_list[dma_tx_ring_id].flag = dma_tx_flag;
    dma_unmap_single(&pdev->dev, priv->tx_ring.tx_list[dma_tx_ring_id].dma_addr, priv->tx_ring.tx_list[dma_tx_ring_id].data_length, DMA_MEM_TO_DEV);
    iowrite32(0, priv->mmio_addr + WIRELESS_REG_EVENT);
}

static void wireless_rx(struct wireless_simu *priv)
{
    // struct pci_dev *pdev = priv->pci_dev;
    u32 dma_rx_ring_id = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_OUT_BUFF_ID);
    u32 dma_rx_length = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_OUT_LENGTH);
    u32 dma_rx_flag = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_OUT_FLAG);
    u32 val;
    if (dma_rx_ring_id >= WIRELESS_RX_RING_SIZE)
    {
        pr_err("%s : rx ring id BIG \n", WIRELESS_SIMU_DEVICE_NAME);
        return;
    }
    struct Wireless_DMA_Buf *dma_buf = &priv->rx_ring.rx_list[dma_rx_ring_id];
    char *data = dma_buf->data;
    pr_info("%s : rx len : %08x %c %c %c %c %c %c flag %08x \n", WIRELESS_SIMU_DEVICE_NAME, dma_rx_length, data[0], data[1], data[2], data[3], data[4], data[5], dma_rx_flag);

    // rx结束, 清理设备中对应的rx ring
    WIRELESS_BITCLR(dma_buf->flag, WIRELESS_RX_RING_BUF_IS_USING);
    WIRELESS_BITSET(dma_buf->flag, WIRELESS_RX_RING_BUF_INIT_END);

    iowrite32(dma_rx_ring_id, priv->mmio_addr + WIRELESS_REG_DMA_RX_RING_BUF_ID);
    val = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_RX_RING_BUF_ID);
    pr_info("%s : rx : clr ring id %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    iowrite32(dma_buf->flag, priv->mmio_addr + WIRELESS_REG_DMA_RX_RING_FLAG);
    val = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_RX_RING_FLAG);
    pr_info("%s : rx : clr ring flag %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    iowrite32(0, priv->mmio_addr + WIRELESS_REG_EVENT);
}

static irqreturn_t wireless_simu_irq_handler(int irq, void *dev)
{
    struct pci_dev *pdev = dev;
    struct wireless_simu *priv = pci_get_drvdata(pdev);
    u32 irq_status = ioread32(priv->mmio_addr + WIRELESS_REG_IRQ_STATUS);
    pr_info("%s : interrupt start , ioaddr : %p , irq_status %08x \n", WIRELESS_SIMU_DEVICE_NAME, priv->mmio_addr, irq_status);
    switch (irq_status)
    {
    case WIRELESS_IRQ_TEST:
        pr_info("%s : irq test\n", WIRELESS_SIMU_DEVICE_NAME);
        iowrite32(0, priv->mmio_addr + WIRELESS_REG_EVENT);
        break;
    case WIRELESS_IRQ_DNA_MEM_TO_DEVICE_END:
        wireless_tx_end(priv);
        break;
    case WIRELESS_IRQ_DMA_DEVICE_TO_MEM_END:
        wireless_rx(priv);
        break;
    default:
        break;
    }

    return IRQ_HANDLED;
}

static const struct pci_device_id wireless_simu_id_table[] = {
    {PCI_DEVICE(PCI_VENDOR_ID_QEMU, PCI_DEVICE_ID_WIRELESS_SIMU)},
    {0},
};
MODULE_DEVICE_TABLE(pci, wireless_simu_id_table);


static int wireless_simu_pci_claim(struct wireless_simu* priv)
{
    int ret = 0;
    static void __iomem *mmio_addr = NULL;

    ret = pci_assign_resource(priv->pci_dev, 0);
    if (ret)
    {
        pr_err("%s : fail to assign pci resource \n", WIRELESS_SIMU_DEVICE_NAME);
        return ret;
    }

    // 启用pci设备
    ret = pci_enable_device(priv->pci_dev);
    if (ret)
    {
        dev_err(&priv->pci_dev->dev, "%s : pci_ebable_device failed\n", WIRELESS_SIMU_DEVICE_NAME);
        goto PCI_END;
    }
    pr_info("%s : pci device enable done\n", WIRELESS_SIMU_DEVICE_NAME);

    // 申请mmio资源
    ret = pci_request_regions(priv->pci_dev, WIRELESS_SIMU_DEVICE_NAME);
    if (ret)
    {
        dev_err(&priv->pci_dev->dev, "%s : pci_request_regions failed \n", WIRELESS_SIMU_DEVICE_NAME);
        goto DISABLE_PCI;
    }

    // 设置dma地址长度
    ret = dma_set_mask_and_coherent(&priv->pci_dev->dev, DMA_BIT_MASK(WIRELESS_SIMU_DMA_MASK));
    if (ret)
    {
        pr_err("%s : set dma mask err : %d \n", WIRELESS_SIMU_DEVICE_NAME, WIRELESS_SIMU_DMA_MASK);
        goto RELEASE_REGIONS;
    }
    pr_info("%s : pci dma mask set %016llx \n", WIRELESS_SIMU_DEVICE_NAME, DMA_BIT_MASK(WIRELESS_SIMU_DMA_MASK));

    pci_set_master(priv->pci_dev);

    // 申请 mmio 地址, 对应于pci设备中的 BAR 0
    mmio_addr = pci_iomap(priv->pci_dev, WIRELESS_SIMU_BAR_NUM, 0);
    if (!mmio_addr)
    {
        dev_err(&priv->pci_dev->dev, "%s : pci_iomap failed \n", WIRELESS_SIMU_DEVICE_NAME);
        ret = -EIO;
        goto RELEASE_REGIONS;
    }
    pr_info("%s , pci device request mmio addr done : %p \n", WIRELESS_SIMU_DEVICE_NAME, mmio_addr);
    priv->mmio_addr = mmio_addr;
    return 0;

RELEASE_REGIONS:
    pci_release_regions(priv->pci_dev);
DISABLE_PCI:
    pci_disable_device(priv->pci_dev);
PCI_END:
    return ret;
}

static int wireless_simu_pci_irq_init(struct wireless_simu* priv)
{
    int num_vectors = 0;
    int ret = 0;
    num_vectors = pci_alloc_irq_vectors(priv->pci_dev, 1, 10, PCI_IRQ_MSI); // 最少的向量数要大于 1
    if (num_vectors < 0)
    {
        priv->msi_enable = false;
        pr_info("%s : this kernel dont support msi \n", WIRELESS_SIMU_DEVICE_NAME);
        num_vectors = pci_alloc_irq_vectors(priv->pci_dev, 1, 10, PCI_IRQ_ALL_TYPES);
    }
    else
    {
        priv->msi_enable = true;
        pr_info("%s : msi enabled \n", WIRELESS_SIMU_DEVICE_NAME);
    }

    if (num_vectors < 0)
    {
        pr_err("%s : msi malloc error \n", WIRELESS_SIMU_DEVICE_NAME);
        pci_release_regions(priv->pci_dev);
        pci_disable_device(priv->pci_dev);
        return num_vectors;
    }
    pr_info("%s : msi vectors : %d \n", WIRELESS_SIMU_DEVICE_NAME, num_vectors);
    // 此时应该将中断disable, 以免在驱动程序初始化的过程中发生中断导致出错

    struct msi_desc *msi_desc;
    if (priv->msi_enable)
    {
        msi_desc = irq_get_msi_desc(priv->pci_dev->irq);
        if (!msi_desc)
        {
            pr_err("%s : msi desc null \n", WIRELESS_SIMU_DEVICE_NAME);
            pci_free_irq_vectors(priv->pci_dev);
            pci_release_regions(priv->pci_dev);
            pci_disable_device(priv->pci_dev);
            return -1;
        }
        priv->msi_desc = msi_desc;
    }

    int irq = pci_irq_vector(priv->pci_dev, 0);
    pr_info("%s : pci irq vector num is %d \n", WIRELESS_SIMU_DEVICE_NAME, irq);
    ret = request_irq(irq, wireless_simu_irq_handler, IRQF_SHARED, "wireless_simu_irq", priv->pci_dev);
    priv->irq_vectors_num = num_vectors;

    return 0;
}

static int wireless_simu_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id_dev)
{
    pr_info("%s : pci probe info from pci_device : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            WIRELESS_SIMU_DEVICE_NAME, pdev->vendor, pdev->device, pdev->subsystem_vendor, pdev->subsystem_device);
    pr_info("%s : pci probe info from pci_dev_id : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            WIRELESS_SIMU_DEVICE_NAME, pci_id_dev->vendor, pci_id_dev->device, pci_id_dev->subvendor, pci_id_dev->subdevice);

    static struct wireless_simu *priv = NULL;
    int ret = 0;
    u32 val = 0;
    size_t priv_size = 0;
    priv = kzalloc(sizeof(struct wireless_simu) + priv_size, GFP_KERNEL);
    priv->stop = false;
    priv->pci_dev = pdev;

    ret = wireless_simu_pci_claim(priv);
    if(ret){
        pr_err("%s : pci clim err %d \n", WIRELESS_SIMU_DEVICE_NAME, ret);
        goto End;
    }

    ret = wireless_simu_pci_irq_init(priv);
    if(ret){
        pr_err("%s : pci irq err %d \n", WIRELESS_SIMU_DEVICE_NAME, ret);
        goto End;
    }

    pci_set_drvdata(priv->pci_dev, priv);

    ret = wireless_simu_rx_ring_init(priv);
    if (ret)
    {
        pr_info("%s : rx ring init err %d", WIRELESS_SIMU_DEVICE_NAME, ret);
        wireless_rx_ring_exit(priv);
        goto End;
    }

    ret = wireless_simu_tx_ring_init(priv);
    if(ret){
        pr_info("%s : tx ring init err %08x \n", WIRELESS_SIMU_DEVICE_NAME, ret);
        goto End;
    }
    pr_info("%s : tx ring list init success \n", WIRELESS_SIMU_DEVICE_NAME);

    // 启动中断
    iowrite32(1, priv->mmio_addr + WIRELESS_REG_IRQ_ENABLE);
    val = ioread32(priv->mmio_addr + WIRELESS_REG_IRQ_ENABLE);
    pr_info("%s : probe device irq enable \n", WIRELESS_SIMU_DEVICE_NAME);

    pr_info("%s : pci device probe done \n", WIRELESS_SIMU_DEVICE_NAME);

    wireless_simu_dma_test(priv);

    wireless_mac80211_core_probe(priv);
    return 0;

End:
    return ret;
}

static void wireless_simu_pci_remove(struct pci_dev *pdev)
{
    pr_info("%s : pci device remove start \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *priv = pci_get_drvdata(pdev);
    if (priv)
    {
        priv->stop = true;
        iowrite32(0x00, priv->mmio_addr + WIRELESS_REG_EVENT);
        iowrite32(0x00, priv->mmio_addr + WIRELESS_REG_IRQ_ENABLE);
        wireless_tx_ring_exit(priv);
        wireless_rx_ring_exit(priv);
        pci_iounmap(pdev, priv->mmio_addr);
        pci_free_irq_vectors(pdev);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        kfree(priv->tx_ring.tx_list);
        kfree(priv);
    }
}

static void wireless_simu_shutdown(struct pci_dev *pdev)
{
    wireless_simu_pci_remove(pdev);
}

static struct pci_driver wireless_simu_driver = {
    .name = WIRELESS_SIMU_DEVICE_NAME,
    .id_table = wireless_simu_id_table,
    .probe = wireless_simu_pci_probe,
    .remove = wireless_simu_pci_remove,
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