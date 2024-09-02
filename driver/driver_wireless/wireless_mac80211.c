#include "wireless_mac80211.h"

static void wireless_mac80211_tx(struct ieee80211_hw *dev,
                                 struct ieee80211_tx_control *control,
                                 struct sk_buff *skb)
{
}

static int wireless_mac80211_start(struct ieee80211_hw *hw)
{
}

static void wireless_mac80211_stop(struct ieee80211_hw *hw)
{
}

static int wireless_mac80211_add_interface(struct ieee80211_hw *hw,
                                           struct ieee80211_vif *vif)
{
}

static void wireless_mac80211_remove_interface(struct ieee80211_hw *hw,
                                               struct ieee80211_vif *vif)
{
}

static int wireless_mac80211_config(struct ieee80211_hw *hw,
                                    u32 changed)
{
}

static void wireless_mac80211_configure_filter(struct ieee80211_hw *hw,
                                               unsigned int changed_flags,
                                               unsigned int *total_flags,
                                               u64 multicast)
{
}

static void wireless_mac80211_wake_tx_queue(struct ieee80211_hw *hw,
                                            struct ieee80211_txq *txq)
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
}

static int wireless_mac80211_set_antenna(struct ieee80211_hw *hw,
                                         u32 tx_ant,
                                         u32 rx_ant)
{
}

static int wireless_mac80211_get_antenna(struct ieee80211_hw *hw,
                                         u32 *tx_ant,
                                         u32 *rx_ant)
{
}

static int wireless_mac80211_ampdu_action(struct ieee80211_hw *hw,
                                          struct ieee80211_vif *vif,
                                          struct ieee80211_ampdu_params *params)
{
}

static int wireless_mac80211_set_rts_threshold(struct ieee80211_hw *hw,
                                               u32 value)
{
}

static int wireless_mac80211_testmode_cmd(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
                                          void *data, int len)
{
}

static int wireless_mac80211_prepare_multicast(struct ieee80211_hw *hw,
                                               struct netdev_hw_addr_list *mc_list)
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
    .wake_tx_queue = wireless_mac80211_wake_tx_queue, // 这个是最新版linux新添加的强制性接口

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
    .rfkill_poll =,
    .get_tsf =,
    .set_tsf =,
    .reset_tsf =,
};

enum wireless_simu_err_code
wireless_mac80211_core_probe(struct wireless_simu *priv)
{
    // 该方法中对mac80211系统中api的调用顺序尽量不要打乱

    struct ieee80211_hw *hw = NULL;
    enum wireless_simu_err_code err = SUCCESS;

    hw = ieee80211_alloc_hw(sizeof(*priv), wireless_mac80211_ops);
    if (!hw)
    {
        err = MAC80211_ALLOC_ERR;
        pr_info("%s : mac probe : alloc hw failed \n", WIRELESS_SIMU_DEVICE_NAME);
        goto err_free_dev;
    }
    hw->priv = priv;
    priv->hw = hw;

    SET_IEEE80211_DEV(hw, &priv->pci_dev->dev);

    /* set channel rates
     * ath11k_mac_setup_channels_rates in ath11k driver
     */
    priv->band_2GHZ.band = NL80211_BAND_2GHZ;
    priv->band_2GHZ.channels = wireless_simu_2ghz_channels;
    priv->band_2GHZ.n_channels = ARRAY_SIZE(wireless_simu_2ghz_channels);
    priv->band_2GHZ.bitrates = wireless_simu_g_rates;
    priv->band_2GHZ.n_bitrates = wireless_simu_g_rates_size;
    priv->hw->wiphy->bands[NL80211_BAND_2GHZ] = &priv->band_2GHZ;

    wiphy_read_of_freq_limits(priv->hw->wiphy);
    /*
     * set hw_flags
     * enum ieee80211_hw_flags
     */
    

    /*
     * mac address
     * 在高通 ath11k driver 中, 对每一个 radio 都注册了一个ieee80211_hw ,
     * 并且每个 hw 都拥有自己的 mac 地址.
     * 至于mac地址的设定, 在到达提交mac地址给netdevice系统之前, 高通使用了多种方式来得到一个mac地址
     * iwlwifi 的 mac 地址设定乱的一批.
     * 找人问一下高通芯片的mac地址: 
     *  1. ax 芯片中使用ifconfig仅能看到一个mac地址
     */
    device_get_mac_address(priv->pci_dev->dev, priv->mac_addr);
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
        pr_info("%S : mac probe : can not register device \n", WIRELESS_SIMU_DEVICE_NAME);
        err = MAC80211_REGISTER_ERR;
        goto err_free_dev;
    }
    else
    {
        pr_info("%s : mac80211 probe : ieee80211_hw regi success %d \n", WIRELESS_SIMU_DEVICE_NAME, err);
    }

err_free_dev:
    ieee80211_free_hw(hw);
    return err;
}