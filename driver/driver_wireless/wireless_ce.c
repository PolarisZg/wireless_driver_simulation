#include "wireless.h"

/* 最多(必须)含有 12 个 ce
 * 这个 12 个 ce 不仅是由 CE_COUNT_MAX 指定
 * 还因为 enum hal_srng_ring_id 中 ce 相关的 ring_id
 * 包括 src dst dst_status
 * 仅分配了 12 个
 * 再多会和其他的 ring 冲突 */
const struct ce_attr wireless_simu_ce_config[] = {

    /* CE0 : host --> target */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 32,
        .src_sz_max = 2048,
        .dest_nentries = 0,
    },

    /* CE1 : target --> host */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 2048,
        .dest_nentries = 512,
    },

    /* CE2 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE3 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE4 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE5 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE6 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE7 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE8 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE9 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE10 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },

    /* CE11 : no used */
    {
        .flags = CE_ATTR_FLAGS,
        .src_nentries = 0,
        .src_sz_max = 0,
        .dest_nentries = 0,
    },
};

void wireless_simu_ce_poll_send_completred(struct wireless_simu *priv, u8 pipe_id)
{
    wireless_ce_per_engine_service(priv, pipe_id);
}

static void wireless_hal_ce_src_set_desc(void *buf, dma_addr_t paddr, u32 len, u32 id, u8 byte_swap_data)
{
    struct hal_ce_srng_src_desc *desc = buf;

    desc->buffer_addr_low = paddr & HAL_ADDR_LSB_REG_MASK;
    desc->buffer_addr_info =
        FIELD_PREP(GENMASK(7, 0), ((u64)paddr >> HAL_ADDR_MSB_REG_SHIFT)) |
        FIELD_PREP(BIT(9), byte_swap_data) |
        FIELD_PREP(BIT(11), 0) |
        FIELD_PREP(GENMASK(31, 16), len);
    desc->meta_info = FIELD_PREP(GENMASK(15, 0), id);

    /* 一个用来进行大小端对齐的magic num, 本身无意义 */
    desc->flags = 0x12345678;
}

int wireless_simu_ce_send(struct wireless_simu *priv, struct sk_buff *skb, u8 pipe_id, u16 transfer_id)
{
    struct wireless_simu_ce *ce = &priv->ce;
    struct wireless_simu_ce_pipe *pipe = &ce->ce_pipe[pipe_id];
    struct hal_srng *srng;
    u32 *desc;
    unsigned int write_index, sw_index;
    unsigned int nentries_mask;
    int ret = 0;
    u8 byte_swap_data = 0;
    int num_used;

    if (pipe->attr_flags & CE_ATTR_DIS_INTR)
    {
        spin_lock_bh(&ce->ce_lock);

        if (!pipe->src_ring)
        {
            spin_unlock_bh(&ce->ce_lock);
            return -EINVAL;
        }

        write_index = pipe->src_ring->write_index;

        sw_index = pipe->src_ring->sw_index;

        if (write_index >= sw_index)
        {
            num_used = write_index - sw_index;
        }
        else
        {
            num_used = pipe->src_ring->nentries - sw_index + write_index;
        }

        spin_unlock_bh(&ce->ce_lock);

        if (num_used >= WIRELESS_SIMU_CE_USAGE_THRESHOLD)
            wireless_simu_ce_poll_send_completred(priv, pipe_id);
    }

    /* todo 有一个判断硬件是否在工作的 flag 位, 这里就没写 */

    spin_lock_bh(&ce->ce_lock);

    if (!pipe->src_ring)
    {
        spin_unlock_bh(&ce->ce_lock);
        return -EINVAL;
    }

    write_index = pipe->src_ring->write_index;
    nentries_mask = pipe->src_ring->nentries_mask;

    srng = &priv->hal.srng_list[pipe->src_ring->hal_ring_id];

    spin_lock_bh(&srng->lock);

    wireless_simu_hal_srng_access_begin(priv, srng);

    if (unlikely(wireless_simu_hal_srng_src_num_free(priv, srng, false) < 1))
    {
        ret = -ENOBUFS;
        pr_err("%s : err srng %08x no free entry \n", WIRELESS_SIMU_DEVICE_NAME, srng->ring_id);
        goto exit;
    }

    /* 这里选择使用 reaped 而不是 entry 的原因是
     * 必须要被清空且空闲的位置才能放数据
     * 否则会导致数据重合, 内存泄露
     * */
    desc = wireless_simu_hal_srng_src_get_next_reaped(priv, srng);
    if (!desc)
    {
        ret = -ENOBUFS;
        pr_info("%s : err srng %08x no next reap entry \n", WIRELESS_SIMU_DEVICE_NAME, srng->ring_id);
        goto exit;
    }

    wireless_hal_ce_src_set_desc(desc, WIRELESS_SKB_CB(skb)->paddr, skb->len, transfer_id, byte_swap_data);

    pipe->src_ring->skb[write_index] = skb;
    pipe->src_ring->write_index = (((write_index) + 1) & (nentries_mask));

    wireless_simu_hal_srng_access_end(priv, srng);

    /* todo 关于 timer 的不太懂 */

    spin_lock_bh(&srng->lock);

    spin_unlock_bh(&ce->ce_lock);

    return 0;

exit:
    wireless_simu_hal_srng_access_end(priv, srng);

    spin_unlock_bh(&srng->lock);

    spin_unlock_bh(&ce->ce_lock);

    return ret;
}

