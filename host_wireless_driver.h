#ifndef HOST_WIRELESS_DRIVER_HEADER
#define HOST_WIRELESS_DRIVER_HEADER

#define NETLINK_SIMUWIFI 31
#define PAYLOAD_MAX_SIZE 1024 // 编译netlink_maxpayload可以得到当前系统最大支持的netlink长度
#define MAX_NUM_VIF 4

struct simuwifi_host_priv {
    struct platform_device *pdev;
    struct ieee80211_vif *vif[MAX_NUM_VIF];

    struct sock *netlink_sock;

};

#endif /* HOST_WIRELESS_DRIVER_HEADER */