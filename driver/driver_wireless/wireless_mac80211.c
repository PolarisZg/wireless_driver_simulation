#include "wireless_mac80211.h"

void wireless_sample_send_cb(struct wireless_simu *priv, struct sk_buff *skb)
{
    pr_info("%s : sample send cb end \n", WIRELESS_SIMU_DEVICE_NAME);
    struct ieee80211_hw *dev = priv->hw;
    // 需要添加 tx 发送完成之后帧的操作
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    bool use_ht_aggr = ((info->flags&IEEE80211_TX_CTL_AMPDU)!=0);
    bool tx_fail = true;
    bool pkt_need_ack = (!(info->flags & IEEE80211_TX_CTL_NO_ACK));
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
    u32 addr1_low32 = *((u32*)(hdr->addr1+2));


    ieee80211_tx_info_clear_status(info);

    if(use_ht_aggr) {
        pr_info("%s : sample send cb : dont support ht_aggr \n", WIRELESS_SIMU_DEVICE_NAME);
        info->flags |= IEEE80211_TX_STAT_AMPDU;
    } else {
        tx_fail = false;
        info->flags &= (~IEEE80211_TX_CTL_AMPDU);
    }

    // 填充速率和天线配置
    info->status.rates[0].count = 1;
    info->status.rates[0].idx = 0; // 默认速率索引
    info->status.rates[1].idx = -1; // 结束标志
    info->status.antenna = priv->simu_simple.runtime_tx_ant_cfg;
    
    ieee80211_tx_status_irqsafe(dev, skb);
    pr_info("%s : sample send cb end \n", WIRELESS_SIMU_DEVICE_NAME);
}
EXPORT_SYMBOL(wireless_sample_send_cb);

void wireless_sample_recv_cb(struct wireless_simu *priv, struct sk_buff *skb){
    pr_info("%s : sample recv cb end \n", WIRELESS_SIMU_DEVICE_NAME);
    struct ieee80211_hw *dev = priv->hw;
    struct ieee80211_rx_status rx_status = {0};

    rx_status.antenna = priv->simu_simple.runtime_rx_ant_cfg;

    memcpy(IEEE80211_SKB_RXCB(skb), &rx_status, sizeof(rx_status));
    ieee80211_rx_irqsafe(dev, skb);

}
EXPORT_SYMBOL(wireless_sample_recv_cb);

static void wireless_simu_mac_tx_end(int irq, struct wireless_simu *priv)
{
    // 取出skb，或者其他东西，比如tx_info, ack 状态之类的
    // 另外，这个tx end应该在什么时候调用？是发送到空中之后，还是发送到firmware之后
    // 插入tx_end skb 队列
    // 调度启动一个作业
}

static void wireless_simu_mac_rx(int irq, struct wireless_simu *priv)
{
    // 取出skb
    // 插入rx skb队列
    // 调度启动一个作业
}

static void wireless_mgmt_over_wmi_tx_drop(struct wireless_simu *priv, struct sk_buff *skb)
{
    int num_mgmt;

    ieee80211_free_txskb(priv->hw, skb);

    num_mgmt = atomic_dec_if_positive(&priv->num_pending_mgmt_tx);

    if (num_mgmt < 0)
        WARN_ON_ONCE(1);

    /* 唤醒等待在 txmgmt_empty_waitq 的线程,
     * 其他线程可以使用
     * wait_event_timeout(priv->txmgmt_empty_waitq, ...)
     * 在该等待队列中等待
     */
    if (!num_mgmt)
        wake_up(&priv->txmgmt_empty_waitq);
}

