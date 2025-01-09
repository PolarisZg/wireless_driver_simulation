#ifndef WIRELESS_SIMU_DPTX
#define WIRELESS_SIMU_DPTX
#include "wireless.h"

struct wireless_simu;
struct wireless_simu_sta;
struct wireless_simu_vif;

// mac80211加密方式转firmware支持加密方式
enum hal_encrypt_type wireless_simu_dp_tx_get_encrypt_type(u32 cipher);

int wireless_dp_tx(struct wireless_simu *priv, struct wireless_simu_vif *wivif, struct wireless_simu_sta *wista, struct sk_buff *skb);

#endif /*WIRELESS_SIMU_DPTX*/