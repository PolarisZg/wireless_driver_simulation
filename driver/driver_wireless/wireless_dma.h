#ifndef WIRELESS_SIMU_DMA
#define WIRELESS_SIMU_DMA

#include <linux/pci.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/msi.h>
#include <asm-generic/io.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/dmaengine.h>

#include <linux/skbuff.h>
#include <linux/mutex.h>

#include "wireless.h"
#include "wireless_err.h"

enum wireless_simu_err_code 
wireless_simu_tx_ring_init(struct wireless_simu *priv);

/*
 * dma 发送数据至设备
 *
 * @date 必须是内存中连续数据
 * @length 数据长度
 * */
enum wireless_simu_err_code 
wireless_simu_dma_tx(struct wireless_simu *priv, void* data, size_t length);

#endif /*WIRELESS_SIMU_DMA*/