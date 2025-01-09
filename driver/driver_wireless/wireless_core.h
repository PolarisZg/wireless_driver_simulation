#ifndef WIRELESS_SIMU_CORE
#define WIRELESS_SIMU_CORE

#include "wireless.h"

static inline struct wireless_simu_vif *wireless_vif_to_wivif(struct ieee80211_vif *vif)
{
    return (struct wireless_simu_vif *)vif->drv_priv;
}

static inline struct wireless_simu_sta *wireless_sta_to_wista(struct ieee80211_sta *sta)
{
    return (struct wireless_simu_sta *)sta->drv_priv;
}

#endif // WIRELESS_SIMU_CORE