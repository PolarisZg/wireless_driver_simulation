#include "wireless.h"

static const struct hal_srng_config hw_srng_config_template[] = {
	{
		/* HAL_TEST_SRNG  与 enum hal_ring_type 对应*/
		.start_ring_id = HAL_SRNG_RING_ID_TEST_SW2HW,
		.max_rings = 1,
		.entry_size = sizeof(struct hal_test_sw2hw) >> 2, // 右移 2 位代表以32bit为单位
		.lmac_ring = false,
		.ring_dir = HAL_SRNG_DIR_SRC,
		.max_size = HAL_TEST_SW2HW_SIZE,
	},
	{
		/* HAL_TEST_SRNG_DST */
		.start_ring_id = HAL_SRNG_RING_ID_TEST_DST,
		.max_rings = 1,
		.entry_size = sizeof(struct hal_test_dst) >> 2,
		.lmac_ring = false,
		.ring_dir = HAL_SRNG_DIR_SRC,
		.max_size = HAL_TEST_SW2HW_SIZE,
	},
	{
		/* HAL_TEST_SRNG_DST_STATUS 和上一个ring配套使用*/
		.start_ring_id = HAL_SRNG_RING_ID_TEST_DST_STATUS,
		.max_rings = 1,
		.entry_size = sizeof(struct hal_test_dst_status) >> 2,
		.lmac_ring = false,
		.ring_dir = HAL_SRNG_DIR_DST,
		.max_size = HAL_TEST_SW2HW_SIZE,
	},

	/* TODO: max_rings can populated by querying HW capabilities */
	// { /* REO_DST */
	// 	.start_ring_id = HAL_SRNG_RING_ID_REO2SW1,
	// 	.max_rings = 4,
	// 	.entry_size = sizeof(struct hal_reo_dest_ring) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_REO_REO2SW1_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* REO_EXCEPTION */
	// 	/* Designating REO2TCL ring as exception ring. This ring is
	// 	 * similar to other REO2SW rings though it is named as REO2TCL.
	// 	 * Any of theREO2SW rings can be used as exception ring.
	// 	 */
	// 	.start_ring_id = HAL_SRNG_RING_ID_REO2TCL,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_reo_dest_ring) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_REO_REO2TCL_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* REO_REINJECT */
	// 	.start_ring_id = HAL_SRNG_RING_ID_SW2REO,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_reo_entrance_ring) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_REO_SW2REO_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* REO_CMD */
	// 	.start_ring_id = HAL_SRNG_RING_ID_REO_CMD,
	// 	.max_rings = 1,
	// 	.entry_size = (sizeof(struct hal_tlv_hdr) +
	// 		sizeof(struct hal_reo_get_queue_stats)) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_REO_CMD_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* REO_STATUS */
	// 	.start_ring_id = HAL_SRNG_RING_ID_REO_STATUS,
	// 	.max_rings = 1,
	// 	.entry_size = (sizeof(struct hal_tlv_hdr) +
	// 		sizeof(struct hal_reo_get_queue_stats_status)) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_REO_STATUS_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* TCL_DATA */
	// 	.start_ring_id = HAL_SRNG_RING_ID_SW2TCL1,
	// 	.max_rings = 3,
	// 	.entry_size = (sizeof(struct hal_tlv_hdr) +
	// 		     sizeof(struct hal_tcl_data_cmd)) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_SW2TCL1_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* TCL_CMD */
	// 	.start_ring_id = HAL_SRNG_RING_ID_SW2TCL_CMD,
	// 	.max_rings = 1,
	// 	.entry_size = (sizeof(struct hal_tlv_hdr) +
	// 		     sizeof(struct hal_tcl_gse_cmd)) >> 2,
	// 	.lmac_ring =  false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_SW2TCL1_CMD_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* TCL_STATUS */
	// 	.start_ring_id = HAL_SRNG_RING_ID_TCL_STATUS,
	// 	.max_rings = 1,
	// 	.entry_size = (sizeof(struct hal_tlv_hdr) +
	// 		     sizeof(struct hal_tcl_status_ring)) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_TCL_STATUS_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* CE_SRC */
	// 	.start_ring_id = HAL_SRNG_RING_ID_CE0_SRC,
	// 	.max_rings = 12,
	// 	.entry_size = sizeof(struct hal_ce_srng_src_desc) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_CE_SRC_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* CE_DST */
	// 	.start_ring_id = HAL_SRNG_RING_ID_CE0_DST,
	// 	.max_rings = 12,
	// 	.entry_size = sizeof(struct hal_ce_srng_dest_desc) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_CE_DST_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* CE_DST_STATUS */
	// 	.start_ring_id = HAL_SRNG_RING_ID_CE0_DST_STATUS,
	// 	.max_rings = 12,
	// 	.entry_size = sizeof(struct hal_ce_srng_dst_status_desc) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_CE_DST_STATUS_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* WBM_IDLE_LINK */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WBM_IDLE_LINK,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_wbm_link_desc) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* SW2WBM_RELEASE */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WBM_SW_RELEASE,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_wbm_release_ring) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_SW2WBM_RELEASE_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* WBM2SW_RELEASE */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WBM2SW0_RELEASE,
	// 	.max_rings = 5,
	// 	.entry_size = sizeof(struct hal_wbm_release_ring) >> 2,
	// 	.lmac_ring = false,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_WBM2SW_RELEASE_RING_BASE_MSB_RING_SIZE,
	// },
	// { /* RXDMA_BUF */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WMAC1_SW2RXDMA0_BUF,
	// 	.max_rings = 2,
	// 	.entry_size = sizeof(struct hal_wbm_buffer_ring) >> 2,
	// 	.lmac_ring = true,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_RXDMA_RING_MAX_SIZE,
	// },
	// { /* RXDMA_DST */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WMAC1_RXDMA2SW0,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_reo_entrance_ring) >> 2,
	// 	.lmac_ring = true,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_RXDMA_RING_MAX_SIZE,
	// },
	// { /* RXDMA_MONITOR_BUF */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WMAC1_SW2RXDMA2_BUF,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_wbm_buffer_ring) >> 2,
	// 	.lmac_ring = true,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_RXDMA_RING_MAX_SIZE,
	// },
	// { /* RXDMA_MONITOR_STATUS */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WMAC1_SW2RXDMA1_STATBUF,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_wbm_buffer_ring) >> 2,
	// 	.lmac_ring = true,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_RXDMA_RING_MAX_SIZE,
	// },
	// { /* RXDMA_MONITOR_DST */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WMAC1_RXDMA2SW1,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_reo_entrance_ring) >> 2,
	// 	.lmac_ring = true,
	// 	.ring_dir = HAL_SRNG_DIR_DST,
	// 	.max_size = HAL_RXDMA_RING_MAX_SIZE,
	// },
	// { /* RXDMA_MONITOR_DESC */
	// 	.start_ring_id = HAL_SRNG_RING_ID_WMAC1_SW2RXDMA1_DESC,
	// 	.max_rings = 1,
	// 	.entry_size = sizeof(struct hal_wbm_buffer_ring) >> 2,
	// 	.lmac_ring = true,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_RXDMA_RING_MAX_SIZE,
	// },
	// { /* RXDMA DIR BUF */
	// 	.start_ring_id = HAL_SRNG_RING_ID_RXDMA_DIR_BUF,
	// 	.max_rings = 1,
	// 	.entry_size = 8 >> 2, /* TODO: Define the struct */
	// 	.lmac_ring = true,
	// 	.ring_dir = HAL_SRNG_DIR_SRC,
	// 	.max_size = HAL_RXDMA_RING_MAX_SIZE,
	// },
};

static int hal_srng_get_ring_id(struct wireless_simu_hal *hal, enum hal_ring_type type, int ring_num, int mac_id)
{
	struct hal_srng_config *srng_config = &hal->srng_config[type];
	int ring_id;

	if (ring_num >= srng_config->max_rings)
	{
		pr_err("%s : get_RING_ID %d type ivalid ring num %d \n", WIRELESS_SIMU_DEVICE_NAME, type, ring_num);
		return -EINVAL;
	}

	ring_id = srng_config->start_ring_id + ring_num;

	if (srng_config->lmac_ring)
		ring_id += mac_id * HAL_SRNG_RINGS_PER_LMAC;

	if (WARN_ON(ring_id >= HAL_SRNG_RING_ID_MAX))
		return -EINVAL;

	return ring_id;
}

