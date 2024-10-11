#ifndef WIRELESS_SIMU_DPTX
#define WIRELESS_SIMU_DPTX
#include "wireless.h"

// mac80211加密方式转firmware支持加密方式
enum hal_encrypt_type wireless_simu_dp_tx_get_encrypt_type(u32 cipher);

#endif /*WIRELESS_SIMU_DPTX*/