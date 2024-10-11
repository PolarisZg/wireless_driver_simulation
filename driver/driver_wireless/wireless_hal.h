#ifndef WIRELESS_SIMU_HAL
#define WIRELESS_SIMU_HAL

#include "wireless.h"

/* TCL ring field mask and offset */
#define HAL_TCL1_RING_BASE_MSB_RING_SIZE		GENMASK(27, 8)
#define HAL_TCL1_RING_BASE_MSB_RING_BASE_ADDR_MSB	GENMASK(7, 0)
#define HAL_TCL1_RING_ID_ENTRY_SIZE			GENMASK(7, 0)
#define HAL_TCL1_RING_MISC_MSI_LOOPCNT_DISABLE		BIT(1)
#define HAL_TCL1_RING_MISC_MSI_SWAP			BIT(3)
#define HAL_TCL1_RING_MISC_HOST_FW_SWAP			BIT(4)
#define HAL_TCL1_RING_MISC_DATA_TLV_SWAP		BIT(5)
#define HAL_TCL1_RING_MISC_SRNG_ENABLE			BIT(6)
#define HAL_TCL1_RING_CONSR_INT_SETUP_IX0_INTR_TMR_THOLD   GENMASK(31, 16)
#define HAL_TCL1_RING_CONSR_INT_SETUP_IX0_BATCH_COUNTER_THOLD GENMASK(14, 0)
#define HAL_TCL1_RING_CONSR_INT_SETUP_IX1_LOW_THOLD	GENMASK(15, 0)
#define HAL_TCL1_RING_MSI1_BASE_MSB_MSI1_ENABLE		BIT(8)
#define HAL_TCL1_RING_MSI1_BASE_MSB_ADDR		GENMASK(7, 0)
#define HAL_TCL1_RING_CMN_CTRL_DSCP_TID_MAP_PROG_EN	BIT(17)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP		GENMASK(31, 0)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP0		GENMASK(2, 0)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP1		GENMASK(5, 3)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP2		GENMASK(8, 6)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP3		GENMASK(11, 9)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP4		GENMASK(14, 12)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP5		GENMASK(17, 15)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP6		GENMASK(20, 18)
#define HAL_TCL1_RING_FIELD_DSCP_TID_MAP7		GENMASK(23, 21)

/* REO ring field mask and offset */
#define HAL_REO1_RING_BASE_MSB_RING_SIZE		GENMASK(27, 8)
#define HAL_REO1_RING_BASE_MSB_RING_BASE_ADDR_MSB	GENMASK(7, 0)
#define HAL_REO1_RING_ID_RING_ID			GENMASK(15, 8)
#define HAL_REO1_RING_ID_ENTRY_SIZE			GENMASK(7, 0)
#define HAL_REO1_RING_MISC_MSI_SWAP			BIT(3)
#define HAL_REO1_RING_MISC_HOST_FW_SWAP			BIT(4)
#define HAL_REO1_RING_MISC_DATA_TLV_SWAP		BIT(5)
#define HAL_REO1_RING_MISC_SRNG_ENABLE			BIT(6)
#define HAL_REO1_RING_PRDR_INT_SETUP_INTR_TMR_THOLD	GENMASK(31, 16)
#define HAL_REO1_RING_PRDR_INT_SETUP_BATCH_COUNTER_THOLD GENMASK(14, 0)
#define HAL_REO1_RING_MSI1_BASE_MSB_MSI1_ENABLE		BIT(8)
#define HAL_REO1_RING_MSI1_BASE_MSB_ADDR		GENMASK(7, 0)
#define HAL_REO1_GEN_ENABLE_FRAG_DST_RING		GENMASK(25, 23)
#define HAL_REO1_GEN_ENABLE_AGING_LIST_ENABLE		BIT(2)
#define HAL_REO1_GEN_ENABLE_AGING_FLUSH_ENABLE		BIT(3)
#define HAL_REO1_MISC_CTL_FRAGMENT_DST_RING		GENMASK(20, 17)