/*
 * 将 src 类型的 srng 信息写入到 hw 寄存器;
 * 可以看到以 HAL_SRNG_REG_GRP_R0 序号为基址的mmio寄存器组功能为配置hw中的srng基础信息；
 * 以HAL_SRNG_REG_GRP_R2序号为基址的寄存器组功能为配置srng ring相关的信息，如hp tp
 */

static void hal_srng_src_hw_init(struct wireless_simu *priv, struct hal_srng *srng)
{
	pr_info("%s : %d ring hal srng src hw init start \n", WIRELESS_SIMU_DEVICE_NAME, srng->ring_id);
	struct wireless_simu_hal *hal = &priv->hal;
	u32 reg_base;
	u32 val;

	reg_base = srng->hwreg_base[HAL_SRNG_REG_GRP_R0];
	if (srng->flags & HAL_SRNG_FLAGS_MSI_INTR)
	{
		// 不支持msi中断，这里就不写了
	}

	wireless_hif_write32(priv, reg_base, srng->ring_base_paddr); // 具体的寄存器地址, 参考hal_srng_create_config中的描述

	val = FIELD_PREP(HAL_TCL1_RING_BASE_MSB_RING_BASE_ADDR_MSB, ((u64)srng->ring_base_paddr >> HAL_ADDR_MSB_REG_SHIFT)) |
		  FIELD_PREP(HAL_TCL1_RING_BASE_MSB_RING_SIZE, (srng->entry_size * srng->num_entries));
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(1), val);

	val = FIELD_PREP(HAL_REO1_RING_ID_ENTRY_SIZE, srng->entry_size);
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(2), val);

	if (srng->ring_id == HAL_SRNG_RING_ID_WBM_IDLE_LINK)
	{
		// 我不明白为什么他重写了一遍上面的那些32bit写
		wireless_hif_write32(priv, reg_base, (u32)srng->ring_base_paddr); // 具体的寄存器地址, 参考hal_srng_create_config中的描述

		val = FIELD_PREP(HAL_TCL1_RING_BASE_MSB_RING_BASE_ADDR_MSB, ((u64)srng->ring_base_paddr >> HAL_ADDR_MSB_REG_SHIFT)) |
			  FIELD_PREP(HAL_TCL1_RING_BASE_MSB_RING_SIZE, (srng->entry_size * srng->num_entries));
		wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(1), val);

		val = FIELD_PREP(HAL_REO1_RING_ID_ENTRY_SIZE, srng->entry_size);
		wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(2), val);
	}

	// interrupt set up
	val = FIELD_PREP(HAL_TCL1_RING_CONSR_INT_SETUP_IX0_INTR_TMR_THOLD, srng->intr_timer_thres_us) |
		  FIELD_PREP(HAL_TCL1_RING_CONSR_INT_SETUP_IX0_BATCH_COUNTER_THOLD, (srng->intr_batch_cntr_thres_entries * srng->entry_size));
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(3), val);

	val = 0;
	if (srng->flags & HAL_SRNG_FLAGS_LOW_THRESH_INTR_EN)
	{
		val = FIELD_PREP(HAL_TCL1_RING_CONSR_INT_SETUP_IX1_LOW_THOLD, srng->u.src_ring.low_threshold);
	}
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(4), val);

	if (srng->ring_id != HAL_SRNG_RING_ID_WBM_IDLE_LINK)
	{
		u64 tp_addr = hal->rdp.paddr +
					  ((unsigned long)srng->u.src_ring.tp_addr -
					   (unsigned long)hal->rdp.vaddr); // 参考wireless_simu_hal_srng_setup中对src tp_addr的设计，实际这个差就是ring_id

		wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(5), tp_addr & HAL_ADDR_LSB_REG_MASK);
		wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(6), tp_addr >> HAL_ADDR_MSB_REG_SHIFT);
	}

	// init src ring tail pointer and head pointer,对于src ring的hw来说，hp应该是无效的
	reg_base = srng->hwreg_base[HAL_SRNG_REG_GRP_R2];
	wireless_hif_write32(priv, reg_base, 0);
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(1), 0);
	*srng->u.src_ring.tp_addr = 0;

	reg_base = srng->hwreg_base[HAL_SRNG_REG_GRP_R0];
	val = 0;

	// 大端cpu
	if (srng->flags & HAL_SRNG_FLAGS_DATA_TLV_SWAP)
		val |= HAL_TCL1_RING_MISC_DATA_TLV_SWAP;
	if (srng->flags & HAL_SRNG_FLAGS_RING_PTR_SWAP)
		val |= HAL_TCL1_RING_MISC_HOST_FW_SWAP;
	if (srng->flags & HAL_SRNG_FLAGS_MSI_SWAP)
		val |= HAL_TCL1_RING_MISC_MSI_SWAP;

	/* Loop count is not used for SRC rings */
	val |= HAL_TCL1_RING_MISC_MSI_LOOPCNT_DISABLE;

	val |= HAL_TCL1_RING_MISC_SRNG_ENABLE;
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(7), val);
}

static void hal_srng_dst_hw_init(struct wireless_simu *priv, struct hal_srng *srng)
{
	// pr_err("%s : no dst ring \n", WIRELESS_SIMU_DEVICE_NAME);
	pr_info("%s : %d ring srng dst hw init start \n", WIRELESS_SIMU_DEVICE_NAME, srng->ring_id);
	struct wireless_simu_hal *hal = &priv->hal;
	u32 val;
	u64 hp_addr;
	u32 reg_base;

	reg_base = srng->hwreg_base[HAL_SRNG_REG_GRP_R0];

	// msi 中断配置

	// 也就是说无论src还是dst，本质上都是在mem中建立的数据结构，并不会大量占用device的内存
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(0), srng->ring_base_paddr);

	val = FIELD_PREP(HAL_REO1_RING_BASE_MSB_RING_BASE_ADDR_MSB, ((u64)srng->ring_base_paddr >> HAL_ADDR_MSB_REG_SHIFT)) |
		  FIELD_PREP(HAL_REO1_RING_BASE_MSB_RING_SIZE, (srng->entry_size * srng->num_entries));
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(1), val);

	// 相对于src ring 多了一个ring id 但我并不懂为什么多这个
	val = FIELD_PREP(HAL_REO1_RING_ID_RING_ID, srng->ring_id) |
		  FIELD_PREP(HAL_REO1_RING_ID_ENTRY_SIZE, srng->entry_size);
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(2), val);

	val = FIELD_PREP(HAL_REO1_RING_PRDR_INT_SETUP_INTR_TMR_THOLD, (srng->intr_timer_thres_us >> 3)) |
		  FIELD_PREP(HAL_REO1_RING_PRDR_INT_SETUP_BATCH_COUNTER_THOLD, (srng->intr_batch_cntr_thres_entries * srng->entry_size));
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(3), val);

	// hp addr 由device进行更新，需要将paddr传下去
	hp_addr = hal->rdp.paddr +
			  ((unsigned long)srng->u.dst_ring.hp_addr -
			   (unsigned long)hal->rdp.vaddr);
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(5), hp_addr & HAL_ADDR_LSB_REG_MASK);
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(6), hp_addr >> HAL_ADDR_MSB_REG_SHIFT);

	// 初始化底部的hp
	reg_base = srng->hwreg_base[HAL_SRNG_REG_GRP_R2];
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(0), 0);
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(1), 0);
	*srng->u.dst_ring.hp_addr = 0;

	// 写flag
	reg_base = srng->hwreg_base[HAL_SRNG_REG_GRP_R0];
	val = 0;
	if (srng->flags & HAL_SRNG_FLAGS_DATA_TLV_SWAP)
		val |= HAL_REO1_RING_MISC_DATA_TLV_SWAP;
	if (srng->flags & HAL_SRNG_FLAGS_RING_PTR_SWAP)
		val |= HAL_REO1_RING_MISC_HOST_FW_SWAP;
	if (srng->flags & HAL_SRNG_FLAGS_MSI_SWAP)
		val |= HAL_REO1_RING_MISC_MSI_SWAP;
	val |= HAL_REO1_RING_MISC_SRNG_ENABLE;
	wireless_hif_write32(priv, reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(7), val);
}

