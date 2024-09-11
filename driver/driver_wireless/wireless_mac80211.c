#include "wireless_mac80211.h"

static void wireless_mac80211_tx(struct ieee80211_hw *dev,
                                 struct ieee80211_tx_control *control,
                                 struct sk_buff *skb)
{
}

static int wireless_mac80211_start(struct ieee80211_hw *hw)
{
    pr_info("%s : mac80211 start \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *priv = hw->priv;

    // 清空vif
    for(int i = 0; i < WIRELESS_MAX_NUM_VIF; i++){
        priv->vif[i] = NULL;
    }

    // 打开天线

    // 申请dma通道和ring，但是这个已经在pci中做过了

    // 申请中断，注册rx和tx complete, 这个已经在pci中申请过中断
    priv->tx_interrupt_enable = true;
    priv->rx_interrupt_enable = true;

    pr_info("%s : mac80211 start done \n", WIRELESS_SIMU_DEVICE_NAME);
    return 0;
}

static void wireless_mac80211_stop(struct ieee80211_hw *hw)
{
    pr_info("%s : mac80211 stop \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *priv = hw->priv;

    // 关闭天线

    // 停掉队列

    // 关闭driver中的vif
    for(int i = 0; i < WIRELESS_MAX_NUM_VIF; i++){
        priv->vif[i] = NULL;
    }

    // 关闭中断
    priv->tx_interrupt_enable = false;
    priv->rx_interrupt_enable = false;

    pr_info("%s : mac80211 stop done \n", WIRELESS_SIMU_DEVICE_NAME);
}

static int wireless_mac80211_add_interface(struct ieee80211_hw *hw,
                                           struct ieee80211_vif *vif)
{
    return 0;
}

static void wireless_mac80211_remove_interface(struct ieee80211_hw *hw,
                                               struct ieee80211_vif *vif)
{
}

static int wireless_mac80211_config(struct ieee80211_hw *hw,
                                    u32 changed)
{
    return 0;
}

static void wireless_mac80211_configure_filter(struct ieee80211_hw *hw,
                                               unsigned int changed_flags,
                                               unsigned int *total_flags,
                                               u64 multicast)
{
}

static void wireless_mac80211_bss_info_changed(struct ieee80211_hw *hw,
                                               struct ieee80211_vif *vif,
                                               struct ieee80211_bss_conf *info,
                                               u64 changed)
{
}

static int wireless_mac80211_conf_tx(struct ieee80211_hw *hw,
                                     struct ieee80211_vif *vif,
                                     unsigned int link_id, u16 ac,
                                     const struct ieee80211_tx_queue_params *params)
{
    return 0;
}

static int wireless_mac80211_set_antenna(struct ieee80211_hw *hw,
                                         u32 tx_ant,
                                         u32 rx_ant)
{
    return 0;
}

static int wireless_mac80211_get_antenna(struct ieee80211_hw *hw,
                                         u32 *tx_ant,
                                         u32 *rx_ant)
{
    return 0;
}

static int wireless_mac80211_ampdu_action(struct ieee80211_hw *hw,
                                          struct ieee80211_vif *vif,
                                          struct ieee80211_ampdu_params *params)
{
    return 0;
}

static int wireless_mac80211_set_rts_threshold(struct ieee80211_hw *hw,
                                               u32 value)
{
    return 0;
}

static int wireless_mac80211_testmode_cmd(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
                                          void *data, int len)
{
    return 0;
}

static u64 wireless_mac80211_prepare_multicast(struct ieee80211_hw *hw,
                                               struct netdev_hw_addr_list *mc_list)
{
    return 0;
}

static void wireless_mac80211_rfkill_poll(struct ieee80211_hw *hw)
{
}

static u64 wireless_mac80211_get_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
    return 0;
}

static void wireless_mac80211_set_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
                                      u64 tsf)
{
}

static void wireless_mac80211_reset_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
}

