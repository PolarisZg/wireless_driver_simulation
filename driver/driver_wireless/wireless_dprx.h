#ifndef WIRELESS_SIMU_DPRX
#define WIRELESS_SIMU_DPRX
#include "wireless.h"

struct wireless_simu; // 总之需要在一个.h文件中前向声明一次这个结构体，才能被其他的各个模块使用

int wireless_simu_dp_rx_crypto_mic_len(struct wireless_simu *priv, enum hal_encrypt_type enctype);
#endif