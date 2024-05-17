#include <linux/platform_device.h>
#include <linux/module.h>

#include <net/mac80211.h> // mac80211 head files in net/
#include "host_wireless_driver.h"

#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>

#define HOST_WIFI_NAME "simulation_wifi"
static char *simuwifi_dev_name = HOST_WIFI_NAME;

static int simuwifi_netlink_tx_brodcast(struct sock *netlink_socket, void *data, size_t data_size, __u32 group)
{
    printk(KERN_INFO "%s : start netlink tx \n", simuwifi_dev_name);
    if (group <= 0)
        group = NETLINK_SIMUWIFI_GROUP;

    struct sk_buff *skb_out;
    skb_out = nlmsg_new(data_size, GFP_ATOMIC);
    if (!skb_out)
    {
        printk(KERN_ERR "%s : Failed to allocate a new nl_sk_buff\n", simuwifi_dev_name);
        return -1;
    }

    struct nlmsghdr *nlh;
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, data_size, 0);

    memcpy(nlmsg_data(nlh), data, data_size);

    // NETLINK_CB(skb_out).pid = 0;
    NETLINK_CB(skb_out).portid = 0;
    NETLINK_CB(skb_out).dst_group = group;
    int ret = netlink_broadcast(netlink_socket, skb_out, 0, group, GFP_ATOMIC);
    if (ret < 0)
    {
        printk(KERN_ERR "%s : Error in sending msg to userspace %d \n", simuwifi_dev_name, ret);
        return ret;
    }

    return 0;
}

static void simuwifi_tx(struct ieee80211_hw *dev,
                        struct ieee80211_tx_contril *control,
                        struct sk_buff *skb)
{
    printk(KERN_INFO "%s : simuwifi tx start \n", simuwifi_dev_name);
    struct simuwifi_host_priv *priv = dev->priv;
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;

    printk(KERN_INFO "%s : skb->data addr %p , skb->len %d , skb : tail - data %d \n", simuwifi_dev_name, (void *)skb->data, skb->len, skb->tail - skb->data)
        simuwifi_netlink_tx_brodcast(priv->netlink_sock, skb->data, skb->len, priv->group);
}

static int simuwifi_start(struct ieee80211_hw *dev)
{
    printk(KERN_INFO "%s : wifi start %p \n", simuwifi_dev_name, (void *)dev);

    return 0;
}

static int simuwifi_stop(struct ieee80211_hw *dev)
{
    printk(KERN_INFO "%s : wifi end %p \n", simuwifi_dev_name, (void *)dev);

    return 0;
}

static int simuwifi_config(struct ieee80211_hw *dev, u32 changed)
{
    printk(KERN_INFO "%s : simuwifi config changed %08x \n", simuwifi_dev_name, changed);
    struct ieee80211_conf *conf = &dev->conf;
    struct simuwifi_host_priv *priv = dev->priv;
}

static int simuwifi_add_interface(struct ieee80211_hw *dev,
                                  struct ieee80211_vif *vif)
{
    printk(KERN_INFO "%s : simuwifi add interface %d \n", simuwifi_dev_name, vif->type);
    struct simuwifi_host_priv *priv = dev->priv;

    switch (vif->type)
    {
    case NL80211_IFTYPE_AP:
    case NL80211_IFTYPE_STATION:
    case NL80211_IFTYPE_ADHOC:
    case NL80211_IFTYPE_MONITOR:
    case NL80211_IFTYPE_MESH_POINT:
        break;
    default:
        return -EOPNOTSUPP;
    }

    int i = 0;
    for (i = 0; i < MAX_NUM_VIF; i++)
        if (priv->vif[i] == NULL)
            break;

    if (i == MAX_NUM_VIF)
        return -EBUSY;

    priv->vif[i] = vif;
    priv->mac_addr[0] = vif->addr[0];
    priv->mac_addr[1] = vif->addr[1];
    priv->mac_addr[2] = vif->addr[2];
    priv->mac_addr[3] = vif->addr[3];
    priv->mac_addr[4] = vif->addr[4];
    priv->mac_addr[5] = vif->addr[5];

    printk(KERN_INFO "%s : simuwifi add interface addr : %02x:%02x:%02x:%02x:%02x:%02x \n", simuwifi_dev_name,
           vif->addr[0], vif->addr[1], vif->addr[2], vif->addr[3], vif->addr[4], vif->addr[5]);

    return 0;
}

