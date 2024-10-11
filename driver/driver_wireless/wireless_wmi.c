#include "wireless.h"

// 申请wmi skb
struct sk_buff *wireless_simu_wmi_alloc_skb(struct wireless_simu *priv, int len)
{
    struct sk_buff *skb;
    u32 round_len = roundup(len, 4);

    // 申请htc层skb，暂时不知道htc层作用
    skb = wireless_simu_htc_alloc_skb(priv, WMI_SKB_HEADROOM + round_len);
    if (!skb)
        return NULL;

    skb_reserve(skb, WMI_SKB_HEADROOM);
    if (!IS_ALIGNED((unsigned long)skb->data, 4))
        pr_warn("%s : wmi alloc skb unalogned \n", WIRELESS_SIMU_DEVICE_NAME);

    skb_put(skb, round_len);
    memset(skb->data, 0, round_len);

    return skb;
}

int wireless_simu_wmi_cmd_send_nowait(struct wireless_simu *priv, struct sk_buff *skb, u32 cmd_id){
    struct wireless_simu_skb_cb *skb_cb = WIRELESS_SIMU_SKB_CB(skb);
    struct wmi_cmd_hdr *cmd_hdr;
    int ret;
    u32 cmd = 0;

    if(skb_push(skb, sizeof(struct wmi_cmd_hdr)) == NULL)
        return -ENOMEM;
    
    cmd |= FIELD_PREP(WMI_CMD_HDR_CMD_ID, cmd_id);
    cmd_hdr = (struct wmi_cmd_hdr*)skb->data;
    cmd_hdr->cmd_id = cmd;

    // 没有追踪框架，不写追踪部分

    memset(skb_cb, 0, sizeof(*skb_cb));
    ret = wireless_simu_htc_send(priv, skb);

    if(ret)
        goto err_pull;

    return 0;

err_pull:
    skb_pull(skb, sizeof(struct wmi_cmd_hdr));
    return ret;
}

int wireless_simu_wmi_cmd_send(struct wireless_simu *priv, struct sk_buff *skb, u32 cmd_id)
{
    int ret = -EOPNOTSUPP;

    might_sleep();

    /* 接下来的这个判断，我推断其含义为在不同版本的驱动中使用了不同等级的等待队列，因此在这里进行了一次区分；
     * 上述的不同等级指的是对应于hw等级的等待队列，所有的wmi cmd都要在该队列中排队；和对应于vif的等待队列，不同vif的skb对应的cmd在不同队列中排队
     * 我这里的实现仅使用了一个等待队列，就不区分了；
     * 
     * c 语言中还能出现lambda表达式？？？
     */
    wait_event_timeout(priv->tx_ce_desc_wq, ({
                           ret = wireless_simu_wmi_cmd_send_nowait(priv, skb, cmd_id);
                           if (ret && test_bit(WIRELESS_SIMU_DEV_FLAG_CRASH_FLUSH, &priv->dev_flag))
                               ret = -ESHUTDOWN;

                           (ret != -ENOBUFS);
                       }),
                       WMI_SEND_TIMEOUT_HZ);


    return ret;
}

int wireless_simu_wmi_mgmt_send(struct wireless_simu *priv, u32 vif_id, u32 buf_id, struct sk_buff *frame)
{
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(frame);
    struct wmi_mgmt_send_cmd *cmd;
    struct wmi_tlv *frame_tlv;
    struct sk_buff *skb;
    u32 buf_len;
    int ret, len;

    // 这里不知道为什么高通协议中将mgmt帧大小和 64 取了一个最小值，之后将frame中data部分仅发送了取得最小值的部分
    // 这一部分就搞不懂，难道说大于64的不再发送吗？不应该啊
    // 考虑到cmd中存储待发送的mgmt帧的大小，所以firmware想要拿到整个帧也并不是不可能，那这个最小值应该是有别样的意义，难道是为了防止申请的空间过大？
    buf_len = frame->len < WMI_MGMT_SEND_DOWNLD_LEN ? frame->len : WMI_MGMT_SEND_DOWNLD_LEN;

    len = sizeof(*cmd) + sizeof(*frame_tlv) + roundup(buf_len, 4);

    skb = wireless_simu_wmi_alloc_skb(priv, len);
    if (!skb)
        return -ENOMEM;

    cmd = (struct wmi_mgmt_send_cmd *)skb->data;
    cmd->tlv_header = FIELD_PREP(WMI_TLV_TAG, WMI_TAG_MGMT_TX_SEND_CMD) | FIELD_PREP(WMI_TLV_LEN, sizeof(*cmd) - TLV_HDR_SIZE);
    cmd->vdev_id = vif_id;
    cmd->desc_id = buf_id;
    cmd->chanfreq = 0; // 暂时写成0, 还不知道应该从哪里获取到发送skb的物理层参数
    cmd->paddr_lo = lower_32_bits(WIRELESS_SIMU_SKB_CB(frame)->paddr);
    cmd->paddr_hi = upper_32_bits(WIRELESS_SIMU_SKB_CB(frame)->paddr);
    cmd->frame_len = frame->len;
    cmd->buf_len = buf_len;
    cmd->tx_params_valid = 0;

    frame_tlv = (struct wmi_tlv *)(skb->data + sizeof(*cmd));
    frame_tlv->header = FIELD_PREP(WMI_TLV_TAG, WMI_TAG_ARRAY_BYTE) | FIELD_PREP(WMI_TLV_LEN, buf_len);
    memcpy(frame_tlv->value, frame->data, buf_len);

    // data大端转小端

    ret = wireless_simu_wmi_cmd_send(priv, skb, WMI_MGMT_TX_SEND_CMDID);

    return ret;
}