/* For Big Endian Host, Copy Engine byte_swap is enabled
 * When Copy Engine does byte_swap, need to byte swap again for the
 * Host to get/put buffer content in the correct byte order
 */
void wireless_simu_ce_byte_swap(void *mem, u32 len)
{
    int i;

    if (IS_ENABLED(CONFIG_CPU_BIG_ENDIAN))
    {
        if (!mem)
            return;

        for (i = 0; i < (len / 4); i++)
        {
            *(u32 *)mem = swab32(*(u32 *)mem);
            mem += 4;
        }
    }
}

static struct sk_buff *wireless_ce_completed_send_next(struct wireless_simu_ce_pipe *pipe)
{
    struct wireless_simu *priv = pipe->priv;
    struct wireless_simu_ce *ce = &priv->ce;
    struct hal_srng *srng;
    unsigned int sw_index;
    unsigned int nentries_mask;
    struct sk_buff *skb = NULL;
    u32 *desc;

    spin_lock_bh(&ce->ce_lock);

    if (!pipe->src_ring)
    {
        spin_unlock_bh(&ce->ce_lock);
        return ERR_PTR(-EINVAL);
    }

    sw_index = pipe->src_ring->sw_index;
    nentries_mask = pipe->src_ring->nentries_mask;

    srng = &priv->hal.srng_list[pipe->src_ring->hal_ring_id];

    spin_lock_bh(&srng->lock);

    wireless_simu_hal_srng_access_begin(priv, srng);

    /* 这里的作用是更新 srng 中的 reap_hp
     * 实际的数据并不会从 srng 中获得
     * 通过返回 NULL 表示没有数据，也表示没有 skb */
    desc = wireless_simu_hal_srng_src_reap_next(priv, srng);
    if (!desc)
    {
        skb = ERR_PTR(-EIO);
        goto err_unlock;
    }

    skb = pipe->src_ring->skb[sw_index];

    pipe->src_ring->skb[sw_index] = NULL;

    sw_index = (((sw_index) + 1) & (nentries_mask));
    pipe->src_ring->sw_index = sw_index;

err_unlock:
    spin_unlock_bh(&srng->lock);

    spin_unlock_bh(&ce->ce_lock);

    return skb;
}

