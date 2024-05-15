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

// 驱动私有结构体
struct simuwifi_host_priv
{
    struct platform_device *pdev;
    struct ieee80211_vif *vif[MAX_NUM_VIF];

    struct sock *netlink_sock;
    __u32 group;

    u8 mac_addr[ETH_ALEN];

    // 物理层支持的频率
    struct ieee80211_rate           rates_2GHz[12];
	struct ieee80211_rate           rates_5GHz[12];
	struct ieee80211_channel        channels_2GHz[13];
	struct ieee80211_channel        channels_5GHz[11];
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