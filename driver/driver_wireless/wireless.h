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

#include <linux/skbuff.h>
#include <linux/mutex.h>
#include "wireless_mac80211.h"
#include "wireless_err.h"
#include "wireless_dma.h"

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
#define WIRELESS_REG_DMA_RX_RING_BUF_ID 0xB0
#define WIRELESS_REG_DMA_RX_RING_HOSTADDR 0xC0
#define WIRELESS_REG_DMA_RX_RING_LENGTH 0xD0
#define WIRELESS_REG_DMA_RX_RING_FLAG 0xE0
#define WIRELESS_REG_IRQ_ENABLE 0xF0

#define SETBIT(x, y) (x |= 1 << y)
#define CLBIT(x, y) (x &= ~(1 << y))

#define WIRELESS_BITCHECK(num, n) ((num >> n) & 1)
#define WIRELESS_BITSET(num, n) (num |= (1 << n))
#define WIRELESS_BITCLR(num, n) (num &= ~(1 << n))

#define WIRELESS_RX_RING_BUF_IS_USING 0
#define WIRELESS_RX_RING_BUF_INIT_END 1

#define WIRELESS_TX_RING_BUF_IS_USING 0

enum Wireless_DMA_IRQ_STATUS
{
    WIRELESS_IRQ_TEST = 0,
    WIRELESS_IRQ_DNA_MEM_TO_DEVICE_END,
    WIRELESS_IRQ_DMA_DELALL_END,
    WIRELESS_IRQ_RX_START,
    WIRELESS_IRQ_DMA_DEVICE_TO_MEM_END,
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

struct wireless_simu
{
    struct pci_dev *pci_dev;
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
    struct ieee80211_supported_band band_2GHZ;
    struct ieee80211_supported_band band_5GHZ;
    struct ieee80211_supported_band band_6GHZ;
    u8 mac_addr[ETH_ALEN];

    bool stop;
};

#define CHAN2G(_channel, _freq, _flags) { \
	.band                   = NL80211_BAND_2GHZ, \
	.hw_value               = (_channel), \
	.center_freq            = (_freq), \
	.flags                  = (_flags), \
	.max_antenna_gain       = 0, \
	.max_power              = 30, \
}

#define CHAN5G(_channel, _freq, _flags) { \
	.band                   = NL80211_BAND_5GHZ, \
	.hw_value               = (_channel), \
	.center_freq            = (_freq), \
	.flags                  = (_flags), \
	.max_antenna_gain       = 0, \
	.max_power              = 30, \
}

#define CHAN6G(_channel, _freq, _flags) { \
	.band                   = NL80211_BAND_6GHZ, \
	.hw_value               = (_channel), \
	.center_freq            = (_freq), \
	.flags                  = (_flags), \
	.max_antenna_gain       = 0, \
	.max_power              = 30, \
}