static int wireless_simu_mgmt_tx_wmi(struct wireless_simu *priv, struct wireless_simu_vif *simu_vif, struct sk_buff *skb)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
    struct wireless_skb_cb *skb_cb = WIRELESS_SKB_CB(skb);
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    enum hal_encrypt_type enctype;
    unsigned int mic_length;
    int buf_id;
    int ret = 0;
    dma_addr_t paddr;

    skb_cb->priv = priv;

    /* 统计 skb 的 id, 该 id 的上限由 WIRELESS_SIMU_TX_MGMT_NUM_PENDING_MAX 确定 */
    spin_lock_bh(&priv->txmgmt_idr_lock);
    buf_id = idr_alloc(&priv->txmgmt_idr, skb, 0, WIRELESS_SIMU_TX_MGMT_NUM_PENDING_MAX, GFP_ATOMIC);
    spin_unlock_bh(&priv->txmgmt_idr_lock);

    pr_info("%s : mac tx mgmt id %d \n", WIRELESS_SIMU_DEVICE_NAME, buf_id);

    if (buf_id < 0)
        return -ENOSPC;

    // 对于不同的加密，扩大skb空间，推测需要设置IEEE80211_HW_SW_CRYPTO_CONTROL标志位，并实现set_key函数来保证mac80211子系统不对skb加密
    if (!(info->flags & IEEE80211_TX_CTL_HW_80211_ENCAP))
    {
        if ((ieee80211_has_protected(hdr->frame_control)) &&
            (ieee80211_is_action(hdr->frame_control) ||
             ieee80211_is_deauth(hdr->frame_control) ||
             ieee80211_is_disassoc(hdr->frame_control)))
        {
            if (!(skb_cb->flags & WIRELESS_SIMU_SKB_CIPHER_SET))
                pr_info("%s : WMI management tx frame without ATH11K_SKB_CIPHER_SET \n", WIRELESS_SIMU_DEVICE_NAME);

            enctype = wireless_simu_dp_tx_get_encrypt_type(skb_cb->cipher);
            mic_length = wireless_simu_dp_rx_crypto_mic_len(priv, enctype);
            skb_put(skb, mic_length);
        }
    }

    // 转dma地址，并存到skb中，后期需要将pci_dev提出来，用来屏蔽掉总线的区别
    paddr = dma_map_single(&priv->pci_dev->dev, skb->data, skb->len, DMA_TO_DEVICE);
    if (dma_mapping_error(&priv->pci_dev->dev, paddr))
    {
        pr_err("%s : mac mgmt tx dma map err \n", WIRELESS_SIMU_DEVICE_NAME);
        ret = -EIO;
        goto err_free_idr;
    }

    skb_cb->paddr = paddr;

    // 发送到 wmi 层
    ret = wireless_simu_wmi_mgmt_send(priv, simu_vif->vif_id, buf_id, skb);
    if (ret)
    {
        pr_warn("%s : fail to send mgmt frame %d\n", WIRELESS_SIMU_DEVICE_NAME, ret);
        goto err_unmap_buf;
    }

    return 0;

err_unmap_buf:
    dma_unmap_single(&priv->pci_dev->dev, paddr, skb->len, DMA_TO_DEVICE);

err_free_idr:
    spin_lock_bh(&priv->txmgmt_idr_lock);
    idr_remove(&priv->txmgmt_idr, buf_id);
    spin_unlock_bh(&priv->txmgmt_idr_lock);

    return ret;
}

static void wireless_mgmt_over_wmi_tx_work(struct work_struct *work)
{
    // pr_info("%s : mac mgmt tx work start \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *priv = container_of(work, struct wireless_simu, mgmt_tx_work);
    struct sk_buff *skb = NULL;
    struct wireless_skb_cb *skb_cb;
    struct wireless_simu_vif *simu_vif;
    int ret;

    while ((skb = skb_dequeue(&priv->mgmt_tx_queue)) != NULL)
    {
        skb_cb = WIRELESS_SKB_CB(skb);
        if (!skb_cb->vif)
        {
            pr_err("%s : no vif find in skb \n", WIRELESS_SIMU_DEVICE_NAME);
            wireless_mgmt_over_wmi_tx_drop(priv, skb);
            continue;
        }

        simu_vif = wireless_vif_to_wivif(skb_cb->vif);
        mutex_lock(&priv->mac_conf_mutex);

        /* todo : 需要判断 simu_vif 中 vdev 是否存在且被分配空间
         * 一般来说分配空间和释放空间的操作存在于 add _interface 和 del_interface 之中
         */
        ret = wireless_simu_mgmt_tx_wmi(priv, simu_vif, skb);
        if (ret)
        {
            /* todo : 在还没有确定下 vdev 的含义之前, 暂时使用 vif 来代替
             */
            pr_warn("%s : fail to tx mgmt frame vif id %d : %d \n", WIRELESS_SIMU_DEVICE_NAME, simu_vif->vif_id, ret);
            wireless_mgmt_over_wmi_tx_drop(priv, skb);
        }
        else
        {
            pr_info("%s : tx mgmt frame vif %d \n", WIRELESS_SIMU_DEVICE_NAME, simu_vif->vif_id);
        }

        mutex_unlock(&priv->mac_conf_mutex);
    }
}

static int wireless_simu_mac_mgmt_tx(struct wireless_simu *priv, struct sk_buff *skb, bool is_prb_rsp)
{
    struct sk_buff_head *q = &priv->mgmt_tx_queue;

    /* 判断当前设备状态, 当前设备故障则中止发送
     * 而设备的故障信息是通过 qmi 通道发送过来的
     */

    /*
     * 打印发送的 mgmt 类型
     */
    u16 mgmt_frame_type = le16_to_cpu(((struct ieee80211_hdr *)skb->data)->frame_control);
    pr_info("%s : mgmt tx %04x type frame \n", WIRELESS_SIMU_DEVICE_NAME, mgmt_frame_type);

    /*
     * 应该优先保证response之外的mgmt帧发送，因此将rsp帧抽出来单独统计
     */
    if (is_prb_rsp && atomic_read(&priv->num_pending_mgmt_tx) > WIRELESS_SIMU_PRB_RSP_DROP_THRESHOLD)
    {
        pr_err("%s : mac mgmt tx prb_rsp is almost full \n", WIRELESS_SIMU_DEVICE_NAME);
        return -ENOSPC;
    }

    if (skb_queue_len_lockless(q) >= WIRELESS_SIMU_TX_MGMT_NUM_PENDING_MAX)
    {
        pr_err("%s : mac mgmt tx queue full \n", WIRELESS_SIMU_DEVICE_NAME);
        return -ENOSPC;
    }

    skb_queue_tail(q, skb);
    atomic_inc(&priv->num_pending_mgmt_tx);
    queue_work(priv->workqueue_aux, &priv->mgmt_tx_work); // 转发到 wireless_mgmt_over_wmi_tx_work
    // pr_info("%s : mac mgmt tx in queue \n", WIRELESS_SIMU_DEVICE_NAME);

    return 0;
}

