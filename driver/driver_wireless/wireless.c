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

#define WIRELESS_SIMU_DEVICE_NAME "wirelesssimu"
#define PCI_VENDOR_ID_QEMU 0x1234
#define PCI_DEVICE_ID_WIRELESS_SIMU 0x1145
#define WIRELESS_SIMU_BUFF_SIZE 0x100
#define WIRELESS_SIMU_BAR_NUM 0
#define WIRELESS_SIMU_DMA_MASK 32
#define WIRELESS_TX_RING_SIZE 10
#define WIRELESS_RX_RING_SIZE 2
#define WIRELESS_RX_BUFF_MAX_SIZE 2048

#define WIRELESS_REG_TEST 0x00
#define WIRELESS_REG_EVENT 0x10
#define WIRELESS_REG_IRQ_STATUS 0x50

#define WIRELESS_REG_DMA_IN_HOSTADDR 0x20
#define WIRELESS_REG_DMA_IN_LENGTH 0x30
#define WIRELESS_REG_DMA_IN_BUFF_ID 0x40
#define WIRELESS_REG_DMA_IN_FLAG 0xA0
#define WIRELESS_REG_DMA_OUT_HOSTADDR 0x60
#define WIRELESS_REG_DMA_OUT_LENGTH 0x70
#define WIRELESS_REG_DMA_OUT_BUFF_ID 0x80
#define WIRELESS_REG_DMA_OUT_FLAG 0x90

#define SETBIT(x, y) (x |= 1 << y)
#define CLBIT(x, y) (x &= ~(1 << y))

static void __iomem *mmio_addr = NULL;
static struct wireless_simu *wireless_simu_priv = NULL;

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
        u32 dma_tx_ring_id = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_OUT_BUFF_ID);
        u32 dma_tx_flag = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_OUT_FLAG);
        pr_info("%s : mem to device dma end ; ring id %08x , flag %08x\n", WIRELESS_SIMU_DEVICE_NAME, dma_tx_ring_id, dma_tx_flag);
        priv->tx_ring.tx_list[dma_tx_ring_id].flag = dma_tx_flag;
        dma_unmap_single(&pdev->dev, priv->tx_ring.tx_list[dma_tx_ring_id].dma_addr, priv->tx_ring.tx_list[dma_tx_ring_id].data_length, DMA_MEM_TO_DEV);
        iowrite32(0, priv->mmio_addr + WIRELESS_REG_EVENT);
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

static int wireless_simu_rx_ring_init(struct wireless_simu *wireless_simu_priv)
{
    struct Wireless_Rx_Ring *rx_ring = &wireless_simu_priv->rx_ring;
    int ret;
    rx_ring->rx_list_size = WIRELESS_RX_RING_SIZE;
    if (rx_ring->rx_list == NULL)
    {
        rx_ring->rx_list = (struct Wireless_DMA_Buf *)kmalloc(rx_ring->rx_list_size * sizeof(struct Wireless_DMA_Buf), GFP_KERNEL);
        if(rx_ring->rx_list == NULL){
            ret = -1;
            return ret;
        }
    }
    for(int i = 0; i < rx_ring->rx_list_size; i++){
        struct Wireless_DMA_Buf *dma_buf = &rx_ring->rx_list[i];
        dma_buf->flag = 0;
        dma_buf->data_length = 0;
        dma_addr_t dma_addr = 0;
        // 必须使用dma_alloc_coherent分配空间以突破cache的限制
        dma_buf->data = dma_alloc_coherent(&wireless_simu_priv->pci_dev->dev, WIRELESS_RX_BUFF_MAX_SIZE, &dma_addr, GFP_KERNEL);
        if(dma_buf->data == NULL){
            ret = -2;
            return ret;
        }
        pr_info("%s : rx ring init dma addr %llx \n", WIRELESS_SIMU_DEVICE_NAME, dma_addr);
        dma_buf->dma_addr = dma_addr;
    }
    pr_info("%s : rx ring init success \n", WIRELESS_SIMU_DEVICE_NAME);
    return 0;
}