static void hal_srng_hw_init(struct wireless_simu *priv, struct hal_srng *srng)
{
	if (srng->ring_dir == HAL_SRNG_DIR_SRC)
		hal_srng_src_hw_init(priv, srng);
	else
		hal_srng_dst_hw_init(priv, srng);
}

int wireless_simu_hal_srng_setup(struct wireless_simu *priv, enum hal_ring_type type,
								 int ring_num, int mac_id,
								 struct hal_srng_params *params)
{
	int ring_id;
	u32 lmac_idx;
	u32 reg_base;
	struct hal_srng *srng;
	struct wireless_simu_hal *hal = &priv->hal;
	struct hal_srng_config *srng_config = &hal->srng_config[type];

	// 获取ring_id
	ring_id = hal_srng_get_ring_id(hal, type, ring_num, mac_id);
	if (ring_id < 0)
		return ring_id;

	srng = &hal->srng_list[ring_id];

	// 写ring内容
	srng->ring_id = ring_id;
	srng->ring_dir = srng_config->ring_dir;
	srng->ring_base_paddr = params->ring_base_paddr;
	srng->ring_base_vaddr = params->ring_base_vaddr;
	srng->entry_size = srng_config->entry_size;
	srng->num_entries = params->num_entries;
	srng->ring_size = srng->entry_size * srng->num_entries;
	srng->intr_batch_cntr_thres_entries = params->intr_batch_cntr_thres_entries;
	srng->intr_timer_thres_us = params->intr_timer_thres_us;
	srng->flags = params->flags;
	srng->msi_addr = params->msi_addr;
	srng->msi_data = params->msi_data;
	srng->initialized = 1;
	spin_lock_init(&srng->lock);
	lockdep_set_class(&srng->lock, hal->srng_key + ring_id);

	// srng->hwreg_base 的初始化，该处和硬件设计强相关，需特别注意寄存器的地址和功能的对应
	/*
	 * |-------- 16bit --------|---- 8 bit ----|- 1 bit -|--- 5 bit ---|- 2 bit -|
	 * |          0001         |    ring_id    |  grp    |    reg      | 4 char  |*/
	srng->hwreg_base[HAL_SRNG_REG_GRP_R0] = (HAL_SRNG_REG_BASE | (ring_id << 8) | (HAL_SRNG_REG_GRP_R0 << 7));
	srng->hwreg_base[HAL_SRNG_REG_GRP_R2] = (HAL_SRNG_REG_BASE | (ring_id << 8) | (HAL_SRNG_REG_GRP_R2 << 7));

	// for (int i = 0; i < HAL_SRNG_NUM_REG_GRP; i++)
	// {
	// 	srng->hwreg_base[i] = srng_config->reg_start[i] + (ring_num * srng_config->reg_size[i]);
	// }

	reg_base = srng->hwreg_base[HAL_SRNG_REG_GRP_R2];

	// 此处 << 2 是因为entrysize以 32 bit为单位
	memset(srng->ring_base_vaddr, 0, (srng->entry_size * srng->num_entries) << 2);

	if (IS_ENABLED(CONFIG_CPU_BIG_ENDIAN))
	{
		srng->flags |= HAL_SRNG_FLAGS_MSI_SWAP | HAL_SRNG_FLAGS_DATA_TLV_SWAP |
					   HAL_SRNG_FLAGS_RING_PTR_SWAP;
	}

	if (srng->ring_dir == HAL_SRNG_DIR_SRC)
	{
		srng->u.src_ring.hp = 0;
		srng->u.src_ring.cached_tp = 0;
		srng->u.src_ring.reap_hp = srng->ring_size - srng->entry_size;
		srng->u.src_ring.tp_addr = (void *)(hal->rdp.vaddr + ring_id);
		srng->u.src_ring.low_threshold = params->low_threshold * srng->entry_size;
		if (srng_config->lmac_ring)
		{
			lmac_idx = ring_id - HAL_SRNG_RING_ID_LMAC1_ID_START;
			srng->u.src_ring.hp_addr = (void *)(hal->wrp.vaddr + lmac_idx);
			srng->flags |= HAL_SRNG_FLAGS_LMAC_RING; // 也就是说有lmac ring的在向ring添加时不再需要去写寄存器
		}
		else
		{
			// 此处出现了support shadow reg 问题，但我并不知道该问题应该怎么处理，因此暂时修改为将hp直接指向对应的mmio寄存器
			srng->u.src_ring.hp_addr = (u32 *)((unsigned long)priv->mmio_addr + reg_base);
			pr_info("%s : srng setup %d type %d ring num hp_addr %lx \n", WIRELESS_SIMU_DEVICE_NAME,
					type, ring_id, (unsigned long)srng->u.src_ring.hp_addr - (unsigned long)priv->mmio_addr);
		}
	}
	else
	{
		// HAL_SRNG_DIR_DST 这个方向应该是HW上传数据的方向
		srng->u.dst_ring.loop_cnt = 1;
		srng->u.dst_ring.tp = 0;
		srng->u.dst_ring.cached_hp = 0;
		srng->u.dst_ring.hp_addr = (void *)(hal->rdp.vaddr + ring_id);
		if (srng_config->lmac_ring)
		{
			/*
			 * 在dst方向上，如果支持lmac ring，那么tail pointer会被hw直接修改共享内存区域
			 */
			lmac_idx = ring_id - HAL_SRNG_RING_ID_LMAC1_ID_START;
			srng->u.dst_ring.tp_addr = (void *)(hal->wrp.vaddr + lmac_idx);
			srng->flags |= HAL_SRNG_FLAGS_LMAC_RING;
		}
		else
		{
			// srng->u.dst_ring.tp_addr = (u32 *)((unsigned long)priv->mmio_addr + reg_base + HAL_SRNG_REG_R_GROUP_OFFSET(1));
			/* 上一行使用了一个offset来区分dst和src方向，但实际上并不需要这样的区分，device中能够根据ring_id判断某个ring是dst还是src*/
			srng->u.dst_ring.tp_addr = (u32 *)((unsigned long)priv->mmio_addr + reg_base);
			pr_info("%s : srng setup %d type %d ring num tp_addr %lx \n", WIRELESS_SIMU_DEVICE_NAME,
					type, ring_id, (unsigned long)srng->u.dst_ring.tp_addr - (unsigned long)priv->mmio_addr);
		}
	}

	// 返回ring_id表示设置完对应的ring，上层子模块保存该ring_id等使用ring时传入
	if (srng_config->lmac_ring)
		return ring_id;

	hal_srng_hw_init(priv, srng);

	// if(type == HAL_CE_DST)

	return ring_id;
}

static int hal_srng_create_config(struct wireless_simu_hal *hal)
{
	// struct hal_srng_config *s;
	hal->srng_config = kmemdup(hw_srng_config_template, sizeof(hw_srng_config_template), GFP_KERNEL);

	if (!hal->srng_config)
	{
		return -ENOMEM;
	}

	// 根据我的对寄存器的设定，不需要对不同类型的srng的基地址进行初始化配置，只需要在wireless_simu_hal_srng_setup中得到ring_id后进行配置即可
	// 当ring数量超过 256 时再来讨论优化的问题
	// s = &hal->srng_config[HAL_TEST_SRNG];
	// s->reg_start[HAL_SRNG_REG_GRP_R0] = HAL_TEST_SRNG_REG_GRP;		  // srng配置信息 【寄存器组】 的基地址, 这里面存放的是相对于mmio的偏移地址
	// s->reg_size[HAL_SRNG_REG_GRP_R0] = HAL_TEST_SRNG_REG_GRP_R0_SIZE; // srng配置信息寄存器组中寄存器的数量
	// s->reg_start[HAL_SRNG_REG_GRP_R2] = HAL_TEST_SRNG_REG_GRP_R2;
	// s->reg_size[HAL_SRNG_REG_GRP_R2] = HAL_TEST_SRNG_REG_GRP_R2_SIZE;

	// 后续就是对不同type的srng载入不同的寄存器地址

	return 0;
}