static void wireless_mac80211_tx(struct ieee80211_hw *dev,
                                 struct ieee80211_tx_control *control,
                                 struct sk_buff *skb)
{
    struct wireless_simu *priv = (struct wireless_simu *)dev->priv;
    struct wireless_skb_cb *skb_cb = WIRELESS_SKB_CB(skb);
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    struct ieee80211_key_conf *key = info->control.hw_key;
    struct ieee80211_vif *vif = info->control.vif;
    struct wireless_simu_vif *wivif = (struct wireless_simu_vif *)vif->drv_priv;
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
    struct wireless_simu_sta *wista = NULL;
    u32 info_flags = info->flags;
    bool is_prb_rsp;
    int ret = 0;

    memset(skb_cb, 0, sizeof(*skb_cb));
    skb_cb->vif = vif;

    if (key)
    {
        skb_cb->cipher = key->cipher;
        skb_cb->flags |= WIRELESS_SIMU_SKB_CIPHER_SET;
    }

    if (info_flags & IEEE80211_TX_CTL_HW_80211_ENCAP)
    {
        skb_cb->flags |= WIRELESS_SIMU_SKB_HW_80211_ENCAP;
    }
    else if (ieee80211_is_mgmt(hdr->frame_control))
    {
        is_prb_rsp = ieee80211_is_probe_resp(hdr->frame_control);
        ret = wireless_simu_mac_mgmt_tx(priv, skb, is_prb_rsp);
        if (ret)
        {
            pr_err("%s : mgmt tx fail queue \n", WIRELESS_SIMU_DEVICE_NAME);
            ieee80211_free_txskb(priv->hw, skb);
        }
        return;
    }

    // 非 mgmt 的发送
    if (control->sta)
    {
        wista = wireless_sta_to_wista(control->sta);
    }

    ret = wireless_dp_tx(priv, wivif, wista, skb);
    if (unlikely(ret))
    {
        pr_err("%s : fail to transmit frame %d \n", WIRELESS_SIMU_DEVICE_NAME, ret);
        ieee80211_free_txskb(priv->hw, skb);
    }
}

inline void report_pkt_loss_due_to_driver_drop(struct ieee80211_hw *dev, struct sk_buff *skb)
{
    struct wireless_simu *priv = dev->priv;
    struct ieee80211_tx_info *info;

    info = IEEE80211_SKB_CB(skb);
    ieee80211_tx_info_clear_status(info);
    info->status.rates[0].count = 1;
    info->status.rates[1].idx = -1;
    info->status.antenna = priv->simu_simple.runtime_tx_ant_cfg;
    ieee80211_tx_status_irqsafe(dev, skb);
}

static void simu_simple_tx(struct ieee80211_hw *dev, struct ieee80211_tx_control *control, struct sk_buff *skb)
{
    pr_info("%s : simple tx start \n", WIRELESS_SIMU_DEVICE_NAME);
    // goto simu_simple_tx_early_out;

    struct wireless_simu *priv = dev->priv;
    // unsigned long flags;
    // struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    struct wireless_skb_cb *skb_cb = WIRELESS_SKB_CB(skb);
    // struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
    u16 len_mpdu;
    u16 prio;
    // u32 addr_low32;
    int ret;

    memset(skb_cb, 0, sizeof(*skb_cb));

    // 不去传输被仍是链表状态的skb
    if (skb->data_len > 0)
    {
        pr_info("%s : tx : WARNING skb->data_len > 0 \n", WIRELESS_SIMU_DEVICE_NAME);
        goto simu_simple_tx_early_out;
    }

    len_mpdu = skb->len;

    // 不去传输队列号太高的 skb
    prio = skb_get_queue_mapping(skb);
    if (prio > WIRELESS_SIMU_HW_QUEUE)
    {
        pr_info("%s : tx : queue more %04x", WIRELESS_SIMU_DEVICE_NAME, prio);
        goto simu_simple_tx_early_out;
    }

    /* 不去理解后面的步骤, 也不对mgmt和data区分, 直接发包 */
    /* 借用一下 CE 的 srng 去发包 */
    /* 不需要对 skb 做缓存, 因为在 pipe 中已经将 skb 缓存到了 write_index 的地方
     * 只需要在硬件传输完毕后从 tp 处开始读取就好了
     */
    skb_cb->paddr = dma_map_single(&priv->pci_dev->dev, skb->data, skb->len, DMA_TO_DEVICE);

    if(dma_mapping_error(&priv->pci_dev->dev, skb_cb->paddr)){
        pr_info("%s : simu_simple_tx dma map err \n", WIRELESS_SIMU_DEVICE_NAME);
        goto simu_simple_tx_early_out;
    }
    ret = wireless_simu_ce_send(priv, skb, 2, 0);
    if (ret != 0)
    {
        pr_info("%s : simu_simple tx send err %d \n", WIRELESS_SIMU_DEVICE_NAME, ret);
        goto simu_simple_tx_dma_free;
    }

    pr_info("%s : pkt tx succeed \n", WIRELESS_SIMU_DEVICE_NAME);
    return;

simu_simple_tx_dma_free:
    dma_unmap_single(&priv->pci_dev->dev, skb_cb->paddr, skb->len, DMA_TO_DEVICE);
simu_simple_tx_early_out:
    report_pkt_loss_due_to_driver_drop(dev, skb);
}

