#ifndef WIRELESS_SIMU_CE
#define WIRELESS_SIMU_CE

#include "wireless.h"
#include "wireless_hal.h"

/* no interrupt on copy completion */
#define CE_ATTR_DIS_INTR 8

#define CE_COUNT_MAX 12

/* Byte swap data words */
#define CE_ATTR_BYTE_SWAP_DATA 2

/* Host software's Copy Engine configuration. */
#ifdef __BIG_ENDIAN
#define CE_ATTR_FLAGS CE_ATTR_BYTE_SWAP_DATA
#else
#define CE_ATTR_FLAGS 0
#endif

#define WIRELESS_SIMU_CE_USAGE_THRESHOLD 32

struct ce_attr
{
    unsigned int flags;

    /* src entry 数量 */
    unsigned int src_nentries;

    /* 每一个 entry 的最大大小, 虽然名为src, 但dst的也可以用这个参数来设定entry大小限制 */
    unsigned int src_sz_max;

    /* dst entry 数量 */
    unsigned int dest_nentries;

    void (*recv_cb)(struct wireless_simu *priv, struct sk_buff *skb);
    void (*send_cb)(struct wireless_simu *priv, struct sk_buff *skb);
};

struct wireless_simu_ce_ring
{
    /* ring 中 entries 数量 */
    unsigned int nentries;
    unsigned int nentries_mask;

    /* 对 src 环，该 index 指向最后一个被放入 ring 的 descriptor ;
     *
     * 对 dst 环， 该index 指向下一个需要被处理的entry;
     * */
    unsigned int sw_index;

    unsigned int write_index;

    /* 为 entries alloc 的 memory 空间*/
    /* 逻辑内存地址 */
    void *base_addr_owner_space_unaligned;
    /* 物理内存地址 */
    dma_addr_t base_addr_ce_space_unaligned;

    /* dma malloc size */
    size_t dma_size;

    /* 经由内存对齐之后的，为 entries alloc 的 memory 地址
     * 这就产生了问题，当使用ALIGN进行对齐的时候，一方面会导致void *被修改，使整个内存空间变少，另一个dmaaddr和void *未必有关联，那万一一个改了另一个没改不就发生冲突了吗？*/
    /* 逻辑内存地址 */
    void *base_addr_owner_space;
    /* 物理内存地址 */
    dma_addr_t base_addr_ce_space;

    /* hal ring id */
    u32 hal_ring_id;

    /* keep last */
    struct sk_buff *skb[];
};

struct wireless_simu_ce_pipe
{
    struct wireless_simu *priv;
    u16 pipe_num;
    unsigned int attr_flags;
    unsigned int buf_sz;
    unsigned int rx_buf_needed;

    void (*recv_cb)(struct wireless_simu *priv, struct sk_buff *skb);
    void (*send_cb)(struct wireless_simu *priv, struct sk_buff *skb);

    struct tasklet_struct intr_tq;

    /* 向hw传输数据 */
    struct wireless_simu_ce_ring *src_ring;

    /* 填充sw中申请的dma地址 */
    struct wireless_simu_ce_ring *dest_ring;

    /* 填充有rx的数据 */
    struct wireless_simu_ce_ring *status_ring;

    u64 timestamp;
};

struct wireless_simu_ce
{
    /* 一些 ce 的 配置信息 */
    u32 ce_count;
    struct ce_attr *host_config;
    struct wireless_simu *priv;

    struct wireless_simu_ce_pipe ce_pipe[CE_COUNT_MAX];
    spinlock_t ce_lock;
    // 不懂的一个timer
};

extern const struct ce_attr wireless_simu_ce_config[];

int wireless_simu_ce_send(struct wireless_simu *priv, struct sk_buff *skb, u8 pipe_id, u16 transfer_id);

void wireless_simu_ce_byte_swap(void *mem, u32 len);

int wireless_simu_ce_alloc_pipes(struct wireless_simu *priv);

void wireless_simu_ce_free_pipes(struct wireless_simu *priv);

u32 wireless_simu_ce_get_desc_size(enum hal_ce_desc type);

int wireless_simu_ce_init_pipes(struct wireless_simu *priv);

void wireless_simu_ce_poll_send_completred(struct wireless_simu *priv, u8 pipe_id);

void wireless_simu_ce_stop(struct wireless_simu *priv);

void wireless_ce_per_engine_service(struct wireless_simu *priv, u16 id);

#endif