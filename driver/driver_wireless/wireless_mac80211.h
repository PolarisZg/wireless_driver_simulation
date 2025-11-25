#ifndef WIRELESS_MAC80211
#define WIRELESS_MAC80211

#include "wireless.h"
#include "wireless_wmi.h"

enum wmi_vdev_subtype;
enum wmi_vdev_type;

#define WIRELESS_SIMU_HW_QUEUE 4

// 80211帧加密方式，使用枚举单独列出便于firmware端同步
enum hal_encrypt_type
{
	HAL_ENCRYPT_TYPE_WEP_40,
	HAL_ENCRYPT_TYPE_WEP_104,
	HAL_ENCRYPT_TYPE_TKIP_NO_MIC,
	HAL_ENCRYPT_TYPE_WEP_128,
	HAL_ENCRYPT_TYPE_TKIP_MIC,
	HAL_ENCRYPT_TYPE_WAPI,
	HAL_ENCRYPT_TYPE_CCMP_128,
	HAL_ENCRYPT_TYPE_OPEN,
	HAL_ENCRYPT_TYPE_CCMP_256,
	HAL_ENCRYPT_TYPE_GCMP_128,
	HAL_ENCRYPT_TYPE_AES_GCMP_256,
	HAL_ENCRYPT_TYPE_WAPI_GCM_SM4,
};

enum wireless_simu_skb_cb_flags
{
	WIRELESS_SIMU_SKB_HW_80211_ENCAP = BIT(0),
	WIRELESS_SIMU_SKB_CIPHER_SET = BIT(1),
};

struct wireless_skb_cb
{
	struct wireless_simu *priv;
	dma_addr_t paddr;
	u8 flags;
	u8 eid;
	u32 cipher;
	struct ieee80211_vif *vif;
} __packed;

static inline struct wireless_skb_cb *WIRELESS_SKB_CB(struct sk_buff *skb)
{
	BUILD_BUG_ON(sizeof(struct wireless_skb_cb) >
				 IEEE80211_TX_INFO_DRIVER_DATA_SIZE);
	return (struct wireless_skb_cb *)&IEEE80211_SKB_CB(skb)->driver_data;
}

struct wireless_simu_tx_params_arg
{
	u8 acm;
	u8 aifs;
	u16 cwmin;
	u16 cwmax;
	u16 txop;
	u8 no_ack;
};

struct wireless_simu_vif
{
	u32 vdev_id;
	enum wmi_vdev_type vdev_type;
	enum wmi_vdev_subtype vdev_subtype;

	struct wireless_simu *priv;
	struct ieee80211_vif *vif;
	int vif_id;
	struct wireless_simu_tx_params_arg ac_be;
	struct wireless_simu_tx_params_arg ac_bk;
	struct wireless_simu_tx_params_arg ac_vi;
	struct wireless_simu_tx_params_arg ac_vo;

	struct {
		u32 bssid_low;
		u32 bssid_hi;
		struct delayed_work beacon_work;
		u16 beacon_int;
		bool enable_beacon;
	}simu_simple;
};

struct wireless_simu_sta
{
	struct wireless_simu *priv;
};


enum wireless_simu_err_code
wireless_mac80211_core_probe(struct wireless_simu *priv);

int wireless_mac80211_core_remove(struct wireless_simu *priv);

int wireless_mac80211_wait_tx_complete(struct wireless_simu *priv);

void wireless_mac80211_drain_tx(struct wireless_simu *priv);

void wireless_sample_send_cb(struct wireless_simu *priv, struct sk_buff *skb);

void wireless_sample_recv_cb(struct wireless_simu *priv, struct sk_buff *skb);

#endif /*WIRELESS_MAC80211*/