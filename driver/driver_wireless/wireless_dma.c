#include "wireless_dma.h"

enum wireless_simu_err_code wireless_simu_tx_ring_init(struct wireless_simu *priv)
{
    struct Wireless_Tx_Ring *tx_ring = &priv->tx_ring;
    struct mutex *dma_tx_mutex = &tx_ring->tx_ring_mutex;
    enum wireless_simu_err_code ret = SUCCESS;
    tx_ring->tx_list_size = WIRELESS_TX_RING_SIZE;
    if (tx_ring->tx_list == NULL)
    {
        tx_ring->tx_list = (struct Wireless_DMA_Buf *)kzalloc(tx_ring->tx_list_size * sizeof(struct Wireless_DMA_Buf), GFP_KERNEL);
        if (tx_ring->tx_list == NULL)
        {
            pr_err("%s : tx ring malloc null \n", WIRELESS_SIMU_DEVICE_NAME);
            ret = TX_RING_INIT_MALLOC_ERR;
            goto END;
        }
    }
    mutex_init(dma_tx_mutex);
    // pr_info("%s : tx ring list init success \n", WIRELESS_SIMU_DEVICE_NAME);

END:
    return ret;
}
EXPORT_SYMBOL(wireless_simu_tx_ring_init);

enum wireless_simu_err_code wireless_simu_dma_tx(struct wireless_simu *priv, void *data, size_t length)
{
    struct Wireless_Tx_Ring *tx_ring = &priv->tx_ring;
    struct mutex *dma_tx_mutex = &tx_ring->tx_ring_mutex;
    struct Wireless_DMA_Buf *tx_list = tx_ring->tx_list;
    dma_addr_t dma_addr = 0;
    u32 tx_ring_id = 0;
    u32 val = 0;
    enum wireless_simu_err_code ret = SUCCESS;

    if (tx_list == NULL)
    {
        ret = wireless_simu_tx_ring_init(priv);
        if (ret)
        {
            goto END;
        }
    }

    mutex_lock(dma_tx_mutex);

    dma_addr = dma_map_single(&priv->pci_dev->dev, data, length, DMA_MEM_TO_DEV);
    if (dma_mapping_error(&priv->pci_dev->dev, dma_addr))
    {
        pr_err("%s : dma_addr err \n", WIRELESS_SIMU_DEVICE_NAME);
        goto END;
    }

    for (tx_ring_id = 0; tx_ring_id < tx_ring->tx_list_size; tx_ring_id++)
    {
        if (!WIRELESS_BITCHECK(tx_list[tx_ring_id].flag, WIRELESS_TX_RING_BUF_IS_USING))
        {
            break;
        }
    }
    if (tx_ring_id == tx_ring->tx_list_size)
    {
        pr_err("%s : no tx ring empty for data \n", WIRELESS_SIMU_DEVICE_NAME);
        ret = TX_RING_FULL; // 需要查阅Linux的网络数据缓存的设计，以确定在网卡发送速率慢的情况下数据被缓存在了哪里；
        goto FULL_END;
    }
    WIRELESS_BITSET(tx_list[tx_ring_id].flag, WIRELESS_TX_RING_BUF_IS_USING);

    // io写host_buffer_id 用于返回时确定应该释放哪一个dma buffer
    iowrite32(tx_ring_id, priv->mmio_addr + WIRELESS_REG_DMA_IN_BUFF_ID);
    val = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_IN_BUFF_ID);
    // pr_info("%s : read from host host ring id reg %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    // io写地址 dma_addr
    iowrite32((u32)dma_addr, priv->mmio_addr + WIRELESS_REG_DMA_IN_HOSTADDR);
    val = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_IN_HOSTADDR);
    tx_list[tx_ring_id].dma_addr = val;
    // pr_info("%s : read from host addr reg %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    // io写长度 data_length
    iowrite32(length, priv->mmio_addr + WIRELESS_REG_DMA_IN_LENGTH);
    val = ioread32(priv->mmio_addr + WIRELESS_REG_DMA_IN_LENGTH);
    tx_list[tx_ring_id].data_length = val;
    // pr_info("%s : read from host data length reg %08x \n", WIRELESS_SIMU_DEVICE_NAME, val);

    // io写flag
    iowrite32(tx_list[tx_ring_id].flag, priv->mmio_addr + WIRELESS_REG_DMA_IN_FLAG);

    // 启动DMA
    iowrite32(WIRELESS_EVENT_DMA, priv->mmio_addr + WIRELESS_REG_EVENT);

    // 写 驱动的 tx 缓存list
    tx_list[tx_ring_id].data = data;
    mutex_unlock(dma_tx_mutex);
    return SUCCESS;

FULL_END:
    dma_unmap_single(&priv->pci_dev->dev, dma_addr, length, DMA_MEM_TO_DEV);
END:
    mutex_unlock(dma_tx_mutex);
    return ret;
}
EXPORT_SYMBOL(wireless_simu_dma_tx);