/*
 * */
static int wireless_simu_rx_ring_dma_init(struct wireless_simu *wireless_simu_priv)
{
    struct Wireless_Rx_Ring *rx_ring = &wireless_simu_priv->rx_ring;
    int ret;
    if(rx_ring->rx_list == NULL){
        ret = wireless_simu_rx_ring_init(wireless_simu_priv);
        if(ret){
            return ret;
        }
    }
    
}

static int wireless_simu_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id_dev)
{
    pr_info("%s : pci probe info from pci_device : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            WIRELESS_SIMU_DEVICE_NAME, pdev->vendor, pdev->device, pdev->subsystem_vendor, pdev->subsystem_device);
    pr_info("%s : pci probe info from pci_dev_id : vendor : %04x , device id : %04x , subsystem vendor : %04x , subsystem id : %04x \n",
            WIRELESS_SIMU_DEVICE_NAME, pci_id_dev->vendor, pci_id_dev->device, pci_id_dev->subvendor, pci_id_dev->subdevice);

    int ret = 0;
    size_t priv_size = 0;
    wireless_simu_priv = kzalloc(sizeof(*wireless_simu_priv) + priv_size, GFP_KERNEL);

    /*
     * 对DMA功能的测试，测试数据在函数前部创造；
     * 在pci设备初始化完毕后再开的话，万一没开出来便样衰了；
     * 还要把pci相关的一个个退出
     * */
    void *data = NULL;
    u32 data_length = 1024;
    data = kmalloc(data_length, GFP_KERNEL);
    if (data == NULL)
    {
        pr_err("%s : tx test data malloc error \n", WIRELESS_SIMU_DEVICE_NAME);
        return -1;
    }

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
    ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(WIRELESS_SIMU_DMA_MASK));
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

    /*
     * tx DMA 测试
     * 当前仅测试单次DMA写入功能，多次DMA写入需要构造环形缓冲区，防止写入的数据冲突
     *
     * 在传输数据前需要将数据线性化(放入一段连续的物理内存中), skb中有自己的线性化api
     * */
    // skb_linearize(skb);

    // 初始化 tx_ring 结构
    struct Wireless_Tx_Ring *tx_ring = &wireless_simu_priv->tx_ring;
    struct mutex *dma_tx_mutex = &tx_ring->tx_ring_mutex;
    tx_ring->tx_list_size = WIRELESS_TX_RING_SIZE;
    if (tx_ring->tx_list == NULL)
    {
        tx_ring->tx_list = (struct Wireless_DMA_Buf *)kzalloc(tx_ring->tx_list_size * sizeof(struct Wireless_DMA_Buf), GFP_KERNEL);
        if (tx_ring->tx_list == NULL)
        {
            pr_err("%s : tx ring malloc null \n", WIRELESS_SIMU_DEVICE_NAME);
            goto testEnd;
        }
    }
    mutex_init(dma_tx_mutex);
    pr_info("%s : tx ring list init success \n", WIRELESS_SIMU_DEVICE_NAME);

    struct Wireless_DMA_Buf *tx_list = tx_ring->tx_list;
    mutex_lock(dma_tx_mutex);
    dma_addr_t dma_addr = dma_map_single(&pdev->dev, data, data_length, DMA_MEM_TO_DEV);
    if (dma_mapping_error(&pdev->dev, dma_addr))
    {
        pr_err("%s : dma_addr err \n", WIRELESS_SIMU_DEVICE_NAME);
        kfree(data);
        mutex_unlock(dma_tx_mutex);
        goto testEnd;
    }
    pr_info("%s : test dma dma_addr %llx \n", WIRELESS_SIMU_DEVICE_NAME, dma_addr);
    u32 dma_addr_low32 = dma_addr;
    pr_info("%s : test dma dma_addr  low 32 %08x \n", WIRELESS_SIMU_DEVICE_NAME, dma_addr_low32);
    // 寻找驱动的 tx 缓存 list 空位
    u32 tx_ring_id = 0;
    for (tx_ring_id = 0; tx_ring_id < tx_ring->tx_list_size; tx_ring_id++)
    {
        if ((tx_list[tx_ring_id].flag & 0x01) == 0) // 使用flag的最后一位检查该缓存是否被占用
        {
            break;
        }
    }
    if (tx_ring_id == tx_ring->tx_list_size)
    {
        pr_err("%s : no tx ring empty for data \n", WIRELESS_SIMU_DEVICE_NAME);
        dma_unmap_single(&pdev->dev, dma_addr, data_length, DMA_MEM_TO_DEV);
        mutex_unlock(dma_tx_mutex);
        kfree(data);
        goto testEnd;
    }
    pr_info("%s : dma test find ring id %08x \n", WIRELESS_SIMU_DEVICE_NAME, tx_ring_id);

    // io测试：
    u32 val = 0;
    iowrite32(0x1919810, mmio_addr + WIRELESS_REG_TEST);
    val = ioread32(mmio_addr + WIRELESS_REG_TEST);
    pr_info("%s : read from 0x00 : %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    // io写host_buffer_id 用于返回时确定应该释放哪一个dma buffer
    iowrite32(tx_ring_id, mmio_addr + WIRELESS_REG_DMA_IN_BUFF_ID);
    val = ioread32(mmio_addr + WIRELESS_REG_DMA_IN_BUFF_ID);
    pr_info("%s : read from host host ring id reg %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    // io写地址 dma_addr
    iowrite32(dma_addr_low32, mmio_addr + WIRELESS_REG_DMA_IN_HOSTADDR);
    val = ioread32(mmio_addr + WIRELESS_REG_DMA_IN_HOSTADDR);
    pr_info("%s : read from host addr reg %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    // io写长度 data_length
    iowrite32(data_length, mmio_addr + WIRELESS_REG_DMA_IN_LENGTH);
    val = ioread32(mmio_addr + WIRELESS_REG_DMA_IN_LENGTH);
    pr_info("%s : read from host data length reg %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    // io写flag
    iowrite32(0x01, mmio_addr + WIRELESS_REG_DMA_IN_FLAG);

    // 启动DMA
    iowrite32(WIRELESS_EVENT_DMA, mmio_addr + WIRELESS_REG_EVENT);

    // 写 驱动的 tx 缓存list
    tx_list[tx_ring_id].data = data;
    tx_list[tx_ring_id].dma_addr = (u32)dma_addr;
    tx_list[tx_ring_id].data_length = data_length;
    tx_list[tx_ring_id].flag |= 0x01;
    mutex_unlock(dma_tx_mutex);

testEnd:
    wireless_simu_priv->mmio_addr = mmio_addr;
    if (test_msi_isenable)
    {
        wireless_simu_priv->msi_desc = msi_desc;
    }
    wireless_simu_priv->irq_vectors_num = num_vectors;
    pci_set_drvdata(pdev, wireless_simu_priv);
    pr_info("%s : pci device probe done \n", WIRELESS_SIMU_DEVICE_NAME);
    return 0;

test_err:
    pci_free_irq_vectors(pdev);
    pci_release_regions(pdev);
    pci_disable_device(pdev);
    kfree(wireless_simu_priv);
    return -1;
}

static void wireless_simu_remove(struct pci_dev *pdev)
{
    pr_info("%s : pci device remove start \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *wireless_simu_priv = pci_get_drvdata(pdev);
    if (wireless_simu_priv)
    {
        iowrite32(0x00, wireless_simu_priv->mmio_addr + WIRELESS_REG_EVENT);
        pci_iounmap(pdev, wireless_simu_priv->mmio_addr);
        pci_free_irq_vectors(pdev);
        pci_release_regions(pdev);
        pci_disable_device(pdev);
        kfree(wireless_simu_priv->tx_ring.tx_list);
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