static int hal_alloc_cont_rdp(struct wireless_simu_hal *hal)
{
	size_t size;
	size = sizeof(u32) * HAL_SRNG_RING_ID_MAX;
	hal->rdp.vaddr = dma_alloc_coherent(hal->dev, size, &hal->rdp.paddr, GFP_KERNEL);
	if (!hal->rdp.vaddr)
		return -ENOMEM;
	return 0;
}

static int hal_alloc_cont_wdp(struct wireless_simu_hal *hal)
{
	size_t size;
	size = sizeof(u32) * HAL_SRNG_NUM_LMAC_RINGS;
	hal->wrp.vaddr = dma_alloc_coherent(hal->dev, size, &hal->wrp.paddr, GFP_KERNEL);
	if (!hal->wrp.vaddr)
		return -ENOMEM;
	return 0;
}

static void hal_register_srng_key(struct wireless_simu_hal *hal)
{
	u32 ring_id;

	for (ring_id = 0; ring_id < HAL_SRNG_RING_ID_MAX; ring_id++)
		lockdep_unregister_key(hal->srng_key + ring_id);
}

static void hal_free_cont_rdp(struct wireless_simu_hal *hal)
{
	if (!hal->rdp.vaddr)
		return;
	size_t size = sizeof(u32) * HAL_SRNG_RING_ID_MAX;
	dma_free_coherent(hal->dev, size, hal->rdp.vaddr, hal->rdp.paddr);
	hal->rdp.vaddr = NULL;
}

int wireless_simu_hal_srng_init(struct wireless_simu *priv)
{
	struct wireless_simu_hal *hal = &priv->hal;
	int ret = 0;

	memset(hal, 0, sizeof(*hal));
	hal->dev = &priv->pci_dev->dev;

	// 配置hal中config属性，该属性定义了不同类型的srng的entry size大小、总大小及同一类型的ring数量
	ret = hal_srng_create_config(hal);
	if (ret)
		goto err_hal;

	ret = hal_alloc_cont_rdp(hal);
	if (ret)
		goto err_hal;
	pr_info("%s : srng hal rdp vaddr %p paddr %llx \n", WIRELESS_SIMU_DEVICE_NAME, hal->rdp.vaddr, hal->rdp.paddr);

	ret = hal_alloc_cont_wdp(hal);
	if (ret)
		goto err_free_cont_rdp;
	pr_info("%s : srng hal rdp vaddr %p paddr %llx \n", WIRELESS_SIMU_DEVICE_NAME, hal->wrp.vaddr, hal->wrp.paddr);

	hal_register_srng_key(hal);

	return 0;

err_free_cont_rdp:
	hal_free_cont_rdp(hal);
err_hal:
	return ret;
}

static void hal_unregister_srng_key(struct wireless_simu_hal *hal)
{
	u32 ring_id;

	for (ring_id = 0; ring_id < HAL_SRNG_RING_ID_MAX; ring_id++)
		lockdep_unregister_key(hal->srng_key + ring_id);
}

static void hal_free_cont_wrp(struct wireless_simu_hal *hal)
{
	size_t size;

	if (!hal->wrp.vaddr)
		return;

	size = sizeof(u32) * HAL_SRNG_NUM_LMAC_RINGS;
	dma_free_coherent(hal->dev, size,
					  hal->wrp.vaddr, hal->wrp.paddr);
	hal->wrp.vaddr = NULL;
}

void wireless_simu_hal_srng_deinit(struct wireless_simu *priv)
{
	struct wireless_simu_hal *hal = &priv->hal;
	hal_unregister_srng_key(hal);
	hal_free_cont_rdp(hal);
	hal_free_cont_wrp(hal);
	kfree(hal->srng_config);
}

void wireless_simu_hal_srng_access_begin(struct wireless_simu *priv, struct hal_srng *srng)
{
	lockdep_assert_held(&srng->lock);

	if (srng->ring_dir == HAL_SRNG_DIR_SRC)
	{
		srng->u.src_ring.cached_tp = *(volatile u32 *)srng->u.src_ring.tp_addr;
	}
	else
	{
		srng->u.dst_ring.cached_hp = *srng->u.dst_ring.hp_addr;

		if (srng->flags & HAL_SRNG_FLAGS_CACHED) // 缓存优化，看不懂所以就不进行测试和设置
			pr_info("%s : hal srng access begin 'hai mei zuo' \n", WIRELESS_SIMU_DEVICE_NAME);
	}
}

/* 返回 src ring 中剩余 entry 的个数，
 * sync_hw_ptr 参数为 true 时返回即时的数目,
 * 一般在使用access begin 后设定为 false
 *
 * 个数的计算：
 * 1. 当 tp 在 hp前面时，计算二者间的距离，此时hp指向空闲的区域，hp指向之后hw需要读取的区域
 * 2. 当 tp 在 hp 后面时，计算 hp至尾部 再从头部开始计算一直到tp之间的距离
 * 3. 二者重合时，代表此时ring中没有数据
 * */
int wireless_simu_hal_srng_src_num_free(struct wireless_simu *priv, struct hal_srng *srng, bool sync_hw_ptr)
{
	u32 tp, hp;

	lockdep_assert_held(&srng->lock);

	hp = srng->u.src_ring.hp; // src hp 由host端修改

	if (sync_hw_ptr)
	{
		tp = *srng->u.src_ring.tp_addr;
		srng->u.src_ring.cached_tp = tp;
	}
	else
	{
		tp = srng->u.src_ring.cached_tp;
	}

	if (tp > hp)
		return ((tp - hp) / srng->entry_size) - 1;
	else
		return ((srng->ring_size - hp + tp) / srng->entry_size) - 1;
}

/* update cached ring head/tail pointer to hw. 必须在之前调用过access_begin */
void wireless_simu_hal_srng_access_end(struct wireless_simu *priv, struct hal_srng *srng)
{
	lockdep_assert_held(&srng->lock);

	if (srng->flags & HAL_SRNG_FLAGS_LMAC_RING)
	{
		/* 具有 lmac ring 的 host 端 ring 只需要更新 dma malloc 的区域中对应的区域即可 */
		if (srng->ring_dir == HAL_SRNG_DIR_SRC)
		{
			srng->u.src_ring.last_tp = *(volatile u32 *)srng->u.src_ring.tp_addr;
			*srng->u.src_ring.hp_addr = srng->u.src_ring.hp;
		}
		else
		{
			srng->u.dst_ring.last_hp = *srng->u.dst_ring.hp_addr;
			*srng->u.dst_ring.tp_addr = srng->u.dst_ring.tp;
		}
	}
	else
	{
		if (srng->ring_dir == HAL_SRNG_DIR_SRC)
		{
			srng->u.src_ring.last_tp = *(volatile u32 *)srng->u.src_ring.tp_addr;
			wireless_hif_write32(priv, (unsigned long)srng->u.src_ring.hp_addr - (unsigned long)priv->mmio_addr, srng->u.src_ring.hp);
		}
		else
		{
			srng->u.dst_ring.last_hp = *srng->u.dst_ring.hp_addr;
			wireless_hif_write32(priv, (unsigned long)srng->u.dst_ring.tp_addr - (unsigned long)priv->mmio_addr, srng->u.dst_ring.tp);
		}
	}

	srng->timestamp = jiffies;
}

u32 *wireless_simu_hal_srng_src_get_next_reaped(struct wireless_simu *priv, struct hal_srng *srng)
{
	u32 *desc;

	lockdep_assert_held(&srng->lock);

	if (srng->u.src_ring.hp == srng->u.src_ring.reap_hp)
	{
		return NULL;
	}

	desc = srng->ring_base_vaddr + srng->u.src_ring.hp;
	srng->u.src_ring.hp = (srng->u.src_ring.hp + srng->entry_size) %
						  srng->ring_size;

	return desc;
}

