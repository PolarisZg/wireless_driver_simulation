#ifndef WIRELESS_SIMU_HIF
#define WIRELESS_SIMU_HIF

#include "wireless.h"

void wireless_hif_write32(struct wireless_simu *priv, u32 address, u32 data);

u32 wireless_hif_read32(struct wireless_simu *priv, u32 address);

#endif