static void wireless_ce_tx_process_cb(struct wireless_simu_ce_pipe *pipe)
{
    struct wireless_simu *priv = pipe->priv;
    struct sk_buff *skb;
    struct sk_buff_head list;

    __skb_queue_head_init(&list);
    while (!IS_ERR(skb = wireless_ce_completed_send_next(pipe)))
    {
        if (!skb)
            continue;

        dma_unmap_single(&priv->pci_dev->dev, WIRELESS_SKB_CB(skb)->paddr, skb->len, DMA_TO_DEVICE);

        /* 即使是释放 mgmt 相关的 skb
         * 也必须有 send_cb 相关，否则这里只会释放一个 htc 相关的 skb
         * 实际的 mgmt skb 还是被泄露掉了 */
        if ((!pipe->send_cb))
        {

            /* 这里的代码之后必须删除 */
            /* 该处的代码是在没有实现完成 rx 的时候添加的代码, 删除掉所有 wmi 帧中指向的 mgmt 帧*/
            struct sk_buff *msdu;
            struct ieee80211_tx_info *info;
            struct wmi_mgmt_send_cmd *cmd;

            cmd = (struct wmi_mgmt_send_cmd *)(skb->data + sizeof(struct wmi_cmd_hdr) + sizeof(struct wireless_htc_hdr));

            spin_lock_bh(&priv->txmgmt_idr_lock);
            msdu = idr_find(&priv->txmgmt_idr, cmd->desc_id);

            if (msdu)
                idr_remove(&priv->txmgmt_idr, cmd->desc_id);
            spin_unlock_bh(&priv->txmgmt_idr_lock);

            if (msdu)
            {
                dma_unmap_single(&priv->pci_dev->dev, WIRELESS_SKB_CB(msdu)->paddr, msdu->len, DMA_TO_DEVICE);

                info = IEEE80211_SKB_CB(msdu);
                if ((!(info->flags & IEEE80211_TX_CTL_NO_ACK)))
                {
                    info->flags |= IEEE80211_TX_STAT_ACK;
                    ;
                }

                ieee80211_tx_status_irqsafe(priv->hw, msdu);

                atomic_dec_if_positive(&priv->num_pending_mgmt_tx);
            }
            /* 以上的代码之后必须删除 */

            dev_kfree_skb_any(skb);
            continue;
        }

        __skb_queue_tail(&list, skb);
    }

    while ((skb = __skb_dequeue(&list)))
    {
        pr_info("%s : tx ce pipe %d len %d \n", WIRELESS_SIMU_DEVICE_NAME, pipe->pipe_num, skb->len);

        pipe->send_cb(priv, skb);
    }
}

static void wireless_ce_recv_process_cb(struct wireless_simu_ce_pipe *pipe)
{
}

void wireless_ce_per_engine_service(struct wireless_simu *priv, u16 id)
{
    struct wireless_simu_ce_pipe *pipe = &priv->ce.ce_pipe[id];
    const struct ce_attr *attr = &priv->ce.host_config[id];

    if (attr->src_nentries)
    {
        wireless_ce_tx_process_cb(pipe);
    }

    if (pipe->recv_cb)
    {
        wireless_ce_recv_process_cb(pipe);
    }
}

static void wireless_ce_tasklet(struct tasklet_struct *t)
{
    struct wireless_simu_ce_pipe *pipe = from_tasklet(pipe, t, intr_tq);
    struct wireless_simu *priv = pipe->priv;

    wireless_ce_per_engine_service(priv, pipe->pipe_num);
}

static int wireless_simu_ce_init_config(struct wireless_simu *priv)
{
    struct wireless_simu_ce *ce = &priv->ce;

    ce->host_config = kmemdup(wireless_simu_ce_config, sizeof(wireless_simu_ce_config), GFP_KERNEL);
    if (!ce->host_config)
    {
        pr_err("%s : ce config copy err \n", WIRELESS_SIMU_DEVICE_NAME);
        return -ENOBUFS;
    }

    ce->ce_count = CE_COUNT_MAX;

    ce->priv = priv;

    /* lock init */

    return 0;
}