static void simuwifi_remove_interface(struct ieee80211_hw *dev, struct ieee80211_vif *vif)
{
    struct simuwifi_host_priv *priv = dev->priv;
    struct simuwifi_vif *vif_priv = (struct simuwifi_vif *)&vif->drv_priv;

    priv->vif[vif_priv->idx] = NULL;
    printk(KERN_INFO "%s : simuwifi remove interface id : %d \n", simuwifi_dev_name, vif_priv->idx);
}

// 配置硬件的帧过滤器
// 比如在monitor模式下运行的硬件，就要提交所有的帧到kernel中
static void simuwifi_configure_filter(struct ieee80211_hw *dev,
                                      unsigned int changed_flags,
                                      unsigned int *total_flags,
                                      u64 multicast)
{
    printk(KERN_INFO "%s : simuwifi_configure_filter %08x \n", simuwifi_dev_name, *total_flags);

    struct simuwifi_host_priv *priv = dev->priv;
    u32 filter_flag; // 将total_flag转化为硬件接受的filter_flag

    (*total_flags) &= SIMUWIFI_SUPPORTED_FILTERS;
	(*total_flags) |= FIF_ALLMULTI; //because we need to pass all multicast (no matter it is for us or not) to upper layer

	filter_flag = (*total_flags);

	// 对filter_flag进行转化，没有硬件设计说明书，这里就不写了
}

static const struct ieee80211_ops simuwifi_ops = {
    .start = simuwifi_start,
    .stop = simuwifi_stop,
    .tx = simuwifi_tx,
    .config = simuwifi_config,
    .add_interface = simuwifi_add_interface,
    .remove_interface = simuwifi_remove_interface,
    .configure_filter = simuwifi_configure_filter
};

static void simuwifi_netlink_rx(struct sk_buff *skb)
{
}

static int simuwifi_netlink_init(struct simuwifi_host_priv *priv)
{
    struct sock *netlink_socket_temp = NULL;
    struct netlink_kernel_cfg cfg = {
        .input = simuwifi_netlink_rx};
    netlink_socket_temp = (struct sock *)netlink_kernel_create(&init_net, NETLINK_SIMUWIFI, &cfg);
    if (netlink_socket_temp == NULL)
    {
        printk(KERN_ERR "%s : netlink init err \n", simuwifi_dev_name);
        return -1;
    }
    priv->netlink_sock = netlink_socket_temp;
    priv->group = NETLINK_SIMUWIFI_GROUP;
    printk(KERN_INFO "%s : netlink init success addr : %p \n", simuwifi_dev_name, (void *)(priv->netlink_sock));
    return 0;
}

static int simuwifi_dev_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "%s : simuwifi_dev_probe start \n", simuwifi_dev_name);
    struct ieee80211_hw *dev;
    struct simuwifi_host_priv *priv;

    // 设备树结点匹配检查

    // 为ieee设备分配内存
    dev = ieee80211_alloc_hw(sizeof(*priv), &simuwifi_ops);
    if (!dev)
    {
        printk(KERN_ERR "%s : simuwifi probe : ieee80211 alloc failed \n", simuwifi_dev_name);
        goto err_free_dev;
    }
    priv = dev->priv;
    priv->pdev = pdev;

    if (simuwifi_netlink_init(priv))
        goto err_free_dev;

    SET_IEEE80211_DEV(dev, &pdev->dev);
    platform_set_drvdata(pdev, dev);
    
    
    
    
    
    return 0;

err_free_dev:
    ieee80211_free_hw(dev);
    printk(KERN_ERR "%s : probe error \n", simuwifi_dev_name);
    return 1;
}

static int simuwifi_dev_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "%s : simuwifi_dev_remove start \n", simuwifi_dev_name);

    struct ieee80211_hw *dev = platform_get_drvdata(pdev);
    struct simuwifi_host_priv *priv = dev->priv;

    if (!dev)
    {
        printk(KERN_INFO "%s : simuwifi dev remove : addr %p \n", simuwifi_dev_name, (void *)dev);
        return -1;
    }

    if (priv->netlink_sock)
    {
        netlink_kernel_release(priv->netlink_sock);
        priv->netlink_sock = NULL;
    }
    printk(KERN_INFO "%s : netlink socket remove success \n", simuwifi_dev_name);

    ieee80211_unregister_hw(dev);
    ieee80211_free_hw(dev);
    printk(KERN_INFO "%s : ieee80211_hw remove success \n", simuwifi_dev_name);
    return 0;
}

static struct platform_driver simuwifi_dev_driver = {
    .driver = {
        .name = HOST_WIFI_NAME,
        .owner = THIS_MODULE},
    .probe = simuwifi_dev_probe,
    .remove = simuwifi_dev_remove};

module_platform_driver(simuwifi_dev_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PolarisZg");
MODULE_DESCRIPTION("wifi simulation driver in host");