#ifndef WIRELESS_SIMU
#define WIRELESS_SIMU

struct Wireless_DMA_Buf
{
    u32 dma_buf_id;
    u32 flag;
    void *data;
    size_t data_length;
};

/*
 * tx DMA 发送缓冲队列
 *
 * 将发送至device的数据描述符存放在这个队列中, device确认完整的收到数据后再释放
 *
 * 实际上并非一定要使用环形的结构，队列并不长，每次存放时遍历一遍即可，
 * 先进先出的环形结构反而会因为确认速度不一致出问题;
 * 对ring进行修改时需要加互斥锁来保证一致性；
 */
struct Wireless_Tx_Ring
{
    struct Wireless_DMA_Buf *tx_list;
    struct mutex tx_ring_mutex;
    u32 tx_list_size;
    u32 tx_list_head;
    u32 tx_list_tail;
};

struct wireless_simu
{
    struct pci_dev *pci_dev;
    void __iomem *mmio_addr;
    u32 cdev_num;
    struct cdev *char_dev;
    struct class *dev_class;
    struct msi_desc *msi_desc;
    int irq_vectors_num;

    struct Wireless_Tx_Ring tx_ring;
};

#endif