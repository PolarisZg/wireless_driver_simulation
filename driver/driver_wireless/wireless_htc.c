#include "wireless.h"

struct sk_buff *wireless_simu_htc_alloc_skb(struct wireless_simu *priv, int size)
{
    struct sk_buff *skb;

    skb = dev_alloc_skb(size + sizeof(struct wireless_simu_htc_hdr));
    if (!skb)
        return NULL;

    skb_reserve(skb, sizeof(struct wireless_simu_htc_hdr));

    if (!IS_ALIGNED((unsigned long)skb->data, 4))
        pr_warn("%s : mac htc alloc skb unaligned \n", WIRELESS_SIMU_DEVICE_NAME);

    return skb;
}

int wireless_simu_htc_send(struct wireless_simu *priv, struct sk_buff *skb)
{
    int ret = 0;
    struct device *dev = &priv->pci_dev->dev;
    struct wireless_simu_skb_cb *skb_cb = WIRELESS_SIMU_SKB_CB(skb);
    struct wireless_simu_htc_hdr *hdr;

    // 没有endpoint这个参数

    skb_push(skb, sizeof(struct wireless_simu_htc_hdr));

    // credit_flow 不知道有什么用，不影响简单的ce传输

    hdr = (struct wireless_simu_htc_hdr *)skb->data;
    memset(hdr, 0, sizeof(*hdr));
    hdr->htc_info = FIELD_PREP(HTC_HDR_ENDPOINTID, 0) |
                    FIELD_PREP(HTC_HDR_PAYLOADLEN, skb->len - sizeof(*hdr));
    
    spin_lock_bh(&priv->htc_tx_lock);
    // 这里本来是有一个对ep的计数的，但是我连ep都没，就不计数了
    hdr->ctrl_info = 0;
    spin_unlock_bh(&priv->htc_tx_lock);

    skb_cb->paddr = dma_map_single(dev, skb->data, skb->len, DMA_TO_DEVICE);
    ret = dma_mapping_error(dev, skb_cb->paddr);
    if(ret){
        ret = -EIO;
        goto err_pull;
    }

    ret = wireless_simu_ce_send(priv, skb);
    if(ret)
        goto err_unmap;

    return ret;

err_unmap:
    dma_unmap_single(dev, skb_cb->paddr, skb->len, DMA_TO_DEVICE);
err_pull:
    skb_pull(skb, sizeof(struct wireless_simu_htc_hdr));
    return ret;
}