#define HAL_ADDR_LSB_REG_MASK				0xffffffff

#define HAL_ADDR_MSB_REG_SHIFT				32

/* SRNG registers are split into two groups R0 and R2 */
#define HAL_SRNG_REG_GRP_R0 0
#define HAL_SRNG_REG_GRP_R2 1
#define HAL_SRNG_NUM_REG_GRP 2

#define HAL_REO_REO2SW1_RING_BASE_MSB_RING_SIZE		0x000fffff
#define HAL_REO_REO2TCL_RING_BASE_MSB_RING_SIZE		0x000fffff
#define HAL_REO_SW2REO_RING_BASE_MSB_RING_SIZE		0x0000ffff
#define HAL_REO_CMD_RING_BASE_MSB_RING_SIZE		0x0000ffff
#define HAL_REO_STATUS_RING_BASE_MSB_RING_SIZE		0x0000ffff
#define HAL_SW2TCL1_RING_BASE_MSB_RING_SIZE		0x000fffff
#define HAL_SW2TCL1_CMD_RING_BASE_MSB_RING_SIZE		0x000fffff
#define HAL_TCL_STATUS_RING_BASE_MSB_RING_SIZE		0x0000ffff
#define HAL_CE_SRC_RING_BASE_MSB_RING_SIZE		0x0000ffff
#define HAL_CE_DST_RING_BASE_MSB_RING_SIZE		0x0000ffff
#define HAL_CE_DST_STATUS_RING_BASE_MSB_RING_SIZE	0x0000ffff
#define HAL_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE	0x0000ffff
#define HAL_SW2WBM_RELEASE_RING_BASE_MSB_RING_SIZE	0x0000ffff
#define HAL_WBM2SW_RELEASE_RING_BASE_MSB_RING_SIZE	0x000fffff
#define HAL_RXDMA_RING_MAX_SIZE				0x0000ffff
#define HAL_TEST_SW2HW_SIZE 0x0000ffff

enum hal_srng_dir
{
    HAL_SRNG_DIR_SRC,
    HAL_SRNG_DIR_DST
};

/* srng flags */
#define HAL_SRNG_FLAGS_MSI_SWAP			0x00000008
#define HAL_SRNG_FLAGS_RING_PTR_SWAP		0x00000010
#define HAL_SRNG_FLAGS_DATA_TLV_SWAP		0x00000020
#define HAL_SRNG_FLAGS_LOW_THRESH_INTR_EN	0x00010000
#define HAL_SRNG_FLAGS_MSI_INTR			0x00020000
#define HAL_SRNG_FLAGS_CACHED                   0x20000000
#define HAL_SRNG_FLAGS_LMAC_RING		0x80000000
#define HAL_SRNG_FLAGS_REMAP_CE_RING        0x10000000

#define HAL_SRNG_TLV_HDR_TAG		GENMASK(9, 1)
#define HAL_SRNG_TLV_HDR_LEN		GENMASK(25, 10)

/* Common SRNG ring structure for source and destination rings */
struct hal_srng
{
    /* Unique SRNG ring ID */
    u8 ring_id;

    /* Ring initialization done */
    u8 initialized;

    /* Interrupt/MSI value assigned to this ring */
    int irq;

    /* Physical base address of the ring */
    dma_addr_t ring_base_paddr;

    /* Virtual base address of the ring */
    u32 *ring_base_vaddr;

    /* Number of entries in ring */
    u32 num_entries;

    /* Ring size */
    u32 ring_size;

    /* Ring size mask */
    u32 ring_size_mask;

    /* Size of ring entry */
    u32 entry_size;

    /* Interrupt timer threshold - in micro seconds */
    u32 intr_timer_thres_us;

    /* Interrupt batch counter threshold - in number of ring entries */
    u32 intr_batch_cntr_thres_entries;