static const struct ieee80211_ops wireless_mac80211_ops = {
    // 该部分必须完成, 否则无法申请ieee80211_hw结构体内存
    .tx = wireless_mac80211_tx,
    .start = wireless_mac80211_start,
    .stop = wireless_mac80211_stop,
    .config = wireless_mac80211_config,
    .add_interface = wireless_mac80211_add_interface,
    .remove_interface = wireless_mac80211_remove_interface,
    .configure_filter = wireless_mac80211_configure_filter,
    .wake_tx_queue = ieee80211_handle_wake_tx_queue, // 这个是最新版linux新添加的强制性接口

    // 该部分可自行选择满足, 参考ath11k, 保留的参考openwifi
    // .reconfig_complete = ,
    // .update_vif_offload = ,
    .bss_info_changed = wireless_mac80211_bss_info_changed,
    // .hw_scan = ,
    // .cancel_hw_scan = ,
    // .set_key = ,
    // .set_rekey_data = ,
    // .sta_state = ,
    // .sta_set_4addr = ,
    // .sta_set_txpwr = ,
    // .sta_rc_update = ,
    .conf_tx = wireless_mac80211_conf_tx,
    .set_antenna = wireless_mac80211_set_antenna,
    .get_antenna = wireless_mac80211_get_antenna,
    .ampdu_action = wireless_mac80211_ampdu_action,
    // .add_chanctx = ,
    // .remove_chanctx = ,
    // .change_chanctx = ,
    // .assign_vif_chanctx = ,
    // .unassign_vif_chanctx = ,
    // .switch_vif_chanctx = ,
    .set_rts_threshold = wireless_mac80211_set_rts_threshold,
    // .set_frag_threshold = ,
    // .set_bitrate_mask = ,
    // .get_survey = ,
    // .flush = ,
    // .sta_statistics = ,
    CFG80211_TESTMODE_CMD(wireless_mac80211_testmode_cmd)
        // #if IS_ENABLED(CONFIG_IPV6)
        // .ipv6_addr_change = ,
        // #endif
        // .get_txpower = ,
        // .set_sar_specs = ,
        // .remain_on_channel = ,
        // .cancel_remain_on_channel = ,

        // 下面为openwifi中实现但ath11k中没有的，可能是因为softmac和full mac的不同吧
        .prepare_multicast = wireless_mac80211_prepare_multicast,
    .rfkill_poll = wireless_mac80211_rfkill_poll,
    .get_tsf = wireless_mac80211_get_tsf,
    .set_tsf = wireless_mac80211_set_tsf,
    .reset_tsf = wireless_mac80211_reset_tsf,
};

