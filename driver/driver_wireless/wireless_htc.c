#include "wireless.h"

/* htc 申请空间函数内部无 skb_put 需要高级模块自行推进 data 字段
 */
struct sk_buff *wireless_simu_htc_alloc_skb(struct wireless_simu *priv, int size)
{
    struct sk_buff *skb = NULL;

    skb = dev_alloc_skb(size + sizeof(struct wireless_htc_hdr));
    if (!skb)
        return NULL;

    skb_reserve(skb, sizeof(struct wireless_htc_hdr));

    if (!IS_ALIGNED((unsigned long)skb->data, 4))
        pr_warn("%s : mac htc alloc skb unaligned \n", WIRELESS_SIMU_DEVICE_NAME);

    return skb;
}

static void wireless_htc_prepare_tx_skb(struct wireless_simu *priv, struct sk_buff *skb)
{
    struct wireless_htc_hdr *hdr;

    hdr = (struct wireless_htc_hdr *)skb->data;

    memset(hdr, 0, sizeof(*hdr));
    hdr->htc_info = FIELD_PREP(HTC_HDR_ENDPOINTID, 0) |
                    FIELD_PREP(HTC_HDR_PAYLOADLEN, skb->len - sizeof(*hdr));

    // credit_flow 不知道有什么用

    spin_lock_bh(&priv->ep_htc_txlock);
    // 这里本来是有一个对ep的计数的，但是我连ep都没，就不计数了
    hdr->ctrl_info = 0;
    spin_unlock_bh(&priv->ep_htc_txlock);
}

int wireless_simu_htc_send(struct wireless_simu *priv, struct sk_buff *skb)
{
    int ret = 0;
    struct device *dev = &priv->pci_dev->dev;
    struct wireless_skb_cb *skb_cb = WIRELESS_SKB_CB(skb);
    // struct wireless_htc_hdr *hdr;

    // 没有endpoint这个参数

    skb_push(skb, sizeof(struct wireless_htc_hdr));

    // credit_flow 不知道有什么用，不影响简单的ce传输

    wireless_htc_prepare_tx_skb(priv, skb);

    skb_cb->paddr = dma_map_single(dev, skb->data, skb->len, DMA_TO_DEVICE);
    ret = dma_mapping_error(dev, skb_cb->paddr);
    if (ret)
    {
        ret = -EIO;
        goto err_pull;
    }

    ret = wireless_simu_ce_send(priv, skb, 0, 0);
    if (ret)
        goto err_unmap;

    return ret;

err_unmap:
    dma_unmap_single(dev, skb_cb->paddr, skb->len, DMA_TO_DEVICE);
err_pull:
    skb_pull(skb, sizeof(struct wireless_htc_hdr));
    return ret;
}