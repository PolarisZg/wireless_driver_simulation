#ifndef WIRELESS_MAC80211
#define WIRELESS_MAC80211

#include "wireless.h"
#include <net/mac80211.h>

struct wireless_simu;

#define WIRELESS_SIMU_HW_QUEUE 4

enum wireless_simu_err_code
wireless_mac80211_core_probe(struct wireless_simu *priv);

int wireless_mac80211_core_remove(struct wireless_simu *priv);

// 80211帧加密方式，使用枚举单独列出便于firmware端同步
enum hal_encrypt_type {
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

enum wireless_simu_skb_cb_flags {
	WIRELESS_SIMU_SKB_HW_80211_ENCAP = BIT(0),
	WIRELESS_SIMU_SKB_CIPHER_SET = BIT(1),
};

struct wireless_simu_skb_cb{
	dma_addr_t paddr;
	u8 flags;
	u32 cipher;
	struct ieee80211_vif *vif;
}__packed;

static inline struct wireless_simu_skb_cb *WIRELESS_SIMU_SKB_CB(struct sk_buff *skb)
{
	BUILD_BUG_ON(sizeof(struct wireless_simu_skb_cb) > 
		IEEE80211_TX_INFO_DRIVER_DATA_SIZE);
	return (struct wireless_simu_skb_cb *)&IEEE80211_SKB_CB(skb)->driver_data;
}

struct wireless_simu_tx_params_arg {
	u8 acm;
	u8 aifs;
	u16 cwmin;
	u16 cwmax;
	u16 txop;
	u8 no_ack;
};

struct wireless_simu_vif
{
	struct wireless_simu *priv;
	int vif_id;
	struct wireless_simu_tx_params_arg ac_be;
	struct wireless_simu_tx_params_arg ac_bk;
	struct wireless_simu_tx_params_arg ac_vi;
	struct wireless_simu_tx_params_arg ac_vo;

};

struct wireless_simu_sta
{
	struct wireless_simu *priv;
};

#define REG_RULES { \
	REG_RULE(2412-10, 2412+10, 40, 0, 20, NL80211_RRF_NO_CCK), \
	REG_RULE(5160-10, 5865+10, 80, 0, 20, 0), \
}

static const struct ieee80211_reg_rule wireless_simu_reg_rules[] = REG_RULES;

static const struct ieee80211_regdomain wireless_simu_regd = {
	.n_reg_rules = ARRAY_SIZE(wireless_simu_reg_rules),
	.alpha2 = "99",
	.dfs_region = NL80211_DFS_ETSI,
	.reg_rules = REG_RULES,
};

static const struct ieee80211_iface_limit wireless_simu_if_limits[] = {
	{.max = 1, .types = BIT(NL80211_IFTYPE_STATION)},
	{.max = 1, .types = BIT(NL80211_IFTYPE_AP)},
	// {.max = 1, .types = BIT(NL80211_IFTYPE_MONITOR)}, // 一般需要在这里添加对monitor if的限制吗
};

static const struct ieee80211_iface_combination wireless_simu_if_comb = {
	.limits = wireless_simu_if_limits,
	.n_limits = ARRAY_SIZE(wireless_simu_if_limits),
	.max_interfaces = 2, // 对齐上方limit中max的和
	.num_different_channels = 1,
};

#define CHAN2G(_channel, _freq, _flags) { \
	.band = NL80211_BAND_2GHZ,            \
	.hw_value = (_channel),               \
	.center_freq = (_freq),               \
	.flags = (_flags),                    \
	.max_antenna_gain = 0,                \
	.max_power = 30,                      \
}

#define CHAN5G(_channel, _freq, _flags) { \
	.band = NL80211_BAND_5GHZ,            \
	.hw_value = (_channel),               \
	.center_freq = (_freq),               \
	.flags = (_flags),                    \
	.max_antenna_gain = 0,                \
	.max_power = 30,                      \
}