    /* MSI Address */
    dma_addr_t msi_addr;

    /* MSI data */
    u32 msi_data;

    /* Misc flags */
    u32 flags;

    /* Lock for serializing ring index updates */
    spinlock_t lock;

    /* Start offset of SRNG register groups for this ring
     * TBD: See if this is required - register address can be derived
     * from ring ID
     */
    u32 hwreg_base[HAL_SRNG_NUM_REG_GRP];

    u64 timestamp;

    /* Source or Destination ring */
    enum hal_srng_dir ring_dir;

    union
    {
        struct
        {
            /* SW tail pointer */
            u32 tp;

            /* Shadow head pointer location to be updated by HW */
            volatile u32 *hp_addr;

            /* Cached head pointer */
            u32 cached_hp;

            /* Tail pointer location to be updated by SW - This
             * will be a register address and need not be
             * accessed through SW structure
             */
            u32 *tp_addr;

            /* Current SW loop cnt */
            u32 loop_cnt;

            /* max transfer size */
            u16 max_buffer_length;

            /* head pointer at access end */
            u32 last_hp;
        } dst_ring;

        struct
        {
            /* SW head pointer */
            u32 hp;

            /* SW reap head pointer */
            u32 reap_hp;

            /* Shadow tail pointer location to be updated by HW */
            u32 *tp_addr;

            /* Cached tail pointer */
            u32 cached_tp;

            /* Head pointer location to be updated by SW - This
             * will be a register address and need not be accessed
             * through SW structure
             */
            u32 *hp_addr;

            /* Low threshold - in number of ring entries */
            u32 low_threshold;

            /* tail pointer at access end */
            u32 last_tp;
        } src_ring;
    } u;
};

/*
 * 该枚举要和 static const struct hal_srng_config hw_srng_config_template[] 中的预定义数据类型一一对应，
 * hal_srng_create_config会读取预定义数据并根据该枚举中的顺序对数据进行访问
 */
enum hal_ring_type
{
    HAL_TEST_SRNG,
    // HAL_REO_DST,
    // HAL_REO_EXCEPTION,
    // HAL_REO_REINJECT,
    // HAL_REO_CMD,
    // HAL_REO_STATUS,
    // HAL_TCL_DATA,
    // HAL_TCL_CMD,
    // HAL_TCL_STATUS,
    // HAL_CE_SRC,
    // HAL_CE_DST,
    // HAL_CE_DST_STATUS,
    // HAL_WBM_IDLE_LINK,
    // HAL_SW2WBM_RELEASE,
    // HAL_WBM2SW_RELEASE,
    // HAL_RXDMA_BUF,
    // HAL_RXDMA_DST,
    // HAL_RXDMA_MONITOR_BUF,
    // HAL_RXDMA_MONITOR_STATUS,
    // HAL_RXDMA_MONITOR_DST,
    // HAL_RXDMA_MONITOR_DESC,
    // HAL_RXDMA_DIR_BUF,
    // HAL_MAX_RING_TYPES,
};

struct hal_srng_params
{
    dma_addr_t ring_base_paddr;
    u32 *ring_base_vaddr;
    int num_entries;
    u32 intr_batch_cntr_thres_entries;
    u32 intr_timer_thres_us;
    u32 flags;
    u32 max_buffer_len;
    u32 low_threshold;
    dma_addr_t msi_addr;
    u32 msi_data;

    /* Add more params as needed */
};

enum hal_srng_ring_id
{
    HAL_SRNG_RING_ID_REO2SW1 = 0,
    HAL_SRNG_RING_ID_REO2SW2,
    HAL_SRNG_RING_ID_REO2SW3,
    HAL_SRNG_RING_ID_REO2SW4,
    HAL_SRNG_RING_ID_REO2TCL,
    HAL_SRNG_RING_ID_SW2REO,

