#ifndef WIRELESS_SIMU
#define WIRELESS_SIMU

enum Wireless_DMA_IRQ_STATUS
{
    WIRELESS_IRQ_TEST = 0,
    WIRELESS_IRQ_DNA_MEM_TO_DEVICE_END,
    WIRELESS_IRQ_DMA_DELALL_END,
    WIRELESS_IRQ_RX_START,
};

enum Wireless_LongTimeEvent
{
    WIRELESS_EVENT_NOEVENT = 0,
    WIRELESS_EVENT_DMA,
    WIRELESS_EVENT_CLEAN_DMA,
    WIRELESS_EVENT_TEST,
};

/*
 * @flag:   0bit 是否被占用
 *              0 未被占用
 *              1 被占用
 */
struct Wireless_DMA_Buf
{
    dma_addr_t dma_addr;
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
 * 对ring进行修改时需要加互斥锁来保证一致性;
 *
 * @tx_list: 已发送至设备的DMA信息缓存队列, 等待设备DMA完成后释放
 * @tx_ring_mutex: 访问DMA信息缓存队列锁, 保证缓存队列访问的唯一性
 * @tx_list_size: DMA缓存队列长度
 * @tx_list_head: DMA缓存队列已占用部分的头部, 暂时无效, 在缓存队列不长的情况下只需要遍历队列即可
 * @tx_list_tail: DMA缓存队列已占用部分的尾部, 暂时无效, 在缓存队列不长的情况下只需要遍历队列即可
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