static void wireless_simu_ce_deinit_config(struct wireless_simu_ce *ce)
{
    if (!ce->host_config)
        kfree(ce->host_config);

    ce->host_config = NULL;
}

u32 wireless_simu_ce_get_desc_size(enum hal_ce_desc type)
{
    switch (type)
    {
    case HAL_CE_DESC_SRC:
        return sizeof(struct hal_ce_srng_src_desc);
    case HAL_CE_DESC_DST:
        return sizeof(struct hal_ce_srng_dest_desc);
    case HAL_CE_DESC_DST_STATUS:
        return sizeof(struct hal_ce_srng_dst_status_desc);
    }

    return 0;
}

static struct wireless_simu_ce_ring *wireless_ce_alloc_ring(struct wireless_simu *priv, int nentries, int desc_sz)
{
    struct wireless_simu_ce_ring *ring;
    dma_addr_t base_addr;

    ring = kzalloc(struct_size(ring, skb, nentries), GFP_KERNEL);
    if (ring == NULL)
        return ERR_PTR(-ENOMEM);

    ring->nentries = nentries;
    ring->nentries_mask = nentries - 1;

    ring->base_addr_owner_space_unaligned = dma_alloc_coherent(&priv->pci_dev->dev,
                                                               nentries * desc_sz + CE_DESC_RING_ALIGN,
                                                               &base_addr, GFP_KERNEL);
    if (!ring->base_addr_owner_space_unaligned)
    {
        kfree(ring);
        return ERR_PTR(-ENOMEM);
    }
    memset(ring->base_addr_owner_space_unaligned, 0, nentries * desc_sz + CE_DESC_RING_ALIGN);
    ring->dma_size = nentries * desc_sz + CE_DESC_RING_ALIGN;

    ring->base_addr_ce_space_unaligned = base_addr;

    ring->base_addr_owner_space = PTR_ALIGN(ring->base_addr_owner_space_unaligned, CE_DESC_RING_ALIGN);
    ring->base_addr_ce_space = ALIGN(ring->base_addr_ce_space_unaligned, CE_DESC_RING_ALIGN);

    pr_info("%s : ce ring %p vaddr %llx paddr \n", WIRELESS_SIMU_DEVICE_NAME, ring->base_addr_owner_space, ring->base_addr_ce_space);

    return ring;
}

static int wireless_ce_alloc_pipe(struct wireless_simu_ce *ce, int pipe_id)
{
    struct wireless_simu_ce_pipe *pipe = &ce->ce_pipe[pipe_id];
    const struct ce_attr *attr = &ce->host_config[pipe_id];
    struct wireless_simu_ce_ring *ring;
    int nentries;
    int desc_sz;

    pipe->attr_flags = attr->flags;

    if (attr->src_nentries)
    {
        pipe->send_cb = attr->send_cb;
        nentries = roundup_pow_of_two(attr->src_nentries);
        desc_sz = wireless_simu_ce_get_desc_size(HAL_CE_DESC_SRC);
        ring = wireless_ce_alloc_ring(ce->priv, nentries, desc_sz);
        if (IS_ERR(ring))
            return PTR_ERR(ring);
        pipe->src_ring = ring;
    }

    if (attr->dest_nentries)
    {
        pipe->send_cb = attr->send_cb;
        pipe->recv_cb = attr->recv_cb;

        nentries = roundup_pow_of_two(attr->dest_nentries);
        desc_sz = wireless_simu_ce_get_desc_size(HAL_CE_DESC_DST);
        ring = wireless_ce_alloc_ring(ce->priv, nentries, desc_sz);
        if (IS_ERR(ring))
            return PTR_ERR(ring);
        pipe->dest_ring = ring;

        desc_sz = wireless_simu_ce_get_desc_size(HAL_CE_DESC_DST_STATUS);
        ring = wireless_ce_alloc_ring(ce->priv, nentries, desc_sz);
        if (IS_ERR(ring))
            return PTR_ERR(ring);
        pipe->status_ring = ring;
    }

    return 0;
}