u32 *wireless_simu_hal_srng_src_get_next_entry(struct wireless_simu *priv, struct hal_srng *srng)
{
	u32 *desc;
	u32 next_hp;

	lockdep_assert_held(&srng->lock);

	next_hp = (srng->u.src_ring.hp + srng->entry_size) % srng->ring_size;

	if (next_hp == srng->u.src_ring.cached_tp)
		return NULL;

	desc = srng->ring_base_vaddr + srng->u.src_ring.hp;
	srng->u.src_ring.hp = next_hp;

	srng->u.src_ring.reap_hp = next_hp;

	return desc;
}

u32 *wireless_simu_hal_srng_dst_get_next_entry(struct wireless_simu *priv, struct hal_srng *srng)
{
	u32 *desc;

	lockdep_assert_held(&srng->lock);

	if (srng->u.dst_ring.tp == srng->u.dst_ring.cached_hp)
	{
		return NULL;
	}

	desc = srng->ring_base_vaddr + srng->u.dst_ring.tp;

	srng->u.dst_ring.tp += srng->entry_size;

	/* 这是两个人写的吗，上面就是取模，下面是条件判断 */
	if (srng->u.dst_ring.tp == srng->ring_size)
		srng->u.dst_ring.tp = 0;

	if (srng->flags & HAL_SRNG_FLAGS_CACHED)
		pr_info("%s : dst get next entry err flag \n", WIRELESS_SIMU_DEVICE_NAME);

	return desc;
}

/******* srng test ********/

static struct srng_test_ring *hal_srng_test_alloc_ring(struct wireless_simu *priv, int nentries, int desc_sz)
{
	struct srng_test_ring *ring;
	dma_addr_t base_addr;

	ring = kzalloc(struct_size(ring, skb, nentries), GFP_KERNEL);
	if (ring == NULL)
		return ERR_PTR(-ENOMEM);

	ring->nentries = nentries;
	ring->nentries_mask = nentries - 1;

	ring->base_addr_owner_space_unaligned = dma_alloc_coherent(&priv->pci_dev->dev, nentries * desc_sz + SRNG_TEST_DESC_RING_ALIGN, &base_addr, GFP_KERNEL);
	if (!ring->base_addr_owner_space_unaligned)
	{
		kfree(ring);
		return ERR_PTR(-ENOMEM);
	}
	memset(ring->base_addr_owner_space_unaligned, 0, nentries * desc_sz + SRNG_TEST_DESC_RING_ALIGN);
	ring->dma_size = nentries * desc_sz + SRNG_TEST_DESC_RING_ALIGN;

	ring->base_addr_test_space_unaligned = base_addr;

	ring->base_addr_owner_space = PTR_ALIGN(ring->base_addr_owner_space_unaligned, SRNG_TEST_DESC_RING_ALIGN);
	ring->base_addr_test_space = ALIGN(ring->base_addr_test_space_unaligned, SRNG_TEST_DESC_RING_ALIGN);
	pr_info("%s : srng test ring %p vaddr %llx paddr \n", WIRELESS_SIMU_DEVICE_NAME, ring->base_addr_owner_space, ring->base_addr_test_space);

	return ring;
}

// 删除所有st中的pipe，取消dma地址空间
static void hal_srng_test_free_pipes(struct srng_test *st)
{
	struct srng_test_pipe *pipe;
	struct srng_test_ring *ring;
	int desc_sz;

	for (int i = 0; i < SRNG_TEST_PIPE_COUNT_MAX; i++)
	{
		pipe = &st->pipes[i];

		if (pipe->src_ring)
		{
			desc_sz = sizeof(struct hal_test_sw2hw); // 此处需要和hal_srng_config hw_srng_config_template中设定的entrysize相符，并非相同，因为后一个以32bit为单位
			ring = pipe->src_ring;
			dma_free_coherent(&st->priv->pci_dev->dev,
							  ring->nentries * desc_sz + SRNG_TEST_DESC_RING_ALIGN,
							  ring->base_addr_owner_space_unaligned,
							  ring->base_addr_test_space_unaligned);
			kfree(ring);
			pipe->src_ring = NULL;
		}

		if (pipe->dst_ring)
		{
			ring = pipe->dst_ring;
			dma_free_coherent(&st->priv->pci_dev->dev,
							  ring->dma_size,
							  ring->base_addr_owner_space_unaligned,
							  ring->base_addr_test_space_unaligned);
			kfree(ring);
			pipe->dst_ring = NULL;
		}

		if (pipe->status_ring)
		{
			ring = pipe->status_ring;
			dma_free_coherent(&st->priv->pci_dev->dev,
							  ring->dma_size,
							  ring->base_addr_owner_space_unaligned,
							  ring->base_addr_test_space_unaligned);
			kfree(ring);
			pipe->status_ring = NULL;
		}
	}
}

static int hal_srng_test_alloc_pipe(struct srng_test *st, int id)
{
	struct srng_test_pipe *pipe = &st->pipes[id];
	struct srng_test_attr *attr = &st->host_config[id];
	struct srng_test_ring *ring;
	int nentries;
	int desc_sz;

	pipe->attr_flags = attr->flags;
	pipe->priv = st->priv;
	pipe->pipe_num = id;
	pipe->buf_sz = attr->src_sz_max;

	if (attr->src_nentries)
	{
		pipe->send_cb = attr->send_cb;
		nentries = roundup_pow_of_two(attr->src_nentries);
		desc_sz = sizeof(struct hal_test_sw2hw);
		ring = hal_srng_test_alloc_ring(st->priv, nentries, desc_sz);
		if (IS_ERR(ring))
			return PTR_ERR(ring);
		pipe->src_ring = ring;
	}

	if (attr->dest_nentries)
	{
		pipe->send_cb = attr->send_cb;
		pipe->recv_cb = attr->recv_cb;

		nentries = roundup_pow_of_two(attr->dest_nentries);
		desc_sz = sizeof(struct hal_test_dst);
		ring = hal_srng_test_alloc_ring(st->priv, nentries, desc_sz);
		if (IS_ERR(ring))
			return PTR_ERR(ring);
		pipe->dst_ring = ring;

		nentries = roundup_pow_of_two(attr->dest_nentries);
		desc_sz = sizeof(struct hal_test_dst_status);
		ring = hal_srng_test_alloc_ring(st->priv, nentries, desc_sz);
		if (IS_ERR(ring))
			return PTR_ERR(ring);
		pipe->status_ring = ring;
	}

	return 0;
}

// 注册test模块到hal_srng模块，将注册的hal_srng_ring id 存放在传入的ring当中，相当于将上级模块的ring与hal_srng中的ring建立关系
static int hal_srng_test_init_ring(struct wireless_simu *priv, struct srng_test_ring *ring, int id, enum hal_ring_type type)
{
	// 因为只有一种测试用例，所以在前面追加检查
	// if (type != HAL_TEST_SRNG)
	// {
	// 	pr_err("%s : test hal srng init inval type \n", WIRELESS_SIMU_DEVICE_NAME);
	// 	return -EINVAL;
	// }

	struct hal_srng_params params = {0};
	int ret;

	// 填充params
	params.ring_base_paddr = ring->base_addr_test_space;
	params.ring_base_vaddr = ring->base_addr_owner_space;
	params.num_entries = ring->nentries;

	switch (type)
	{
	case HAL_TEST_SRNG:
		break;
	case HAL_TEST_SRNG_DST:
		break;
	case HAL_TEST_SRNG_DST_STATUS:
		break;
	default:
		pr_err("%s : test hal srng init inval type \n", WIRELESS_SIMU_DEVICE_NAME);
		return -EINVAL;
	}

	/* 在此处完成其他 params 的初始化需求 */

	ret = wireless_simu_hal_srng_setup(priv, type, id, 0, &params);

	if (ret < 0)
	{
		pr_err("%s : hal srng setup %d \n", WIRELESS_SIMU_DEVICE_NAME, ret);
		return ret;
	}

	ring->hal_ring_id = ret;

	return 0;
}