enum wireless_simu_err_code
wireless_mac80211_core_probe(struct wireless_simu *priv)
{
    pr_info("%s : mac80211 core probe start \n", WIRELESS_SIMU_DEVICE_NAME);
    // 该方法中对mac80211系统中api的调用顺序尽量不要打乱

    struct ieee80211_hw *hw = NULL;
    enum wireless_simu_err_code err = SUCCESS;

    hw = ieee80211_alloc_hw(sizeof(*priv), &wireless_mac80211_ops);
    if (!hw)
    {
        err = MAC80211_ALLOC_ERR;
        pr_info("%s : mac probe : alloc hw failed \n", WIRELESS_SIMU_DEVICE_NAME);
        goto err_free_dev;
    }
    hw->priv = priv;
    priv->hw = hw;

    SET_IEEE80211_DEV(hw, &priv->pci_dev->dev);

    // priv->hw->max_rates = 1; // 最大速率重试次数，感觉没什么用

    /* set channel rates
     * ath11k_mac_setup_channels_rates in ath11k driver
     */
    void *channels;
    channels = kmemdup(wireless_simu_2ghz_channels, sizeof(wireless_simu_2ghz_channels), GFP_KERNEL);
    if (!channels)
    {
        return MAC80211_ALLOC_ERR;
    }
    priv->band_2GHZ.band = NL80211_BAND_2GHZ;
    priv->band_2GHZ.channels = channels;
    priv->band_2GHZ.n_channels = ARRAY_SIZE(wireless_simu_2ghz_channels);
    priv->band_2GHZ.bitrates = wireless_simu_g_rates;
    priv->band_2GHZ.n_bitrates = wireless_simu_g_rates_size;
    priv->band_2GHZ.ht_cap.ht_supported = true;
    // 对AMPDU的支持也要填写到ht_cap中，缺省则不支持
    memset(&priv->band_2GHZ.ht_cap.mcs, 0, sizeof(priv->band_2GHZ.ht_cap.mcs));
    priv->band_2GHZ.ht_cap.mcs.rx_mask[0] = 0xff;
    priv->band_2GHZ.ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
    priv->hw->wiphy->bands[NL80211_BAND_2GHZ] = &priv->band_2GHZ;

    wiphy_read_of_freq_limits(priv->hw->wiphy);
    pr_info("%s : band 2ghz n_channel %d n_bitrates %d \n", WIRELESS_SIMU_DEVICE_NAME,
            priv->hw->wiphy->bands[NL80211_BAND_2GHZ]->n_channels,
            priv->hw->wiphy->bands[NL80211_BAND_2GHZ]->n_bitrates);

    /*
     * set hw_flags
     * enum ieee80211_hw_flags
     */
    // rx 的skb中包含fcs字段
    ieee80211_hw_set(priv->hw, RX_INCLUDES_FCS);
    ieee80211_hw_set(priv->hw, BEACON_TX_STATUS);
    ieee80211_hw_set(priv->hw, REPORTS_TX_ACK_STATUS);
    ieee80211_hw_set(priv->hw, AP_LINK_PS); // 让硬件管理PS，但硬件实际并不支持PS管理，所以就相当于禁用掉
    ieee80211_hw_set(priv->hw, SIGNAL_DBM);

    // ieee80211_hw_set(priv->hw, AMPDU_AGGREGATION); // 禁用AMPDU
    priv->hw->extra_tx_headroom = 4; // 头部预留空间，对于发送不带802.11头的802.3帧的ath11k来说是不需要的，反正不在driver中处理skb
    priv->hw->vif_data_size = sizeof(struct wireless_simu_vif);
    priv->hw->sta_data_size = sizeof(struct wireless_simu_sta);

    priv->hw->wiphy->interface_modes =
        BIT(NL80211_IFTYPE_MONITOR) |
        BIT(NL80211_IFTYPE_AP) |
        BIT(NL80211_IFTYPE_STATION);
    priv->hw->wiphy->iface_combinations = &wireless_simu_if_comb;
    priv->hw->wiphy->n_iface_combinations = 1; // 这里还没设置成多个的

    priv->hw->wiphy->available_antennas_tx = 1;
    priv->hw->wiphy->available_antennas_rx = 1;

    // 设备监管标识，暂时使用自定义的监管集合，因为我不会使用默认的CN监管
    // ath11k中与cfg80211回调函数接口进行对接来进行监管区域的设置
    priv->hw->wiphy->regulatory_flags = (REGULATORY_STRICT_REG | REGULATORY_CUSTOM_REG);
    wiphy_apply_custom_regulatory(priv->hw->wiphy, &wireless_simu_regd);

    // hw中队列数量设置，一般为4，因为硬件中一般会使用4个队列（对应于4个Qos等级）去发送data数据帧，mgmt单独再起一个队列
    priv->hw->queues = WIRELESS_SIMU_HW_QUEUE;

    /* wiphy扩展标识，用于nl80211部分功能的选择启用
     * nl80211用于userspace的控制软件对网卡进行控制
     */
    wiphy_ext_feature_set(priv->hw->wiphy, NL80211_EXT_FEATURE_CQM_RSSI_LIST);

    /*
     * mac address
     * 在高通 ath11k driver 中, 对每一个 radio 都注册了一个ieee80211_hw ,
     * 并且每个 hw 都拥有自己的 mac 地址.
     * 至于mac地址的设定, 在到达提交mac地址给netdevice系统之前, 高通使用了多种方式来得到一个mac地址
     * iwlwifi 的 mac 地址设定乱的一批.
     * 找人问一下高通芯片的mac地址:
     *  1. ax 芯片中使用ifconfig仅能看到一个mac地址
     */
    device_get_mac_address(&priv->pci_dev->dev, priv->mac_addr);
    if (!is_valid_ether_addr(priv->mac_addr))
    {
        pr_info("%s : mac probe : err mac addr use radom addr \n", WIRELESS_SIMU_DEVICE_NAME);
        eth_random_addr(priv->mac_addr);
    }
    pr_info("%s : mac probe : %02x:%02x:%02x:%02x:%02x:%02x \n", WIRELESS_SIMU_DEVICE_NAME,
            priv->mac_addr[5], priv->mac_addr[4], priv->mac_addr[3], priv->mac_addr[2], priv->mac_addr[1], priv->mac_addr[0]);
    SET_IEEE80211_PERM_ADDR(hw, priv->mac_addr);

    err = ieee80211_register_hw(hw);
    if (err)
    {
        pr_info("%s : mac probe : can not register device \n", WIRELESS_SIMU_DEVICE_NAME);
        err = MAC80211_REGISTER_ERR;
        goto err_free_dev;
    }
    else
    {
        pr_info("%s : mac80211 probe : ieee80211_hw regi success %d \n", WIRELESS_SIMU_DEVICE_NAME, err);
    }

    return SUCCESS;

err_free_dev:
    ieee80211_free_hw(hw);
    return err;
}