#define CHAN6G(_channel, _freq, _flags) { \
	.band = NL80211_BAND_6GHZ,            \
	.hw_value = (_channel),               \
	.center_freq = (_freq),               \
	.flags = (_flags),                    \
	.max_antenna_gain = 0,                \
	.max_power = 30,                      \
}

static const struct ieee80211_channel wireless_simu_2ghz_channels[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0),
};

static const struct ieee80211_channel wireless_simu_5ghz_channels[] = {
	CHAN5G(36, 5180, 0),
	CHAN5G(40, 5200, 0),
	CHAN5G(44, 5220, 0),
	CHAN5G(48, 5240, 0),
	CHAN5G(52, 5260, 0),
	CHAN5G(56, 5280, 0),
	CHAN5G(60, 5300, 0),
	CHAN5G(64, 5320, 0),
	CHAN5G(100, 5500, 0),
	CHAN5G(104, 5520, 0),
	CHAN5G(108, 5540, 0),
	CHAN5G(112, 5560, 0),
	CHAN5G(116, 5580, 0),
	CHAN5G(120, 5600, 0),
	CHAN5G(124, 5620, 0),
	CHAN5G(128, 5640, 0),
	CHAN5G(132, 5660, 0),
	CHAN5G(136, 5680, 0),
	CHAN5G(140, 5700, 0),
	CHAN5G(144, 5720, 0),
	CHAN5G(149, 5745, 0),
	CHAN5G(153, 5765, 0),
	CHAN5G(157, 5785, 0),
	CHAN5G(161, 5805, 0),
	CHAN5G(165, 5825, 0),
	CHAN5G(169, 5845, 0),
	CHAN5G(173, 5865, 0),
	CHAN5G(177, 5885, 0),
};

static const struct ieee80211_channel wireless_simu_6ghz_channels[] = {
	CHAN6G(1, 5955, 0),
	CHAN6G(5, 5975, 0),
	CHAN6G(9, 5995, 0),
	CHAN6G(13, 6015, 0),
	CHAN6G(17, 6035, 0),
	CHAN6G(21, 6055, 0),
	CHAN6G(25, 6075, 0),
	CHAN6G(29, 6095, 0),
	CHAN6G(33, 6115, 0),
	CHAN6G(37, 6135, 0),
	CHAN6G(41, 6155, 0),
	CHAN6G(45, 6175, 0),
	CHAN6G(49, 6195, 0),
	CHAN6G(53, 6215, 0),
	CHAN6G(57, 6235, 0),
	CHAN6G(61, 6255, 0),
	CHAN6G(65, 6275, 0),
	CHAN6G(69, 6295, 0),
	CHAN6G(73, 6315, 0),
	CHAN6G(77, 6335, 0),
	CHAN6G(81, 6355, 0),
	CHAN6G(85, 6375, 0),
	CHAN6G(89, 6395, 0),
	CHAN6G(93, 6415, 0),
	CHAN6G(97, 6435, 0),
	CHAN6G(101, 6455, 0),
	CHAN6G(105, 6475, 0),
	CHAN6G(109, 6495, 0),
	CHAN6G(113, 6515, 0),
	CHAN6G(117, 6535, 0),
	CHAN6G(121, 6555, 0),
	CHAN6G(125, 6575, 0),
	CHAN6G(129, 6595, 0),
	CHAN6G(133, 6615, 0),
	CHAN6G(137, 6635, 0),
	CHAN6G(141, 6655, 0),
	CHAN6G(145, 6675, 0),
	CHAN6G(149, 6695, 0),
	CHAN6G(153, 6715, 0),
	CHAN6G(157, 6735, 0),
	CHAN6G(161, 6755, 0),
	CHAN6G(165, 6775, 0),
	CHAN6G(169, 6795, 0),
	CHAN6G(173, 6815, 0),
	CHAN6G(177, 6835, 0),
	CHAN6G(181, 6855, 0),
	CHAN6G(185, 6875, 0),
	CHAN6G(189, 6895, 0),
	CHAN6G(193, 6915, 0),
	CHAN6G(197, 6935, 0),
	CHAN6G(201, 6955, 0),
	CHAN6G(205, 6975, 0),
	CHAN6G(209, 6995, 0),
	CHAN6G(213, 7015, 0),
	CHAN6G(217, 7035, 0),
	CHAN6G(221, 7055, 0),
	CHAN6G(225, 7075, 0),
	CHAN6G(229, 7095, 0),
	CHAN6G(233, 7115, 0),

	/* new addition in IEEE Std 802.11ax-2021 */
	CHAN6G(2, 5935, 0),
};