static void wireless_simu_hal_srng_test_src_set_desc(void *buf, struct sk_buff *skb, u32 id, u8 byte_swap_data, unsigned int write_index)
{
	struct hal_test_sw2hw *desc = buf;
	dma_addr_t paddr = WIRELESS_SIMU_SKB_CB(skb)->paddr;

	desc->buffer_addr_low = paddr & HAL_ADDR_LSB_REG_MASK;
	desc->buffer_addr_info = FIELD_PREP(GENMASK(7, 0), ((u64)paddr >> HAL_ADDR_MSB_REG_SHIFT)) |
							 FIELD_PREP(BIT(9), byte_swap_data) |
							 FIELD_PREP(BIT(11), 0) |
							 FIELD_PREP(GENMASK(31, 16), skb->len);
	desc->meta_info = FIELD_PREP(GENMASK(15, 0), id);
	desc->write_index = write_index;
	desc->flags = 0x114514ff;
}

static void wireless_simu_test_dst_set_desc(void *buf, dma_addr_t paddr)
{
	struct hal_test_dst *desc = buf;

	desc->buffer_addr_low = paddr & HAL_ADDR_LSB_REG_MASK;
	desc->buffer_addr_info = FIELD_PREP(GENMASK(7, 0), ((u64)paddr >> HAL_ADDR_MSB_REG_SHIFT));

	desc->flag = 0x114514aa;
}

// 专用于srng_test的发送函数，对于其他发送函数需要将模块整合进priv中，对于st来说就直接传过来了
static int wireless_simu_hal_srng_test_send(struct srng_test *st, struct sk_buff *skb, u8 pipe_id, u16 transfer_id)
{
	struct srng_test_pipe *pipe = &st->pipes[pipe_id];
	struct hal_srng *srng;
	u32 *desc;
	unsigned int write_index, sw_index;
	unsigned int nentries_mask;
	int ret = 0;
	u8 byte_swap_data = 0; // 大端置 1
	int num_used;

	/* 对于禁用中断的情景，需要对ring进行遍历来处理已发送完成的entry */
	if (pipe->attr_flags & CE_ATTR_DIS_INTR)
	{
		spin_lock_bh(&st->srng_test_lock);

		write_index = pipe->src_ring->write_index;
		sw_index = pipe->src_ring->sw_index;

		if (write_index >= sw_index)
			num_used = write_index - sw_index;
		else
			num_used = pipe->src_ring->nentries - sw_index + write_index;

		spin_unlock_bh(&st->srng_test_lock);
	}

	spin_lock_bh(&st->srng_test_lock);

	write_index = pipe->src_ring->write_index;
	nentries_mask = pipe->src_ring->nentries_mask;

	srng = &st->priv->hal.srng_list[pipe->src_ring->hal_ring_id];

	spin_lock_bh(&srng->lock);

	wireless_simu_hal_srng_access_begin(st->priv, srng);

	if (unlikely(wireless_simu_hal_srng_src_num_free(st->priv, srng, false) < 1))
	{
		ret = -ENOBUFS;
		pr_err("%s : err srng %08x no free entry \n", WIRELESS_SIMU_DEVICE_NAME, srng->ring_id);
		goto exit;
	}

	desc = wireless_simu_hal_srng_src_get_next_entry(st->priv, srng);
	if (!desc)
	{
		ret = -ENOBUFS;
		pr_err("%s : err srng %08x no next entry \n", WIRELESS_SIMU_DEVICE_NAME, srng->ring_id);
		goto exit;
	}

	wireless_simu_hal_srng_test_src_set_desc(desc, skb, transfer_id, byte_swap_data, write_index);

	pipe->src_ring->skb[write_index] = skb;

	/* 本质上这是一个取模运算，
	 * nentries_mask 为 nentries 的总数量 - 1 
	 * nentries 为 2 的 指数倍(pow of 2)
	*/
	pipe->src_ring->write_index = (((write_index) + 1) & (nentries_mask)); 

exit:
	wireless_simu_hal_srng_access_end(st->priv, srng);

	spin_unlock_bh(&srng->lock);

	spin_unlock_bh(&st->srng_test_lock);
	
	return ret;
}

void wireless_simu_hal_srng_test(struct wireless_simu *priv)
{
	pr_info("%s : srng test start \n", WIRELESS_SIMU_DEVICE_NAME);

	struct srng_test st;
	memset(&st, 0, sizeof(st));
	struct srng_test_pipe *pipe;
	int ret;

	// 创建test模块的ring空间

	st.priv = priv;
	st.host_config = kmemdup(srng_test_configs, sizeof(srng_test_configs), GFP_KERNEL);
	if (!st.host_config)
	{
		pr_err("%s : srng config err \n", WIRELESS_SIMU_DEVICE_NAME);
		return;
	}

	spin_lock_init(&st.srng_test_lock);

	for (int i = 0; i < SRNG_TEST_PIPE_COUNT_MAX; i++)
	{
		ret = hal_srng_test_alloc_pipe(&st, i);
		if (ret)
		{
			hal_srng_test_free_pipes(&st);
			goto err_clear_config;
		}
	}

	// 将test模块ring空间注册至hal_srng模块
	for (int i = 0; i < SRNG_TEST_PIPE_COUNT_MAX; i++)
	{
		pipe = &st.pipes[i];
		if (pipe->src_ring)
		{
			ret = hal_srng_test_init_ring(priv, pipe->src_ring, i, HAL_TEST_SRNG);
			if (ret)
			{
				pr_err("%s : test hal srng setup %d ring \n", WIRELESS_SIMU_DEVICE_NAME, i);

				/* 似乎错误 setup hal srng 之后并没有惩罚 */

				goto err_clear_pipes;
			}

			pipe->src_ring->sw_index = 0;
			pipe->src_ring->write_index = 0;
		}

		if (pipe->dst_ring)
		{
			pr_err("%s : test hal srng no dst ring \n", WIRELESS_SIMU_DEVICE_NAME);
		}

		if (pipe->status_ring)
		{
			pr_err("%s : test hal srng no status ring \n", WIRELESS_SIMU_DEVICE_NAME);
		}
	}

	/* 读写操作 */

	/* 准备数据 */
	int data_size = round_up(50, 4);
	void *data = kzalloc(data_size, GFP_KERNEL);
	if (!data)
	{
		pr_err("%s : srng test data isnull \n", WIRELESS_SIMU_DEVICE_NAME);
		goto err_clear_pipes;
	}
	memset(data, 6, 4);
	memset((void *)data + data_size - 4, 7, 4);

	/* 转移数据至skb */
	struct sk_buff *skb;
	int round_len = round_up(data_size, 4);
	skb = dev_alloc_skb(round_len);
	if (!skb)
	{
		pr_err("%s : srng test skb alloc err \n", WIRELESS_SIMU_DEVICE_NAME);
		goto err_free_data;
	}
	memcpy(skb->data, data, data_size);
	skb_put(skb, round_len);

	/* dma 转换 */
	dma_addr_t skb_paddr; // 在ath11k中paddr要存放在skb_cb之中
	skb_paddr = dma_map_single(&priv->pci_dev->dev, skb->data, skb->len, DMA_TO_DEVICE);
	ret = dma_mapping_error(&priv->pci_dev->dev, skb_paddr);
	if (ret)
	{
		pr_err("%s : srng test dma err \n", WIRELESS_SIMU_DEVICE_NAME);
		goto err_free_skb;
	}
	struct wireless_simu_skb_cb *skb_cb = WIRELESS_SIMU_SKB_CB(skb);
	skb_cb->paddr = skb_paddr;
	pr_info("%s : hal srng test data %llx paddr %d size %08x eg\n", WIRELESS_SIMU_DEVICE_NAME, skb_cb->paddr, skb->len, *(skb->data + 25));

	for (u32 test_count = 0; test_count < 50; test_count++)
	{
		/* 数据传输 */
		memcpy(skb->data, &test_count, sizeof(test_count));
		ret = wireless_simu_hal_srng_test_send(&st, skb, 0, 0);
		if (ret)
		{
			pr_err("%s : err srng test send index %08x code %d \n", WIRELESS_SIMU_DEVICE_NAME, test_count, ret);
			goto err_free_dma;
		}
		pr_info("%s : test hal src post %08x \n", WIRELESS_SIMU_DEVICE_NAME, test_count);
	}

	msleep(100);

err_free_dma:
	dma_unmap_single(&priv->pci_dev->dev, skb_paddr, skb->len, DMA_TO_DEVICE);
err_free_skb:
	dev_kfree_skb_any(skb);
err_free_data:
	kfree(data);
err_clear_pipes:
	hal_srng_test_free_pipes(&st);
err_clear_config:
	kfree(st.host_config);
	return;
}