static int wireless_mac80211_start(struct ieee80211_hw *hw)
{
    pr_info("%s : mac80211 start \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *priv = hw->priv;

    // 清空vif
    for (int i = 0; i < WIRELESS_MAX_NUM_VIF; i++)
    {
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void wireless_mac80211_stop(struct ieee80211_hw *hw, bool suspend)
#else
static void wireless_mac80211_stop(struct ieee80211_hw *hw)
#endif
{
    pr_info("%s : mac80211 stop \n", WIRELESS_SIMU_DEVICE_NAME);
    struct wireless_simu *priv = hw->priv;

    // 关闭天线

    // 停掉队列

    // 关闭driver中的vif
    for (int i = 0; i < WIRELESS_MAX_NUM_VIF; i++)
    {
        priv->vif[i] = NULL;
    }

    // 关闭中断
    priv->tx_interrupt_enable = false;
    priv->rx_interrupt_enable = false;

    pr_info("%s : mac80211 stop done \n", WIRELESS_SIMU_DEVICE_NAME);
}

static void simu_simple_beacon_work(struct work_struct *work)
{
    struct wireless_simu_vif *vif_priv = container_of(work, struct wireless_simu_vif, simu_simple.beacon_work.work);
    struct wireless_simu *priv = vif_priv->priv;
    struct ieee80211_vif *vif = vif_priv->vif;
    struct ieee80211_hw *dev = priv->hw;
    struct ieee80211_mgmt *mgmt;
    struct sk_buff *skb;

    skb = ieee80211_beacon_get(dev, vif, 0);
    if (!skb)
        goto resched;

    mgmt = (struct ieee80211_mgmt *)skb->data;
    mgmt->u.beacon.timestamp = cpu_to_le64(ktime_get_ns() / 1000);

    skb_set_queue_mapping(skb, 0);
    simu_simple_tx(dev, NULL, skb);

resched:
    schedule_delayed_work(&vif_priv->simu_simple.beacon_work, usecs_to_jiffies(1024 * vif->bss_conf.beacon_int));
}

static int wireless_mac80211_add_interface(struct ieee80211_hw *hw,
                                           struct ieee80211_vif *vif)
{
    pr_info("%s : add interface start \n", WIRELESS_SIMU_DEVICE_NAME);

    struct wireless_simu *priv = hw->priv;
    struct wireless_simu_vif *simu_vif = (struct wireless_simu_vif *)vif->drv_priv;
    memset(simu_vif, 0, sizeof(struct wireless_simu_vif));

    mutex_lock(&priv->mac_conf_mutex);

    int vif_id = 0;
    for (vif_id = 0; vif_id < ARRAY_SIZE(priv->vif); vif_id++)
    {
        if (priv->vif[vif_id] == NULL)
            break;
    }
    if (vif_id == ARRAY_SIZE(priv->vif))
    {
        pr_err("%s : cannt add interface no more space \n", WIRELESS_SIMU_DEVICE_NAME);
        mutex_unlock(&priv->mac_conf_mutex);
        return -EBUSY;
    }
    priv->vif[vif_id] = vif;
    simu_vif->vif_id = vif_id;
    simu_vif->priv = priv;
    simu_vif->vif = vif;

    if (vif->type != NL80211_IFTYPE_STATION && vif->type != NL80211_IFTYPE_AP && vif->type != NL80211_IFTYPE_MONITOR)
    {
        mutex_unlock(&priv->mac_conf_mutex);
        pr_err("%s : add interface %d type invalid \n", WIRELESS_SIMU_DEVICE_NAME, vif->type);
        return -EINVAL;
    }

    // // 设置队列id, 因为之前设置了hw的flag IEEE80211_HW_QUEUE_CONTROL
    // pr_info("%s : add interface %d type \n", WIRELESS_SIMU_DEVICE_NAME, vif->type);

    for (int i = 0; i < ARRAY_SIZE(vif->hw_queue); i++)
        vif->hw_queue[i] = i % (WIRELESS_SIMU_HW_QUEUE - 1); // 这里我也不懂为什么要减一

    if (vif->type == NL80211_IFTYPE_AP)
        vif->cab_queue = simu_vif->vif_id % (WIRELESS_SIMU_HW_QUEUE - 1);
    else
        vif->cab_queue = IEEE80211_INVAL_HW_QUEUE;

    // 之后是一些我看不懂的硬件配置

    INIT_DELAYED_WORK(&simu_vif->simu_simple.beacon_work, simu_simple_beacon_work);

    pr_info("%s : add infc idx %d addr %02x:%02x:%02x:%02x:%02x:%02x\n", WIRELESS_SIMU_DEVICE_NAME, simu_vif->vif_id,
            vif->addr[0], vif->addr[1], vif->addr[2], vif->addr[3], vif->addr[4], vif->addr[5]);

    mutex_unlock(&priv->mac_conf_mutex);
    return 0;
}

static void wireless_mac80211_remove_interface(struct ieee80211_hw *hw,
                                               struct ieee80211_vif *vif)
{
    struct wireless_simu *priv = hw->priv;
    struct wireless_simu_vif *simu_vif = (struct wireless_simu_vif *)vif->drv_priv;

    mutex_lock(&priv->mac_conf_mutex);
    // 没有什么删除的操作，底层firmware没有和上层mac80211子系统中interface对应的模块
    // 只需要在驱动中删除对应的interface缓存即可
    priv->vif[simu_vif->vif_id] = NULL;
    pr_info("%s : mac remove interface idx %d type %d \n", WIRELESS_SIMU_DEVICE_NAME, simu_vif->vif_id, vif->type);
    mutex_unlock(&priv->mac_conf_mutex);
}

static int wireless_mac80211_config(struct ieee80211_hw *hw,
                                    u32 changed)
{
    struct ieee80211_conf *conf = &hw->conf;
    pr_info("%s : mac_config flag %08x \n", WIRELESS_SIMU_DEVICE_NAME, changed);
    // pr_info("%s : mac_config center_freq %08x width %d \n", WIRELESS_SIMU_DEVICE_NAME, conf->chandef.chan->center_freq, conf->chandef.width);
    return 0;
}

/* 在这里写明设备可以上报给mac80211子系统的帧类型，写多了也没事，反正不上报又无所谓
 * FIF_FCSFAIL通过FCS检定失败的帧，需对这些帧设置RX_FLAG_FAILED_FCS_CRC
 * FIF_PLCPFAIL 通过PLCP CRC检定失败的帧，需对这些帧设置RX_FLAG_FAILED_PLCP_CRC
 */
#define SUPPORT_FILTERS        \
    (FIF_ALLMULTI |            \
     FIF_FCSFAIL |             \
     FIF_PLCPFAIL |            \
     FIF_BCN_PRBRESP_PROMISC | \
     FIF_CONTROL |             \
     FIF_OTHER_BSS |           \
     FIF_PSPOLL |              \
     FIF_PROBE_REQ |           \
     FIF_MCAST_ACTION)

static void wireless_mac80211_configure_filter(struct ieee80211_hw *hw,
                                               unsigned int changed_flags,
                                               unsigned int *total_flags,
                                               u64 multicast)
{
    // 必须编写的回调函数
    // 该函数表示硬件会上报给mac80211子系统的帧类型，一般来说上报的帧类型越多越好，但会带来性能的问题
    struct wireless_simu *priv = hw->priv;
    mutex_lock(&priv->mac_conf_mutex);

    (*total_flags) &= SUPPORT_FILTERS;
    (*total_flags) |= FIF_ALLMULTI;
    priv->filter_flags = *total_flags;

    mutex_unlock(&priv->mac_conf_mutex);
}

static void wireless_mac80211_bss_info_changed(struct ieee80211_hw *hw,
                                               struct ieee80211_vif *vif,
                                               struct ieee80211_bss_conf *info,
                                               u64 changed)
{
    // bss 变化
    /* 具体的哪种 bss 变化由 changed 给出 */

    struct wireless_simu *priv = (struct wireless_simu *)hw->priv;
    struct wireless_simu_vif *vif_priv = (struct wireless_simu_vif *)vif->drv_priv;

    if (changed & BSS_CHANGED_BSSID)
    {
        // 该 sta 所在的 bss id 被修改
        // 所谓 BSSID 实际就是 mac 地址吧
        pr_info("%s : bss info changed BSSID to %02x:%02x:%02x:%02x:%02x:%02x \n", WIRELESS_SIMU_DEVICE_NAME, info->bssid[0], info->bssid[1], info->bssid[2], info->bssid[3], info->bssid[4], info->bssid[5]);

        vif_priv->simu_simple.bssid_low = (*((u32 *)(info->bssid)));
        vif_priv->simu_simple.bssid_hi = (*((u16 *)(info->bssid + 4)));
    }
    if (changed & BSS_CHANGED_BEACON_INT)
    {
        // 信标帧间隔调整
        // 没写对应的寄存器, 硬件没有提供寄存器
        pr_info("%s : WARNNING bss info changed becon int %x \n", WIRELESS_SIMU_DEVICE_NAME, info->beacon_int);
        vif_priv->simu_simple.beacon_int = info->beacon_int;
    }
    if (changed & BSS_CHANGED_TXPOWER)
    {
        pr_info("%s : WARNNING bss info changed tx power %x \n", WIRELESS_SIMU_DEVICE_NAME, info->txpower);
    }
    if (changed & BSS_CHANGED_ERP_CTS_PROT)
    {
        pr_info("%s : WARNNING bsss info changed ctr protection %x", WIRELESS_SIMU_DEVICE_NAME, info->use_cts_prot);
    }
    if (changed & BSS_CHANGED_BASIC_RATES)
    {
        pr_info("%s : WARNNING bss info changed basic rates %x \n", WIRELESS_SIMU_DEVICE_NAME, info->basic_rates);
    }
    if (changed & (BSS_CHANGED_ERP_SLOT | BSS_CHANGED_ERP_PREAMBLE))
    {
        pr_info("%s : WARNNING BSS_CHANGED_ERP_SLOT BSS_CHANGED_ERP_PREAMBLE short slot %d \n", WIRELESS_SIMU_DEVICE_NAME, info->use_short_slot);
        priv->simu_simple.use_short_slot = info->use_short_slot;
    }
    if (changed & BSS_CHANGED_BEACON_ENABLED)
    {
        printk("%s : WARNNING bss info changed enable beacon %d \n", WIRELESS_SIMU_DEVICE_NAME, info->enable_beacon);
        vif_priv->simu_simple.enable_beacon = info->enable_beacon;
    }
    if (changed & (BSS_CHANGED_BEACON_ENABLED | BSS_CHANGED_BEACON))
    {
        cancel_delayed_work_sync(&vif_priv->simu_simple.beacon_work);
        pr_info("%s WARNNING bss info changed beacon changed \n", WIRELESS_SIMU_DEVICE_NAME);
        if (vif_priv->simu_simple.enable_beacon)
        {
            schedule_work(&vif_priv->simu_simple.beacon_work.work);
            pr_info("%s : WARNNING bss info start beacon_work \n", WIRELESS_SIMU_DEVICE_NAME);
        }
    }
}

static int wireless_mac80211_conf_tx(struct ieee80211_hw *hw,
                                     struct ieee80211_vif *vif,
                                     unsigned int link_id, u16 ac,
                                     const struct ieee80211_tx_queue_params *params)
{
    // 对firmware的qos队列进行配置
    // 具体的配置信息（EDCA相关，包括txop，aifs等）存放在params中
    struct wireless_simu *priv = (struct wireless_simu *)hw->priv;
    struct wireless_simu_vif *simu_vif = (struct wireless_simu_vif *)vif->drv_priv;
    struct wireless_simu_tx_params_arg *arg = NULL;

    mutex_lock(&priv->mac_conf_mutex);
    switch (ac)
    {
    case IEEE80211_AC_VO:
        arg = &simu_vif->ac_vo;
        break;
    case IEEE80211_AC_BE:
        arg = &simu_vif->ac_be;
        break;
    case IEEE80211_AC_BK:
        arg = &simu_vif->ac_bk;
        break;
    case IEEE80211_AC_VI:
        arg = &simu_vif->ac_vi;
        break;
    default:
        mutex_unlock(&priv->mac_conf_mutex);
        return -EINVAL;
    }

    arg->cwmax = params->cw_max;
    arg->cwmin = params->cw_min;
    arg->aifs = params->aifs;
    arg->txop = params->txop;
    // 这里应该还发生一个对no_ack的赋值，但是我找不到
    // u-apsd 用于节能模式下对qos的控制，指示该队列是否启用uapsd
    // 后面的和mu相关，暂时不考虑
    pr_info("%s : mac %d vif %d ac set %d aifs %d cwmin %d cwmax %d txop %d acm %d no_ack \n",
            WIRELESS_SIMU_DEVICE_NAME, simu_vif->vif_id, ac, arg->aifs, arg->cwmin, arg->cwmax, arg->txop, arg->acm, arg->no_ack);
    // 应该将arg发送向firmware去配置特定queue的发送参数，但是我的firmware中没写相关的寄存器去接收数据，所以就不发了
    mutex_unlock(&priv->mac_conf_mutex);
    return 0;
}

static int wireless_mac80211_set_antenna(struct ieee80211_hw *hw,
                                         u32 tx_ant,
                                         u32 rx_ant)
{
    struct wireless_simu *priv = hw->priv;

    pr_info("%s : mac set antenna tx_ant %d rx_ant %d \n", WIRELESS_SIMU_DEVICE_NAME, tx_ant, rx_ant);

    // 检查天线参数是否合法, 是否超出了当前设备支持的天线标号
    // 这里的天线参数指的是 tx_ant 和 tx_ant ，不同的天线参数由驱动定义其含义, 比如在 simu_simple 中 tx_ant = 1/2 代表开启两个天线之一 3 代表两天线全部开启

    // 根据上方的天线参数设置设备上不同天线的开启和关闭状态, 所谓关闭, 指的是将天线的射频信号衰减设置为一个超大的值, 比如 89750
    // 注意对于多天线的设备，要保证每个天线都被设置上正确的参数

    // simu_simple在驱动中对硬件的天线进行配置, 可能和它使用 AD9361 相关, 系统中提供了对应的驱动
    // 这样的配置包括配置无线射频芯片的 本振 和 频偏
    priv->simu_simple.runtime_tx_ant_cfg = tx_ant;
    priv->simu_simple.runtime_tx_ant_cfg = rx_ant;

    return 0;
}

static int wireless_mac80211_get_antenna(struct ieee80211_hw *hw,
                                         u32 *tx_ant,
                                         u32 *rx_ant)
{
    struct wireless_simu *priv = hw->priv;

    *tx_ant = priv->simu_simple.runtime_tx_ant_cfg;
    *rx_ant = priv->simu_simple.runtime_rx_ant_cfg;

    pr_info("%s : mac get antenna tx ant %d rx_ant %d \n", WIRELESS_SIMU_DEVICE_NAME, *tx_ant, *rx_ant);

    return 0;
}

static int wireless_mac80211_ampdu_action(struct ieee80211_hw *hw,
                                          struct ieee80211_vif *vif,
                                          struct ieee80211_ampdu_params *params)
{
    return 0;
}

static int wireless_mac80211_add_chanctx(struct ieee80211_hw *hw,
                                         struct ieee80211_chanctx_conf *ctx)
{
    return 0;
}

static void wireless_mac80211_remove_chanctx(struct ieee80211_hw *hw,
                                             struct ieee80211_chanctx_conf *ctx)
{
}

static void wireless_mac80211_change_chanctx(struct ieee80211_hw *hw,
                                             struct ieee80211_chanctx_conf *ctx,
                                             u32 changed)
{
}

static int wireless_mac80211_assign_vif_chanctx(struct ieee80211_hw *hw,
                                                struct ieee80211_vif *vif,
                                                struct ieee80211_bss_conf *link_conf,
                                                struct ieee80211_chanctx_conf *ctx)
{
    return 0;
}

static void wireless_mac80211_unassign_vif_chanctx(struct ieee80211_hw *hw,
                                                   struct ieee80211_vif *vif,
                                                   struct ieee80211_bss_conf *link_conf,
                                                   struct ieee80211_chanctx_conf *ctx)
{
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
    printk("%s: prepare_muticast \n", WIRELESS_SIMU_DEVICE_NAME);
	return netdev_hw_addr_list_count(mc_list);
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

static int wireless_mac80211_hw_scan(struct ieee80211_hw *hw, struct ieee80211_vif *vif, struct ieee80211_scan_request *req)
 {
    return 1;
 }
static const struct ieee80211_ops wireless_mac80211_ops = {
    // 该部分必须完成, 否则无法申请ieee80211_hw结构体内存
    .tx = simu_simple_tx, // wireless_mac80211_tx,
    .start = wireless_mac80211_start,
    .stop = wireless_mac80211_stop,
    .add_interface = wireless_mac80211_add_interface,
    .remove_interface = wireless_mac80211_remove_interface,
    .config = wireless_mac80211_config,
    .set_antenna = wireless_mac80211_set_antenna,
    .get_antenna = wireless_mac80211_get_antenna,
    .configure_filter = wireless_mac80211_configure_filter,
    .wake_tx_queue = ieee80211_handle_wake_tx_queue, // 这个是最新版linux新添加的强制性接口
    .bss_info_changed = wireless_mac80211_bss_info_changed,
    // 该部分可自行选择满足, 参考ath11k, 保留的参考simu_simple
    // .reconfig_complete = ,
    // .update_vif_offload = ,
    // .hw_scan = ,
    // .cancel_hw_scan = ,
    // .set_key = ,
    // .set_rekey_data = ,
    // .sta_state = ,
    // .sta_set_4addr = ,
    // .sta_set_txpwr = ,
    // .sta_rc_update = ,
    .conf_tx = wireless_mac80211_conf_tx,
    .ampdu_action = wireless_mac80211_ampdu_action,
    .add_chanctx = wireless_mac80211_add_chanctx,
    .remove_chanctx = wireless_mac80211_remove_chanctx,
    .change_chanctx = wireless_mac80211_change_chanctx,
    .assign_vif_chanctx = wireless_mac80211_assign_vif_chanctx,
    .unassign_vif_chanctx = wireless_mac80211_unassign_vif_chanctx,
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

        // 下面为simu_simple中实现但ath11k中没有的，可能是因为softmac和full mac的不同吧
        .prepare_multicast = wireless_mac80211_prepare_multicast,
    .rfkill_poll = wireless_mac80211_rfkill_poll,
    .get_tsf = wireless_mac80211_get_tsf,
    .set_tsf = wireless_mac80211_set_tsf,
    .reset_tsf = wireless_mac80211_reset_tsf,
    .hw_scan = wireless_mac80211_hw_scan,
};

enum wireless_simu_err_code
wireless_mac80211_core_probe(struct wireless_simu *priv)
{
    pr_info("%s : mac80211 core probe start \n", WIRELESS_SIMU_DEVICE_NAME);
    // 该方法中对mac80211系统中api的调用顺序尽量不要打乱

    struct ieee80211_hw *hw = NULL;
    enum wireless_simu_err_code err = SUCCESS;

    mutex_init(&priv->mac_conf_mutex);

    hw = ieee80211_alloc_hw(sizeof(*priv), &wireless_mac80211_ops);
    if (!hw)
    {
        err = MAC80211_ALLOC_ERR;
        pr_info("%s : mac probe : alloc hw failed \n", WIRELESS_SIMU_DEVICE_NAME);
        goto err_end;
    }
    hw->priv = priv;
    priv->hw = hw;

    SET_IEEE80211_DEV(hw, &priv->pci_dev->dev);

    // 对mgmt tx work相关组件的初始化
    INIT_WORK(&priv->mgmt_tx_work, wireless_mgmt_over_wmi_tx_work);
    skb_queue_head_init(&priv->mgmt_tx_queue);

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
    ieee80211_hw_set(priv->hw, QUEUE_CONTROL);

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
    for (int i = 0; i < ARRAY_SIZE(priv->vif); i++)
    {
        priv->vif[i] = NULL;
    }

    priv->hw->wiphy->available_antennas_tx = 1;
    priv->hw->wiphy->available_antennas_rx = 1;

    // 设备监管标识，暂时使用自定义的监管集合，因为我不会使用默认的CN监管
    // ath11k中与cfg80211回调函数接口进行对接来进行监管区域的设置
    priv->hw->wiphy->regulatory_flags = (REGULATORY_STRICT_REG | REGULATORY_CUSTOM_REG);
    wiphy_apply_custom_regulatory(priv->hw->wiphy, &wireless_simu_regd);

    // hw中队列数量设置，一般为4，因为硬件中一般会使用4个队列（对应于4个Qos等级）去发送data数据帧，mgmt单独再起一个队列
    priv->hw->queues = WIRELESS_SIMU_HW_QUEUE;
    priv->hw->offchannel_tx_hw_queue = WIRELESS_SIMU_HW_QUEUE - 1;

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

    init_waitqueue_head(&priv->txmgmt_empty_waitq);
    init_waitqueue_head(&priv->tx_ce_desc_wq);
    spin_lock_init(&priv->txmgmt_idr_lock);
    spin_lock_init(&priv->htc_tx_lock);
    idr_init(&priv->txmgmt_idr);

    err = ieee80211_register_hw(hw);
    if (err)
    {
        pr_info("%s : mac probe : can not register device %d \n", WIRELESS_SIMU_DEVICE_NAME, err);
        err = MAC80211_REGISTER_ERR;
        goto err_free_dev;
    }
    else
    {
        pr_info("%s : mac80211 probe : ieee80211_hw regi success %d \n", WIRELESS_SIMU_DEVICE_NAME, err);
    }

    return SUCCESS;

err_free_dev:
    kfree(priv->band_2GHZ.channels);
    kfree(priv->band_5GHZ.channels);
    kfree(priv->band_6GHZ.channels);
    ieee80211_free_hw(hw);
err_end:
    priv->hw = NULL;
    return err;
}

void wireless_mac80211_drain_tx(struct wireless_simu *priv)
{
    synchronize_net();

    cancel_work_sync(&priv->mgmt_tx_work);
    // todo : 删除 mgmt 队列中的 skb
}

int wireless_mac80211_wait_tx_complete(struct wireless_simu *priv)
{
    wireless_mac80211_drain_tx(priv);
    // todo : 这里应当等待上述删除 skb 的过程
    return 0;
}

int wireless_mac80211_core_remove(struct wireless_simu *priv)
{
    struct ieee80211_hw *hw = priv->hw;
    if (!hw)
    {
        pr_info("%s : mac device remove \n", WIRELESS_SIMU_DEVICE_NAME);
        return 0;
    }

    ieee80211_unregister_hw(hw);

    ieee80211_free_hw(hw);
    return 0;
}