static const struct ieee80211_channel wireless_simu_2ghz_channels[] = {
    CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

static const struct ieee80211_channel wireless_simu_5ghz_channels[] = {
    CHAN5G(36, 5180, 0),
	CHAN5G(40, 5200, 0),
	CHAN5G(44, 5220, 0),
	CHAN5G(48, 5240, 0),
	CHAN5G(52, 5260, 0),
	CHAN5G(56, 5280, 0),
	CHAN5G(60, 5300, 0),
	CHAN5G(64, 5320, 0),
	CHAN5G(100, 5500, 0),
	CHAN5G(104, 5520, 0),
	CHAN5G(108, 5540, 0),
	CHAN5G(112, 5560, 0),
	CHAN5G(116, 5580, 0),
	CHAN5G(120, 5600, 0),
	CHAN5G(124, 5620, 0),
	CHAN5G(128, 5640, 0),
	CHAN5G(132, 5660, 0),
	CHAN5G(136, 5680, 0),
	CHAN5G(140, 5700, 0),
	CHAN5G(144, 5720, 0),
	CHAN5G(149, 5745, 0),
	CHAN5G(153, 5765, 0),
	CHAN5G(157, 5785, 0),
	CHAN5G(161, 5805, 0),
	CHAN5G(165, 5825, 0),
	CHAN5G(169, 5845, 0),
	CHAN5G(173, 5865, 0),
	CHAN5G(177, 5885, 0),
};

static const struct ieee80211_channel wireless_simu_6ghz_channels[] = {
    CHAN6G(1, 5955, 0),
	CHAN6G(5, 5975, 0),
	CHAN6G(9, 5995, 0),
	CHAN6G(13, 6015, 0),
	CHAN6G(17, 6035, 0),
	CHAN6G(21, 6055, 0),
	CHAN6G(25, 6075, 0),
	CHAN6G(29, 6095, 0),
	CHAN6G(33, 6115, 0),
	CHAN6G(37, 6135, 0),
	CHAN6G(41, 6155, 0),
	CHAN6G(45, 6175, 0),
	CHAN6G(49, 6195, 0),
	CHAN6G(53, 6215, 0),
	CHAN6G(57, 6235, 0),
	CHAN6G(61, 6255, 0),
	CHAN6G(65, 6275, 0),
	CHAN6G(69, 6295, 0),
	CHAN6G(73, 6315, 0),
	CHAN6G(77, 6335, 0),
	CHAN6G(81, 6355, 0),
	CHAN6G(85, 6375, 0),
	CHAN6G(89, 6395, 0),
	CHAN6G(93, 6415, 0),
	CHAN6G(97, 6435, 0),
	CHAN6G(101, 6455, 0),
	CHAN6G(105, 6475, 0),
	CHAN6G(109, 6495, 0),
	CHAN6G(113, 6515, 0),
	CHAN6G(117, 6535, 0),
	CHAN6G(121, 6555, 0),
	CHAN6G(125, 6575, 0),
	CHAN6G(129, 6595, 0),
	CHAN6G(133, 6615, 0),
	CHAN6G(137, 6635, 0),
	CHAN6G(141, 6655, 0),
	CHAN6G(145, 6675, 0),
	CHAN6G(149, 6695, 0),
	CHAN6G(153, 6715, 0),
	CHAN6G(157, 6735, 0),
	CHAN6G(161, 6755, 0),
	CHAN6G(165, 6775, 0),
	CHAN6G(169, 6795, 0),
	CHAN6G(173, 6815, 0),
	CHAN6G(177, 6835, 0),
	CHAN6G(181, 6855, 0),
	CHAN6G(185, 6875, 0),
	CHAN6G(189, 6895, 0),
	CHAN6G(193, 6915, 0),
	CHAN6G(197, 6935, 0),
	CHAN6G(201, 6955, 0),
	CHAN6G(205, 6975, 0),
	CHAN6G(209, 6995, 0),
	CHAN6G(213, 7015, 0),
	CHAN6G(217, 7035, 0),
	CHAN6G(221, 7055, 0),
	CHAN6G(225, 7075, 0),
	CHAN6G(229, 7095, 0),
	CHAN6G(233, 7115, 0),

	/* new addition in IEEE Std 802.11ax-2021 */
	CHAN6G(2, 5935, 0),
};

/* rate 参考 wireless_simu 的驱动代码, 不对不同的频率进行区分*/

enum wireless_simu_hw_rate_cck {
	WIRELESS_SIMU_HW_RATE_CCK_LP_11M = 0,
	WIRELESS_SIMU_HW_RATE_CCK_LP_5_5M,
	WIRELESS_SIMU_HW_RATE_CCK_LP_2M,
	WIRELESS_SIMU_HW_RATE_CCK_LP_1M,
	WIRELESS_SIMU_HW_RATE_CCK_SP_11M,
	WIRELESS_SIMU_HW_RATE_CCK_SP_5_5M,
	WIRELESS_SIMU_HW_RATE_CCK_SP_2M,
};

enum wireless_simu_hw_rate_ofdm {
	WIRELESS_SIMU_HW_RATE_OFDM_48M = 0,
	WIRELESS_SIMU_HW_RATE_OFDM_24M,
	WIRELESS_SIMU_HW_RATE_OFDM_12M,
	WIRELESS_SIMU_HW_RATE_OFDM_6M,
	WIRELESS_SIMU_HW_RATE_OFDM_54M,
	WIRELESS_SIMU_HW_RATE_OFDM_36M,
	WIRELESS_SIMU_HW_RATE_OFDM_18M,
	WIRELESS_SIMU_HW_RATE_OFDM_9M,
};

static const struct ieee80211_rate wireless_simu_rates[] = {
    { .bitrate = 10,
	  .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_1M },
	{ .bitrate = 20,
	  .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_2M,
	  .hw_value_short = WIRELESS_SIMU_HW_RATE_CCK_SP_2M,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 55,
	  .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_5_5M,
	  .hw_value_short = WIRELESS_SIMU_HW_RATE_CCK_SP_5_5M,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },
	{ .bitrate = 110,
	  .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_11M,
	  .hw_value_short = WIRELESS_SIMU_HW_RATE_CCK_SP_11M,
	  .flags = IEEE80211_RATE_SHORT_PREAMBLE },

	{ .bitrate = 60, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_6M },
	{ .bitrate = 90, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_9M },
	{ .bitrate = 120, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_12M },
	{ .bitrate = 180, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_18M },
	{ .bitrate = 240, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_24M },
	{ .bitrate = 360, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_36M },
	{ .bitrate = 480, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_48M },
	{ .bitrate = 540, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_54M },
};

#define WIRELESS_SIMU_MAC_FIRST_OFDM_RATE_IDX 4
#define wireless_simu_g_rates wireless_simu_legacy_rates
#define wireless_simu_g_rates_size (ARRAY_SIZE(wireless_simu_legacy_rates))
#define wireless_simu_a_rates (wireless_simu_legacy_rates + WIRELESS_SIMU_MAC_FIRST_OFDM_RATE_IDX)
#define wireless_simu_a_rates_size (ARRAY_SIZE(wireless_simu_legacy_rates) - WIRELESS_SIMU_MAC_FIRST_OFDM_RATE_IDX)

#endif