static u32 wireless_simu_test_dst_get_length(void *buf)
{
	struct hal_test_dst_status *desc = buf;
	u32 ret = 0;
	ret = desc->buffer_length;

	return ret;
}

/* 遍历 skb 中的 dst_status 来寻找对应的设备上传的skb
 */
static int wireless_simu_test_dst_recv_next(struct srng_test_pipe *pipe, struct sk_buff **skb, int *nbytes)
{
	struct wireless_simu *priv = pipe->priv;
	struct hal_srng *srng;
	unsigned int sw_index;
	unsigned int nentries_mask;
	u32 *desc;
	int ret = 0;

	spin_lock_bh(&priv->st_dst.srng_test_lock);

	sw_index = pipe->dst_ring->sw_index;
	nentries_mask = pipe->dst_ring->nentries_mask;

	srng = &priv->hal.srng_list[pipe->status_ring->hal_ring_id];

	spin_lock_bh(&srng->lock);

	wireless_simu_hal_srng_access_begin(priv, srng);

	// pr_info("%s : status hp %08x \n", WIRELESS_SIMU_DEVICE_NAME, srng->u.dst_ring.cached_hp);

	desc = wireless_simu_hal_srng_dst_get_next_entry(priv, srng);
	if (!desc)
	{
		ret = -EIO;
		goto err;
	}

	*nbytes = wireless_simu_test_dst_get_length(desc);
	if (*nbytes == 0)
	{
		ret = -EIO;
		goto err;
	}

	pr_info("%s : read stauts sw_index %08x nbytes %08x \n", WIRELESS_SIMU_DEVICE_NAME, sw_index, *nbytes);

	*skb = pipe->dst_ring->skb[sw_index];
	pipe->dst_ring->skb[sw_index] = NULL;

	sw_index = ((sw_index + 1) & nentries_mask);
	pipe->dst_ring->sw_index = sw_index;

	pipe->rx_buf_needed++;
err:
	wireless_simu_hal_srng_access_end(priv, srng);

	spin_unlock_bh(&srng->lock);

	spin_unlock_bh(&priv->st_dst.srng_test_lock);

	return ret;
}

/*
 * 将dma地址通过类型为 SRC 的 RING 下发
 * 这个被读取走以后无所谓的，等下次下发覆盖掉就好
 * 整个发送过程仿照src数据的发送
 */
static int wireless_simu_test_dst_enqueue_pipe(struct srng_test_pipe *pipe, struct sk_buff *skb, dma_addr_t paddr)
{
	struct wireless_simu *priv = pipe->priv;
	struct srng_test_ring *ring = pipe->dst_ring;
	struct hal_srng *srng;
	unsigned int write_index;
	unsigned int nentries_mask = ring->nentries_mask;
	u32 *desc;
	int ret;

	lockdep_assert_held(&priv->st_dst.srng_test_lock);

	write_index = ring->write_index;

	srng = &priv->hal.srng_list[ring->hal_ring_id];

	spin_lock_bh(&srng->lock);

	wireless_simu_hal_srng_access_begin(priv, srng);

	if (unlikely(wireless_simu_hal_srng_src_num_free(priv, srng, false) < 1))
	{
		ret = -ENOSPC;
		goto exit;
	}

	desc = wireless_simu_hal_srng_src_get_next_entry(priv, srng);
	if (!desc)
	{
		ret = -ENOSPC;
		goto exit;
	}

	wireless_simu_test_dst_set_desc(desc, paddr);

	ring->skb[write_index] = skb;

	pr_info("%s : dst_ring %08x skb %08x paddr %llx \n",
			WIRELESS_SIMU_DEVICE_NAME, ring->hal_ring_id, write_index,
			WIRELESS_SIMU_SKB_CB(ring->skb[write_index])->paddr);

	write_index = (write_index + 1) & nentries_mask;
	ring->write_index = write_index;

	pipe->rx_buf_needed--;

	ret = 0;

exit:
	wireless_simu_hal_srng_access_end(priv, srng);

	spin_unlock_bh(&srng->lock);

	return ret;
}

/* 向dst ring 中填充 dma地址 */
static int wireless_simu_test_dst_post_pipe(struct srng_test_pipe *pipe)
{
	struct wireless_simu *priv = pipe->priv;
	struct sk_buff *skb;
	dma_addr_t paddr;
	int ret = 0;

	/* info
	 * 在高通 ath11k 和 ath12k 中该处的代码使用的是 || 来进行条件的判断, 感觉逻辑上应该使用 && 来进行判断才对
	 */
	if (!(pipe->dst_ring && pipe->status_ring))
		return 0;

	spin_lock_bh(&priv->st_dst.srng_test_lock);
	while (pipe->rx_buf_needed)
	{
		skb = dev_alloc_skb(pipe->buf_sz); // 在alloc_pipes里填充的buf_sz
		if (!skb)
		{
			ret = -ENOMEM;
			goto exit;
		}

		WARN_ON_ONCE(!IS_ALIGNED((unsigned long)skb->data, 4));

		paddr = dma_map_single(&priv->pci_dev->dev, skb->data, skb->len + skb_tailroom(skb), DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(&priv->pci_dev->dev, paddr)))
		{
			pr_err("%s : test dst post pipe fail dma map rx \n", WIRELESS_SIMU_DEVICE_NAME);
			ret = -EIO;
			goto exit;
		}

		WIRELESS_SIMU_SKB_CB(skb)->paddr = paddr;

		ret = wireless_simu_test_dst_enqueue_pipe(pipe, skb, paddr);

		if (ret)
		{
			pr_err("%s : test dst post pipe fail enqueue rx buf %d \n", WIRELESS_SIMU_DEVICE_NAME, ret);
			dma_unmap_single(&priv->pci_dev->dev, paddr, skb->len + skb_tailroom(skb), DMA_FROM_DEVICE);
			dev_kfree_skb_any(skb);
			goto exit;
		}
	}

exit:
	spin_unlock_bh(&priv->st_dst.srng_test_lock);
	return ret;
}

/* 从硬件中提取接受到的DMA数据
 * 该方法会将所有硬件中准备的数据（dst_status ring）全部接收至驱动中，并填充DMA地址（dst ring）准备接收下一批数据
 */
static void wireless_simu_irq_hal_srng_dst_dma_test(struct wireless_simu *priv, int pipe_id)
{
	/*
	 * 在这个方法中要完成两件事：
	 * 1. 遍历RX数据RING，若DMA地址不为空就读取对应id的数据，并释放对应的DMA空间，尾指针++
	 * 2. 若DMA地址为空，则申请DMA空间，填充id，填充src ring向设备进行通知，尾指针++ */

	struct srng_test *st = &priv->st_dst;
	struct srng_test_pipe *pipe = &st->pipes[pipe_id];
	int ret = 0;

	// 遍历DST_STATUS 寻找已经填充完成的RX数据
	struct sk_buff *skb;
	struct sk_buff_head list;
	unsigned int nbytes, max_nbytes;

	__skb_queue_head_init(&list);
	while (wireless_simu_test_dst_recv_next(pipe, &skb, &nbytes) == 0)
	{
		max_nbytes = skb->len + skb_tailroom(skb);
		dma_unmap_single(&priv->pci_dev->dev, WIRELESS_SIMU_SKB_CB(skb)->paddr, max_nbytes, DMA_FROM_DEVICE);
		if (unlikely(max_nbytes < nbytes))
		{
			pr_err("%s : err data from dst ring long nbytes %d max_bytes %d\n",
				   WIRELESS_SIMU_DEVICE_NAME, nbytes, max_nbytes);
			dev_kfree_skb_any(skb);
			continue;
		}

		skb_put(skb, nbytes);
		__skb_queue_tail(&list, skb);
	}

	while ((skb = __skb_dequeue(&list)))
	{
		/* skb 逻辑处理
		 * 打印
		 */
		DEFINE_SPINLOCK(print_skb_lock);
		spin_lock(&print_skb_lock);
		print_hex_dump(KERN_INFO, "wireless_simu : skb : ", DUMP_PREFIX_NONE, 16, 1, skb->data, skb->len, false);
		spin_unlock(&print_skb_lock);
		
		dev_kfree_skb_any(skb);
	}

	// 遍历 DST 申请DMA空间并填充至其中
	ret = wireless_simu_test_dst_post_pipe(pipe);
	if (ret && ret != -ENOSPC)
	{
		pr_err("%s : err dst ring post rx buf pipe %d ret %d \n",
			   WIRELESS_SIMU_DEVICE_NAME, pipe->pipe_num, ret);

		mod_timer(&priv->st_dst.rx_replenish_retry, jiffies + WIRELESS_SIMU_RX_POST_RETRY_JIFFIES);
	}
}