/* rate 参考 wireless_simu 的驱动代码, 不对不同的频率进行区分*/

enum wireless_simu_hw_rate_cck
{
	WIRELESS_SIMU_HW_RATE_CCK_LP_11M = 0,
	WIRELESS_SIMU_HW_RATE_CCK_LP_5_5M,
	WIRELESS_SIMU_HW_RATE_CCK_LP_2M,
	WIRELESS_SIMU_HW_RATE_CCK_LP_1M,
	WIRELESS_SIMU_HW_RATE_CCK_SP_11M,
	WIRELESS_SIMU_HW_RATE_CCK_SP_5_5M,
	WIRELESS_SIMU_HW_RATE_CCK_SP_2M,
};

enum wireless_simu_hw_rate_ofdm
{
	WIRELESS_SIMU_HW_RATE_OFDM_48M = 0,
	WIRELESS_SIMU_HW_RATE_OFDM_24M,
	WIRELESS_SIMU_HW_RATE_OFDM_12M,
	WIRELESS_SIMU_HW_RATE_OFDM_6M,
	WIRELESS_SIMU_HW_RATE_OFDM_54M,
	WIRELESS_SIMU_HW_RATE_OFDM_36M,
	WIRELESS_SIMU_HW_RATE_OFDM_18M,
	WIRELESS_SIMU_HW_RATE_OFDM_9M,
};

static struct ieee80211_rate wireless_simu_rates[] = {
	{.bitrate = 10,
	 .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_1M},
	{.bitrate = 20,
	 .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_2M,
	 .hw_value_short = WIRELESS_SIMU_HW_RATE_CCK_SP_2M,
	 .flags = IEEE80211_RATE_SHORT_PREAMBLE},
	{.bitrate = 55,
	 .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_5_5M,
	 .hw_value_short = WIRELESS_SIMU_HW_RATE_CCK_SP_5_5M,
	 .flags = IEEE80211_RATE_SHORT_PREAMBLE},
	{.bitrate = 110,
	 .hw_value = WIRELESS_SIMU_HW_RATE_CCK_LP_11M,
	 .hw_value_short = WIRELESS_SIMU_HW_RATE_CCK_SP_11M,
	 .flags = IEEE80211_RATE_SHORT_PREAMBLE},

	{.bitrate = 60, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_6M},
	{.bitrate = 90, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_9M},
	{.bitrate = 120, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_12M},
	{.bitrate = 180, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_18M},
	{.bitrate = 240, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_24M},
	{.bitrate = 360, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_36M},
	{.bitrate = 480, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_48M},
	{.bitrate = 540, .hw_value = WIRELESS_SIMU_HW_RATE_OFDM_54M},
};

#define WIRELESS_SIMU_MAC_FIRST_OFDM_RATE_IDX 4
#define wireless_simu_g_rates wireless_simu_rates
#define wireless_simu_g_rates_size (ARRAY_SIZE(wireless_simu_rates))
#define wireless_simu_a_rates (wireless_simu_rates + WIRELESS_SIMU_MAC_FIRST_OFDM_RATE_IDX)
#define wireless_simu_a_rates_size (ARRAY_SIZE(wireless_simu_rates) - WIRELESS_SIMU_MAC_FIRST_OFDM_RATE_IDX)

#endif /*WIRELESS_MAC80211*/