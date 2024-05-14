#include <linux/platform_device.h>
#include <linux/module.h>

#include <net/mac80211.h> // mac80211 head files in net/
#include "host_wireless_driver.h"

#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>

#define HOST_WIFI_NAME "simulation_wifi"
static char *simuwifi_dev_name = HOST_WIFI_NAME;

static const struct ieee80211_ops simuwifi_ops = {

};

static void simuwifi_netlink_rx(struct sk_buff *skb){

}

static int simuwifi_netlink_init(struct simuwifi_host_priv *priv){
    struct sock *netlink_socket_temp = NULL;
    struct netlink_kernel_cfg cfg = {
        .input = simuwifi_netlink_rx
    };
    netlink_socket_temp = (struct sock *)netlink_kernel_create(&init_net, NETLINK_SIMUWIFI, &cfg);
    if(netlink_socket_temp == NULL){
        printk(KERN_ERR "%s : netlink init err \n", simuwifi_dev_name);
        return -1;
    }
    priv->netlink_sock = netlink_socket_temp;
    printk(KERN_INFO "%s : netlink init success addr : %p \n", simuwifi_dev_name, (void*)(priv->netlink_sock));
    return 0;
}

static int simuwifi_dev_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "%s : simuwifi_dev_probe start \n" , simuwifi_dev_name);
    struct ieee80211_hw *dev;
    struct simuwifi_host_priv *priv;

    dev = ieee80211_alloc_hw(sizeof(*priv), &simuwifi_ops);
    if(!dev) {
        printk(KERN_ERR "%s : simuwifi probe : ieee80211 alloc failed \n", simuwifi_dev_name);
        goto err_free_dev;
    }
    priv = dev->priv;
    priv->pdev = pdev;

    if(simuwifi_netlink_init(priv))
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
    printk(KERN_INFO "%s : simuwifi_dev_remove start \n" , simuwifi_dev_name);

    struct ieee80211_hw *dev = platform_get_drvdata(pdev);
	struct openwifi_priv *priv = dev->priv;
    
    if(!dev){
        
    }
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