/* 遍历所有dst_ring, 填充dma地址 */
static void wireless_simu_test_dst_post_buf(struct wireless_simu *priv)
{
	struct srng_test_pipe *pipe;
	int i;
	int ret;

	for (i = 0; i < priv->st_dst.pipes_count; i++)
	{
		pipe = &priv->st_dst.pipes[i];
		ret = wireless_simu_test_dst_post_pipe(pipe);
		if (ret)
		{
			if (ret == -ENOSPC)
				continue;

			pr_err("%s : dst test ring fail post buf to pipe %d \n", WIRELESS_SIMU_DEVICE_NAME, i);
			mod_timer(&priv->st_dst.rx_replenish_retry, jiffies + WIRELESS_SIMU_RX_POST_RETRY_JIFFIES);
			return;
		}
	}
}

static void wireless_simu_test_dst_rx_replenish_retry(struct timer_list *t)
{
	struct srng_test *st = from_timer(st, t, rx_replenish_retry);
	struct wireless_simu *priv = st->priv;

	wireless_simu_test_dst_post_buf(priv);
}

/* 中断下半部 */
static void wireless_simu_hal_srng_dst_tasklet(struct tasklet_struct *t)
{
	struct srng_test_pipe *pipe = from_tasklet(pipe, t, intr_tq);
	struct wireless_simu *priv = pipe->priv;

	// pr_info("%s : task start \n", WIRELESS_SIMU_DEVICE_NAME);

	wireless_simu_irq_hal_srng_dst_dma_test(priv, pipe->pipe_num);

	// 最终有一个我不理解的enable_irq，不知道为什么会在下半部发生这种调用
}

int wireless_simu_hal_srng_dst_test_init(struct wireless_simu *priv)
{
	pr_info("%s : srng dst init start \n", WIRELESS_SIMU_DEVICE_NAME);

	struct srng_test *st = &priv->st_dst;
	struct srng_test_pipe *pipe;
	int ret = 0;

	st->priv = priv;
	st->host_config = kmemdup(srng_test_dst_configs, sizeof(srng_test_dst_configs), GFP_KERNEL);
	if (!st->host_config)
	{
		pr_err("%s : srng dst config mem err \n", WIRELESS_SIMU_DEVICE_NAME);
		return -ENOBUFS;
	}

	spin_lock_init(&st->srng_test_lock);

	/* 超时处理的初始化，当dst的ring为空时，过一段时间后会自动重试 */
	timer_setup(&priv->st_dst.rx_replenish_retry, wireless_simu_test_dst_rx_replenish_retry, 0);

	st->pipes_count = SRNG_TEST_PIPE_COUNT_MAX;
	/* 中断系统的初始化，dst需要使用上下半部的中断系统
	 * 为每一个pipe初始化一个中断处理列表
	 */
	for (int i = 0; i < SRNG_TEST_PIPE_COUNT_MAX; i++)
	{
		tasklet_setup(&st->pipes[i].intr_tq, wireless_simu_hal_srng_dst_tasklet);
	}

	// 为hal srng 分配mem空间
	for (int i = 0; i < SRNG_TEST_PIPE_COUNT_MAX; i++)
	{
		ret = hal_srng_test_alloc_pipe(st, i);
		if (ret)
		{
			hal_srng_test_free_pipes(st);
			goto err_clear_free_configs;
		}
	}

	for (int i = 0; i < SRNG_TEST_PIPE_COUNT_MAX; i++)
	{
		pipe = &st->pipes[i];
		if (pipe->src_ring)
		{
			ret = -EINVAL;
			pr_err("%s : err no src ring in dst test \n", WIRELESS_SIMU_DEVICE_NAME);
			goto err_clear_free_pipes;
		}

		if (pipe->dst_ring && pipe->status_ring)
		{
			// pr_info("%s : dst status ring register to hal_srng \n", WIRELESS_SIMU_DEVICE_NAME);

			ret = hal_srng_test_init_ring(priv, pipe->dst_ring, i, HAL_TEST_SRNG_DST);
			ret |= hal_srng_test_init_ring(priv, pipe->status_ring, i, HAL_TEST_SRNG_DST_STATUS);

			if (ret)
			{
				goto err_clear_free_pipes;
			}

			pipe->dst_ring->write_index = 0;
			pipe->dst_ring->sw_index = 0;
			pipe->rx_buf_needed = pipe->dst_ring->nentries ? pipe->dst_ring->nentries - 2 : 0; // 为之后的dma空间申请做准备，不过为什么要减2？

			pipe->status_ring->sw_index = 0;
			pipe->status_ring->write_index = 0;

			pr_info("%s : dst ring id %d, dst status ring id %d \n",
					WIRELESS_SIMU_DEVICE_NAME, pipe->dst_ring->hal_ring_id, pipe->status_ring->hal_ring_id);
		}
		else
		{
			pr_err("%s : must dst and status \n", WIRELESS_SIMU_DEVICE_NAME);
			goto err_clear_free_pipes;
		}
	}

	// 在初始化dst完成后，需要预先分配dma地址
	wireless_simu_test_dst_post_buf(priv);

	// 没有构造数据，数据需要等待device进行传输
	// 需要等待中断模块完成

	pr_info("%s : dst status ring init success \n", WIRELESS_SIMU_DEVICE_NAME);

	return 0;

err_clear_free_pipes:
	hal_srng_test_free_pipes(st);
err_clear_free_configs:
	kfree(st->host_config);
	return ret;
}

static void wireless_simu_hal_srng_dst_test_cleanuprx(struct srng_test_pipe *pipe)
{
	struct wireless_simu *priv = pipe->priv;
	struct srng_test_ring *ring = pipe->dst_ring;
	struct sk_buff *skb;

	if (!ring && pipe->buf_sz)
		return;

	for (int i = 0; i < ring->nentries; i++)
	{
		skb = ring->skb[i];
		if (!skb)
			continue;

		ring->skb[i] = NULL;
		dma_unmap_single(&priv->pci_dev->dev, WIRELESS_SIMU_SKB_CB(skb)->paddr, skb->len + skb_tailroom(skb), DMA_FROM_DEVICE);
		dev_kfree_skb_any(skb);
	}
}

void wireless_simu_hal_srng_dst_test_deinit(struct wireless_simu *priv)
{
	pr_info("%s : srng dst deinit start \n", WIRELESS_SIMU_DEVICE_NAME);

	struct srng_test *st = &priv->st_dst;
	struct srng_test_pipe *pipe;

	spin_lock(&st->srng_test_lock);

	// 解除dst_srng test内各个中断子系统的下半部
	for (int i = 0; i < st->pipes_count; i++)
	{
		tasklet_kill(&st->pipes[i].intr_tq);
	}

	for (int pipe_num = 0; pipe_num < st->pipes_count; pipe_num++)
	{
		pipe = &st->pipes[pipe_num];
		wireless_simu_hal_srng_dst_test_cleanuprx(pipe);
	}

	hal_srng_test_free_pipes(st);

	kfree(st->host_config);

	spin_unlock(&st->srng_test_lock);

	return;
}