    HAL_SRNG_RING_ID_REO_CMD = 8,
    HAL_SRNG_RING_ID_REO_STATUS,

    HAL_SRNG_RING_ID_SW2TCL1 = 16,
    HAL_SRNG_RING_ID_SW2TCL2,
    HAL_SRNG_RING_ID_SW2TCL3,
    HAL_SRNG_RING_ID_SW2TCL4,

    HAL_SRNG_RING_ID_SW2TCL_CMD = 24,
    HAL_SRNG_RING_ID_TCL_STATUS,

    HAL_SRNG_RING_ID_CE0_SRC = 32,
    HAL_SRNG_RING_ID_CE1_SRC,
    HAL_SRNG_RING_ID_CE2_SRC,
    HAL_SRNG_RING_ID_CE3_SRC,
    HAL_SRNG_RING_ID_CE4_SRC,
    HAL_SRNG_RING_ID_CE5_SRC,
    HAL_SRNG_RING_ID_CE6_SRC,
    HAL_SRNG_RING_ID_CE7_SRC,
    HAL_SRNG_RING_ID_CE8_SRC,
    HAL_SRNG_RING_ID_CE9_SRC,
    HAL_SRNG_RING_ID_CE10_SRC,
    HAL_SRNG_RING_ID_CE11_SRC,

    HAL_SRNG_RING_ID_CE0_DST = 56,
    HAL_SRNG_RING_ID_CE1_DST,
    HAL_SRNG_RING_ID_CE2_DST,
    HAL_SRNG_RING_ID_CE3_DST,
    HAL_SRNG_RING_ID_CE4_DST,
    HAL_SRNG_RING_ID_CE5_DST,
    HAL_SRNG_RING_ID_CE6_DST,
    HAL_SRNG_RING_ID_CE7_DST,
    HAL_SRNG_RING_ID_CE8_DST,
    HAL_SRNG_RING_ID_CE9_DST,
    HAL_SRNG_RING_ID_CE10_DST,
    HAL_SRNG_RING_ID_CE11_DST,

    HAL_SRNG_RING_ID_CE0_DST_STATUS = 80,
    HAL_SRNG_RING_ID_CE1_DST_STATUS,
    HAL_SRNG_RING_ID_CE2_DST_STATUS,
    HAL_SRNG_RING_ID_CE3_DST_STATUS,
    HAL_SRNG_RING_ID_CE4_DST_STATUS,
    HAL_SRNG_RING_ID_CE5_DST_STATUS,
    HAL_SRNG_RING_ID_CE6_DST_STATUS,
    HAL_SRNG_RING_ID_CE7_DST_STATUS,
    HAL_SRNG_RING_ID_CE8_DST_STATUS,
    HAL_SRNG_RING_ID_CE9_DST_STATUS,
    HAL_SRNG_RING_ID_CE10_DST_STATUS,
    HAL_SRNG_RING_ID_CE11_DST_STATUS,

    HAL_SRNG_RING_ID_WBM_IDLE_LINK = 104,
    HAL_SRNG_RING_ID_WBM_SW_RELEASE,
    HAL_SRNG_RING_ID_WBM2SW0_RELEASE,
    HAL_SRNG_RING_ID_WBM2SW1_RELEASE,
    HAL_SRNG_RING_ID_WBM2SW2_RELEASE,
    HAL_SRNG_RING_ID_WBM2SW3_RELEASE,
    HAL_SRNG_RING_ID_WBM2SW4_RELEASE,

    HAL_SRNG_RING_ID_TEST_SW2HW = 125,

    HAL_SRNG_RING_ID_UMAC_ID_END = 127,
    HAL_SRNG_RING_ID_LMAC1_ID_START,

