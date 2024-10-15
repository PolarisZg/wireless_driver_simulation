#include "wireless.h"

void wireless_hif_write32(struct wireless_simu *priv, u32 address, u32 data)
{
    pr_info("%s : hif write32 %p mmio %08x addr %08x data \n", WIRELESS_SIMU_DEVICE_NAME, priv->mmio_addr, address, data);
    iowrite32(data, priv->mmio_addr + address);
}