#include "wireless.h"

enum hal_encrypt_type wireless_simu_dp_tx_get_encrypt_type(u32 cipher)
{
    switch (cipher)
    {
    case WLAN_CIPHER_SUITE_WEP40:
        return HAL_ENCRYPT_TYPE_WEP_40;
    case WLAN_CIPHER_SUITE_WEP104:
        return HAL_ENCRYPT_TYPE_WEP_104;
    case WLAN_CIPHER_SUITE_TKIP:
        return HAL_ENCRYPT_TYPE_TKIP_MIC;
    case WLAN_CIPHER_SUITE_CCMP:
        return HAL_ENCRYPT_TYPE_CCMP_128;
    case WLAN_CIPHER_SUITE_CCMP_256:
        return HAL_ENCRYPT_TYPE_CCMP_256;
    case WLAN_CIPHER_SUITE_GCMP:
        return HAL_ENCRYPT_TYPE_GCMP_128;
    case WLAN_CIPHER_SUITE_GCMP_256:
        return HAL_ENCRYPT_TYPE_AES_GCMP_256;
    default:
        return HAL_ENCRYPT_TYPE_OPEN;
    }
}