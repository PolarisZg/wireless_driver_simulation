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
    struct ieee80211_hw *hw = NULL;

    hw = ieee80211_alloc_hw(sizeof(*priv), wireless_mac80211_ops);
}