    HAL_SRNG_RING_ID_WMAC1_SW2RXDMA0_BUF = HAL_SRNG_RING_ID_LMAC1_ID_START,
    HAL_SRNG_RING_ID_WMAC1_SW2RXDMA1_BUF,
    HAL_SRNG_RING_ID_WMAC1_SW2RXDMA2_BUF,
    HAL_SRNG_RING_ID_WMAC1_SW2RXDMA0_STATBUF,
    HAL_SRNG_RING_ID_WMAC1_SW2RXDMA1_STATBUF,
    HAL_SRNG_RING_ID_WMAC1_RXDMA2SW0,
    HAL_SRNG_RING_ID_WMAC1_RXDMA2SW1,
    HAL_SRNG_RING_ID_WMAC1_SW2RXDMA1_DESC,
    HAL_SRNG_RING_ID_RXDMA_DIR_BUF,

    HAL_SRNG_RING_ID_LMAC1_ID_END = 143
};

/* SRNG registers are split into two groups R0 and R2 */
#define HAL_SRNG_REG_GRP_R0 0
#define HAL_SRNG_REG_GRP_R2 1
#define HAL_SRNG_NUM_REG_GRP 2

#define HAL_SRNG_NUM_LMACS 3
#define HAL_SRNG_REO_EXCEPTION HAL_SRNG_RING_ID_REO2SW1
#define HAL_SRNG_RINGS_PER_LMAC (HAL_SRNG_RING_ID_LMAC1_ID_END - \
                                 HAL_SRNG_RING_ID_LMAC1_ID_START)
#define HAL_SRNG_NUM_LMAC_RINGS (HAL_SRNG_NUM_LMACS * HAL_SRNG_RINGS_PER_LMAC)
#define HAL_SRNG_RING_ID_MAX (HAL_SRNG_RING_ID_UMAC_ID_END + \
                              HAL_SRNG_NUM_LMAC_RINGS)

#define HAL_SHADOW_NUM_REGS 36

/* HW SRNG configuration table */
struct hal_srng_config
{
    int start_ring_id;
    u16 max_rings;
    u16 entry_size;
    u32 reg_start[HAL_SRNG_NUM_REG_GRP];
    u16 reg_size[HAL_SRNG_NUM_REG_GRP];
    u8 lmac_ring;
    enum hal_srng_dir ring_dir;
    u32 max_size;
};

/*
 * 名为hal的srng子模块
 */
struct wireless_simu_hal
{
    /* HAL internal state for all SRNG rings.
     */
    struct hal_srng srng_list[HAL_SRNG_RING_ID_MAX];

    /* SRNG configuration table */
    struct hal_srng_config *srng_config;

    struct device *dev;

    /* Remote pointer memory for HW/FW updates */
    struct
    {
        u32 *vaddr;
        dma_addr_t paddr;
    } rdp;

    /* Shared memory for ring pointer updates from host to FW */
    struct
    {
        u32 *vaddr;
        dma_addr_t paddr;
    } wrp;

    /* Available REO blocking resources bitmap */
    u8 avail_blk_resource;

    u8 current_blk_index;

    /* shadow register configuration */
    u32 shadow_reg_addr[HAL_SHADOW_NUM_REGS];
    int num_shadow_reg_configured;

    struct lock_class_key srng_key[HAL_SRNG_RING_ID_MAX];
};

struct hal_test_sw2hw{
    u32 word0;
    u32 word1;
    u32 word2;
}__packed;

// 根据params调整hal_srng中对应ring_num的ring的详细内容
int wireless_simu_hal_srng_setup(struct wireless_simu *priv, enum hal_ring_type type,
                                 int ring_num, int mac_id,
                                 struct hal_srng_params *params);

// 在驱动启动时初始化各个srng
int wireless_simu_hal_srng_init(struct wireless_simu *priv);

// 既然有初始化就应该有删除
void wireless_simu_hal_srng_deinit(struct wireless_simu *priv);

/*
 * hal srng test
 * 包含 创建srng、发送数据、删除srng*/ 
void wireless_simu_hal_srng_test(struct wireless_simu *priv);

#endif