void wireless_simu_ce_free_pipes(struct wireless_simu *priv)
{
    struct wireless_simu_ce_pipe *pipe;
    struct wireless_simu_ce_ring *ring;
    // int desc_sz; 用了 dma_size 存储 dma 空间, 本函数中无需再次计算
    int i;

    for (i = 0; i < priv->ce.ce_count; i++)
    {
        pipe = &priv->ce.ce_pipe[i];

        /* timer 这里没有 */

        if (pipe->src_ring)
        {
            ring = pipe->src_ring;
            dma_free_coherent(&priv->pci_dev->dev,
                              ring->dma_size,
                              ring->base_addr_owner_space_unaligned,
                              ring->base_addr_ce_space_unaligned);
            kfree(ring);
            pipe->src_ring = NULL;
        }

        if (pipe->dest_ring)
        {
            ring = pipe->dest_ring;
            dma_free_coherent(&priv->pci_dev->dev,
                              ring->dma_size,
                              ring->base_addr_owner_space_unaligned,
                              ring->base_addr_ce_space_unaligned);
            kfree(ring);
            pipe->src_ring = NULL;
        }

        if (pipe->status_ring)
        {
            ring = pipe->status_ring;
            dma_free_coherent(&priv->pci_dev->dev,
                              ring->dma_size,
                              ring->base_addr_owner_space_unaligned,
                              ring->base_addr_ce_space_unaligned);
            kfree(ring);
            pipe->src_ring = NULL;
        }
    }

    wireless_simu_ce_deinit_config(&priv->ce);
}

int wireless_simu_ce_alloc_pipes(struct wireless_simu *priv)
{
    /* 先把 ce 初始化一下
     * ath11k 中 ce 的各种配置参数是放在 ath11k_base 的 hw_params 之中的
     * 这里为了精简, 全放在了 ce 中
     * */
    int ret = 0;
    ret = wireless_simu_ce_init_config(priv);
    if (ret)
    {
        return ret;
    }

    struct wireless_simu_ce *ce = &priv->ce;
    struct wireless_simu_ce_pipe *pipe;
    struct ce_attr *attr;
    int i;

    spin_lock_init(&ce->ce_lock);

    for (i = 0; i < ce->ce_count; i++)
    {
        attr = &ce->host_config[i];
        pipe = &ce->ce_pipe[i];
        pipe->pipe_num = i;
        pipe->priv = priv;
        pipe->buf_sz = attr->src_sz_max;

        ret = wireless_ce_alloc_pipe(ce, i);

        if (ret)
        {
            /* free all successful alloc pipes */
            wireless_simu_ce_free_pipes(priv);
            goto err_free_config;
        }

        /* 这个我也不知道应该写在哪里 */
        tasklet_setup(&pipe->intr_tq, wireless_ce_tasklet);
    }

    return 0;

err_free_config:
    wireless_simu_ce_deinit_config(ce);
    return ret;
}

static int wireless_ce_init_ring(struct wireless_simu *priv, struct wireless_simu_ce_ring *ring, int ce_id, enum hal_ring_type type)
{
    struct hal_srng_params params = {0};
    int ret = 0;

    // 填充params
    params.ring_base_paddr = ring->base_addr_ce_space;
    params.ring_base_vaddr = ring->base_addr_owner_space;
    params.num_entries = ring->nentries;

    // msi 中断部分, 不用管

    switch (type)
    {
    case HAL_CE_SRC:
        if (!(CE_ATTR_DIS_INTR & priv->ce.host_config[ce_id].flags))
            params.intr_batch_cntr_thres_entries = 1;
        break;
    case HAL_CE_DST:
        params.max_buffer_len = priv->ce.host_config[ce_id].src_sz_max;
        if (!(priv->ce.host_config[ce_id].flags & CE_ATTR_DIS_INTR))
        {
            params.intr_timer_thres_us = 1024;
            params.flags |= HAL_SRNG_FLAGS_LOW_THRESH_INTR_EN;
            params.low_threshold = ring->nentries - 3;
        }
        break;
    case HAL_CE_DST_STATUS:
        if (!(priv->ce.host_config[ce_id].flags & CE_ATTR_DIS_INTR))
        {
            params.intr_batch_cntr_thres_entries = 1;
            params.intr_timer_thres_us = 0x1000;
        }
        break;
    default:
        pr_err("%s : Invalid CE ring type %d \n", WIRELESS_SIMU_DEVICE_NAME, type);
        return -EINVAL;
    }

    ret = wireless_simu_hal_srng_setup(priv, type, ce_id, 0, &params);
    if (ret < 0)
    {
        pr_err("%s : fail to setup ce ring %d ret %d \n", WIRELESS_SIMU_DEVICE_NAME, ce_id, ret);
        return ret;
    }

    ring->hal_ring_id = ret;

    return 0;
}

