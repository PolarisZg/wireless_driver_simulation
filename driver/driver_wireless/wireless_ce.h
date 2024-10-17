#ifndef WIRELESS_SIMU_CE
#define WIRELESS_SIMU_CE

#include "wireless.h"

/* no interrupt on copy completion */
#define CE_ATTR_DIS_INTR		8

int wireless_simu_ce_send(struct wireless_simu *priv, struct sk_buff *skb);

#endif