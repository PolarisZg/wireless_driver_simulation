#include "wireless.h"

void wireless_hif_write32(struct wireless_simu *priv, u32 address, u32 data)
{
    // pr_info("%s : hif write32 addr %08x data %08x\n", WIRELESS_SIMU_DEVICE_NAME, address, data);
    iowrite32(data, priv->mmio_addr + address);
}

u32 wireless_hif_read32(struct wireless_simu *priv, u32 address)
{
    u32 val = 0;

    val = ioread32(priv->mmio_addr + address);

    return val;
}