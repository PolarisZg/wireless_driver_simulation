#include "wireless.h"

void wireless_hif_write32(struct wireless_simu *priv, u32 address, u32 data)
{
    iowrite32(data, priv->mmio_addr + address);
}