int wireless_simu_ce_init_pipes(struct wireless_simu *priv)
{
    struct wireless_simu_ce_pipe *pipe;
    int i;
    int ret;

    for (i = 0; i < priv->ce.ce_count; i++)
    {
        pipe = &priv->ce.ce_pipe[i];

        if (pipe->src_ring)
        {
            ret = wireless_ce_init_ring(priv, pipe->src_ring, i, HAL_CE_SRC);

            if (ret)
            {
                pr_err("%s : fail to init ce src ring pipe %d ret %d \n", WIRELESS_SIMU_DEVICE_NAME, i, ret);
                return ret;
            }

            pipe->src_ring->write_index = 0;
            pipe->src_ring->sw_index = 0;

            pr_info("%s : pipe num %d src ring id %d \n", WIRELESS_SIMU_DEVICE_NAME, i, pipe->src_ring->hal_ring_id);
        }

        if (pipe->dest_ring)
        {
            ret = wireless_ce_init_ring(priv, pipe->dest_ring, i, HAL_CE_DST);

            if (ret)
            {
                pr_err("%s : fail to init ce dest ring pipe %d ret %d \n", WIRELESS_SIMU_DEVICE_NAME, i, ret);
                return ret;
            }

            pipe->rx_buf_needed = pipe->dest_ring->nentries ? pipe->dest_ring->nentries - 2 : 0;

            pipe->dest_ring->write_index = 0;
            pipe->dest_ring->sw_index = 0;

            pr_info("%s : pipe num %d dest ring id %d \n", WIRELESS_SIMU_DEVICE_NAME, i, pipe->dest_ring->hal_ring_id);
        }

        if (pipe->status_ring)
        {
            ret = wireless_ce_init_ring(priv, pipe->status_ring, i, HAL_CE_DST_STATUS);

            if (ret)
            {
                pr_err("%s : fail to init ce dest status ring pipe %d ret %d \n", WIRELESS_SIMU_DEVICE_NAME, i, ret);
                return ret;
            }

            pipe->status_ring->write_index = 0;
            pipe->status_ring->sw_index = 0;

            pr_info("%s : pipe num %d dest status ring id %d \n", WIRELESS_SIMU_DEVICE_NAME, i, pipe->status_ring->hal_ring_id);
        }
    }

    return 0;
}

/* 删除所有 ce 相关的空间 配置 等数据
 * 没处理 src ring 中的 wmi frame 指向的 mgmt 数据 */
void wireless_simu_ce_stop(struct wireless_simu *priv)
{
    struct wireless_simu_ce *ce = &priv->ce;
    struct wireless_simu_ce_pipe *pipe;

    int i;

    // 在这里面对 pipe 进行清理

    spin_lock_bh(&ce->ce_lock);

    for (i = 0; i < ce->ce_count; i++)
    {
        pipe = &ce->ce_pipe[i];

        tasklet_kill(&pipe->intr_tq);
    }

    wireless_simu_ce_free_pipes(priv);

    kfree(ce->host_config);

    ce->host_config = NULL;

    spin_unlock(&ce->ce_lock);
}