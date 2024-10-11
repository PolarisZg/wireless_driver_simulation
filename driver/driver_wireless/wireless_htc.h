#ifndef WIRELESS_SIMU_HTC
#define WIRELESS_SIMU_HTC

#include "wireless.h"

#define HTC_HDR_ENDPOINTID                       GENMASK(7, 0)
#define HTC_HDR_FLAGS                            GENMASK(15, 8)
#define HTC_HDR_PAYLOADLEN                       GENMASK(31, 16)
#define HTC_HDR_CONTROLBYTES0                    GENMASK(7, 0)
#define HTC_HDR_CONTROLBYTES1                    GENMASK(15, 8)
#define HTC_HDR_RESERVED                         GENMASK(31, 16)

// 申请htc skb
struct sk_buff *wireless_simu_htc_alloc_skb(struct wireless_simu *priv, int size);

int wireless_simu_htc_send(struct wireless_simu *priv, struct sk_buff *skb);

struct wireless_simu_htc_hdr{
    u32 htc_info;
	u32 ctrl_info;
}__packed __aligned(4);

#endif