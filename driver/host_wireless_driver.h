#ifndef HOST_WIRELESS_DRIVER_HEADER
#define HOST_WIRELESS_DRIVER_HEADER

#define NETLINK_SIMUWIFI 31
#define PAYLOAD_MAX_SIZE 1024 // 编译netlink_maxpayload可以得到当前系统最大支持的netlink长度
#define NETLINK_SIMUWIFI_GROUP 9
#define MAX_NUM_VIF 4

// 设备支持的过滤规则
#define SIMUWIFI_SUPPORTED_FILTERS \
    (FIF_ALLMULTI |                \
     FIF_BCN_PRBRESP_PROMISC |     \
     FIF_CONTROL |                 \
     FIF_OTHER_BSS |               \
     FIF_PSPOLL |                  \
     FIF_PROBE_REQ)

#define RATES_2GHZ_NUM 12
static const struct ieee80211_rate simuwifi_2GHz_rates[] = {
    { .bitrate = 10,  .hw_value = 0,  .flags = 0},
	{ .bitrate = 20,  .hw_value = 1,  .flags = 0},
	{ .bitrate = 55,  .hw_value = 2,  .flags = 0},
	{ .bitrate = 110, .hw_value = 3,  .flags = 0},
	{ .bitrate = 60,  .hw_value = 4,  .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
	{ .bitrate = 90,  .hw_value = 5,  .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
	{ .bitrate = 120, .hw_value = 6,  .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
	{ .bitrate = 180, .hw_value = 7,  .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
	{ .bitrate = 240, .hw_value = 8,  .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
	{ .bitrate = 360, .hw_value = 9,  .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
	{ .bitrate = 480, .hw_value = 10, .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
	{ .bitrate = 540, .hw_value = 11, .flags = IEEE80211_RATE_MANDATORY_G|IEEE80211_RATE_ERP_G},
};

#define RATES_5GHZ_NUM 12
static const struct ieee80211_rate simuwifi_5GHz_rates[] = {
    
};

// 驱动私有结构体
struct simuwifi_host_priv
{
    struct platform_device *pdev;
    struct ieee80211_vif *vif[MAX_NUM_VIF];

    struct sock *netlink_sock;
    __u32 group;

    u8 mac_addr[ETH_ALEN];

    // 物理层支持
    struct ieee80211_rate rates_2GHz[RATES_2GHZ_NUM];
    struct ieee80211_rate rates_5GHz[12];
    struct ieee80211_channel channels_2GHz[13];
    struct ieee80211_channel channels_5GHz[11];
    struct ieee80211_supported_band band_2GHz;
    struct ieee80211_supported_band band_5GHz;
};

// 标识每个处理过的skb
struct simuwifi_skb_info
{
    __u16 seq_no;
};

// 标识新建的每个interface
// 存放于ieee80211_vif私有数据区域的结构体，使ieee80211_vif和simuwifi_host_priv对应
struct simuwifi_vif
{
    struct ieee80211_hw *dev;
    int idx; // 与simuwifi_host_priv中的vif数组中存放的接口位置相对应
};

#endif /* HOST_WIRELESS_DRIVER_HEADER */