#ifndef WIRELESS_SIMU
#define WIRELESS_SIMU

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
#include <net/cfg80211.h>
#include <linux/version.h>

#include <linux/skbuff.h>
#include <linux/mutex.h>
#include "wireless_mac80211.h"
#include "wireless_err.h"
#include "wireless_dma.h"
#include "wireless_dptx.h"
#include "wireless_dprx.h"
#include "wireless_wmi.h"
#include "wireless_htc.h"
#include "wireless_ce.h"
#include "wireless_hal.h"
#include "wireless_hif.h"
#include "wireless_irq.h"

#define WIRELESS_SIMU_DEVICE_NAME "wirelesssimu"
#define WIRELESS_SIMU_RX_POST_RETRY_JIFFIES 50
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
#define WIRELESS_REG_DMA_RX_RING_BUF_ID 0xB0
#define WIRELESS_REG_DMA_RX_RING_HOSTADDR 0xC0
#define WIRELESS_REG_DMA_RX_RING_LENGTH 0xD0
#define WIRELESS_REG_DMA_RX_RING_FLAG 0xE0
#define WIRELESS_REG_IRQ_ENABLE 0xF0

/* 硬件基础地址，基础寄存器的高16bit为全零 */
#define HAL_BASIC_BASE_REG 0x00000000
#define HAL_BASIC_REG(n) ((n << 2) & 0x0000ffff)
enum HAL_ENUM_REG_BASIC{
    WIRELESS_REG_BASIC_IRQ_ENABLE = 1,
    WIRELESS_REG_BASIC_IRQ_STATUS,
    
};

#define SETBIT(x, y) (x |= 1 << y)
#define CLBIT(x, y) (x &= ~(1 << y))

#define WIRELESS_BITCHECK(num, n) ((num >> n) & 1)
#define WIRELESS_BITSET(num, n) (num |= (1 << n))
#define WIRELESS_BITCLR(num, n) (num &= ~(1 << n))

#define WIRELESS_RX_RING_BUF_IS_USING 0
#define WIRELESS_RX_RING_BUF_INIT_END 1

#define WIRELESS_TX_RING_BUF_IS_USING 0

#define WIRELESS_MAX_NUM_VIF 4

enum Wireless_LongTimeEvent
{
    WIRELESS_EVENT_NOEVENT = 0,
    WIRELESS_EVENT_DMA,
    WIRELESS_EVENT_CLEAN_DMA,
    WIRELESS_EVENT_TEST,
};

// dev_flag 需要与设备端同步修改, 驱动中对应于unsigned long wireless_simu::dev_flag
enum wireless_simu_dev_flags{
    WIRELESS_SIMU_DEV_FLAG_CRASH_FLUSH,
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

/*
 * rx DMA 接收队列
 * 
 * 该队列需要在启动时直接申请一块较大的内存用于等待设备端填充数据，并把对应的内存地址传送给设备;
 * 接收队列长度固定，在退出时无需free;
 * */
struct Wireless_Rx_Ring
{
    struct Wireless_DMA_Buf rx_list[WIRELESS_RX_RING_SIZE];
    struct mutex rx_ring_mutex;
    u32 rx_list_size;
    u32 rx_list_head;
    u32 rx_list_tail;
};

#define WIRELESS_SIMU_TX_MGMT_NUM_PENDING_MAX 512 // mgmt tx queue max save data num
#define WIRELESS_SIMU_TX_MGMT_TARGET_MAX_SUPPORT 64
#define WIRELESS_SIMU_PRB_RSP_DROP_THRESHOLD ((WIRELESS_SIMU_TX_MGMT_TARGET_MAX_SUPPORT * 3) >> 2)

struct wireless_simu
{
    struct pci_dev *pci_dev;
    struct device dev;
    void __iomem *mmio_addr;
    u32 cdev_num;
    struct cdev *char_dev;
    struct class *dev_class;
    struct msi_desc *msi_desc;
    bool msi_enable;
    int irq_vectors_num;

    struct mutex iomutex;

    struct Wireless_Tx_Ring tx_ring;
    struct Wireless_Rx_Ring rx_ring;

    struct ieee80211_hw *hw;
    struct mutex mac_conf_mutex;
    unsigned int filter_flags;
    struct ieee80211_supported_band band_2GHZ;
    struct ieee80211_supported_band band_5GHZ;
    struct ieee80211_supported_band band_6GHZ;
    bool tx_interrupt_enable;
    bool rx_interrupt_enable;
    u8 mac_addr[ETH_ALEN];

    struct wireless_simu_hal hal;
    
    // tx_mgmt 会插入到这个workqueue中
    struct workqueue_struct *workqueue_aux;

    struct ieee80211_vif *vif[WIRELESS_MAX_NUM_VIF];
    // int vif_num;

    // mgmt tx 相关
    struct sk_buff_head mgmt_tx_queue;
    atomic_t num_pending_mgmt_tx;
    struct work_struct mgmt_tx_work;
    wait_queue_head_t txmgmt_empty_waitq;
    // mgmt id
    struct idr txmgmt_idr;
    // mgmt id 变化时加锁
    spinlock_t txmgmt_idr_lock;

    // ce 发送等待队列, 会被wmi层调用
    wait_queue_head_t tx_ce_desc_wq;

    // htc tx 时加锁
    spinlock_t htc_tx_lock;

    // device flag 反正最后只会初始化一个设备，写道这里也没差
    unsigned long dev_flag;

    // dst_srng_test
    struct srng_test st_dst;

    bool stop;
};

#endif