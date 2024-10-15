#include "wireless.h"

static const struct hal_srng_config hw_srng_config_template[] = {
	{
		.start_ring_id = HAL_SRNG_RING_ID_TEST_SW2HW,
		.max_rings = 1,
		.entry_size = sizeof(struct hal_test_sw2hw) >> 2, // 右移 2 位代表以32bit为单位
		.lmac_ring = false,
		.ring_dir = HAL_SRNG_DIR_SRC,
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
	pr_err("%s : no dst ring \n", WIRELESS_SIMU_DEVICE_NAME);
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

	// srng->hwreg_base 的初始化，该处和硬件设计强相关，需特别注意寄存器的功能
	for (int i = 0; i < HAL_SRNG_NUM_REG_GRP; i++)
	{
		srng->hwreg_base[i] = srng_config->reg_start[i] + (ring_num * srng_config->reg_size[i]);
	}
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
			srng->u.dst_ring.tp_addr = (u32 *)((unsigned long)priv->mmio_addr + reg_base);
			pr_info("%s : srng setup %d type %d ring num hp_addr %lx \n", WIRELESS_SIMU_DEVICE_NAME, type, ring_id, (unsigned long)srng->u.dst_ring.tp_addr);
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
	struct hal_srng_config *s;
	hal->srng_config = kmemdup(hw_srng_config_template, sizeof(hw_srng_config_template), GFP_KERNEL);

	if (!hal->srng_config)
	{
		return -ENOMEM;
	}

	s = &hal->srng_config[HAL_TEST_SRNG];
	s->reg_start[HAL_SRNG_REG_GRP_R0] = HAL_TEST_SRNG_REG_GRP; // srng配置信息 【寄存器组】 的基地址, 这里面存放的是相对于mmio的偏移地址
	s->reg_size[HAL_SRNG_REG_GRP_R0] = HAL_TEST_SRNG_REG_GRP_R0_SIZE; // srng配置信息寄存器组中寄存器的数量
	s->reg_start[HAL_SRNG_REG_GRP_R2] = HAL_TEST_SRNG_REG_GRP_R2;
	s->reg_size[HAL_SRNG_REG_GRP_R2] = HAL_TEST_SRNG_REG_GRP_R2_SIZE;


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
	pr_info("%s : srng hal rdp vaddr %p paddr %llx \n",WIRELESS_SIMU_DEVICE_NAME, hal->rdp.vaddr, hal->rdp.paddr);

	ret = hal_alloc_cont_wdp(hal);
	if (ret)
		goto err_free_cont_rdp;
	pr_info("%s : srng hal rdp vaddr %p paddr %llx \n",WIRELESS_SIMU_DEVICE_NAME, hal->wrp.vaddr, hal->wrp.paddr);

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

/******* srng test ********/

#define SRNG_TEST_DESC_RING_ALIGN 8

struct srng_test_attr
{
	unsigned int flags;

	/* src entry 数量 */
	unsigned int src_nentries;

	/* 每一个 entry 的最大大小, 没想到有什么意义 */
	unsigned int src_sz_max;

	/* dst entry 数量 */
	unsigned int dest_nentries;

	void (*recv_cb)(struct wireless_simu *priv, struct sk_buff *skb);
	void (*send_cb)(struct wireless_simu *priv, struct sk_buff *skb);
};

static const struct srng_test_attr srng_test_configs[] = {
	{
		.flags = 0,
		.src_nentries = 32,
		.src_sz_max = 2048,
		.dest_nentries = 0,
	},
};

struct srng_test_ring
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
	dma_addr_t base_addr_test_space_unaligned;

	/* 经由内存对齐之后的，为 entries alloc 的 memory 地址
	 * 这就产生了问题，当使用ALIGN进行对齐的时候，一方面会导致void *被修改，使整个内存空间变少，另一个dmaaddr和void *未必有关联，那万一一个改了另一个没改不就发生冲突了吗？*/
	/* 逻辑内存地址 */
	void *base_addr_owner_space;
	/* 物理内存地址 */
	dma_addr_t base_addr_test_space;

	/* hal ring id */
	u32 hal_ring_id;

	/* keep last */
	struct sk_buff *skb[];
};

struct srng_test_pipe
{
	struct wireless_simu *priv;
	u16 pipe_num;
	unsigned int attr_flags;
	unsigned int buf_sz;
	unsigned int rx_buf_needed;

	void (*recv_cb)(struct wireless_simu *priv, struct sk_buff *skb);
	void (*send_cb)(struct wireless_simu *priv, struct sk_buff *skb);

	struct tasklet_struct intr_tq;

	struct srng_test_ring *src_ring;
	struct srng_test_ring *dst_ring;
	struct srng_test_ring *status_ring;

	u64 timestamp;
};

struct srng_test
{
	struct wireless_simu *priv;
	struct srng_test_pipe pipes[SRNG_TEST_PIPE_COUNT_MAX];
	struct srng_test_attr *host_config;
	spinlock_t srng_test_lock;
};

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

	ring->base_addr_test_space_unaligned = base_addr;

	ring->base_addr_owner_space = PTR_ALIGN(ring->base_addr_owner_space_unaligned, SRNG_TEST_DESC_RING_ALIGN);
	ring->base_addr_test_space = ALIGN(ring->base_addr_test_space_unaligned, SRNG_TEST_DESC_RING_ALIGN);
	pr_info("%s : srng test ring %p vaddr %llx paddr \n", WIRELESS_SIMU_DEVICE_NAME, ring->base_addr_owner_space, ring->base_addr_test_space);

	return ring;
}

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

		// 后面dst 和 status 的就不写了
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

	// dst 方向的暂时不写了，反正也差不多，而且测试config没有dst方向

	return 0;
}

// 注册test模块到hal_srng模块，将注册的hal_srng_ring id 存放在传入的ring当中，相当于将上级模块的ring与hal_srng中的ring建立关系
static int hal_srng_test_init_ring(struct wireless_simu *priv, struct srng_test_ring *ring, int id, enum hal_ring_type type)
{
	// 因为只有一种测试用例，所以在前面追加检查
	if (type != HAL_TEST_SRNG)
	{
		pr_err("%s : test hal srng init inval type \n", WIRELESS_SIMU_DEVICE_NAME);
		return -EINVAL;
	}

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

void wireless_simu_hal_srng_test(struct wireless_simu *priv)
{
	pr_info("%s : srng test start \n", WIRELESS_SIMU_DEVICE_NAME);

	struct srng_test st;
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

	// 读写操作

	/* end free malloc */
	hal_srng_test_free_pipes(&st);
	kfree(st.host_config);
	pr_info("%s : srng test end \n", WIRELESS_SIMU_DEVICE_NAME);
	return;

err_clear_pipes:
	hal_srng_test_free_pipes(&st);
err_clear_config:
	kfree(st.host_config);
	return;
}