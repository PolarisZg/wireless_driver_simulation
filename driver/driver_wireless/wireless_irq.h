#ifndef WIRELESS_SIMU_IRQ
#define WIRELESS_SIMU_IRQ

#include "wireless.h"

/* 弃用 */
enum Wireless_DMA_IRQ_STATUS
{
    WIRELESS_IRQ_TEST = 0,
    WIRELESS_IRQ_DNA_MEM_TO_DEVICE_END,
    WIRELESS_IRQ_DMA_DELALL_END,
    WIRELESS_IRQ_RX_START,
    WIRELESS_IRQ_DMA_DEVICE_TO_MEM_END,
    WIRELESS_IRQ_MAC80211_TX_COMPLETE,
    WIRELESS_IRQ_MAC80211_RX
};

enum WIRELESS_SIMU_ENUM_IRQ_STATUS
{
    WIRELESS_SIMU_IRQ_STATU_START = 0,
    WIRELESS_SIMU_IRQ_STATU_SRNG_DST_DMA_TEST_RING_0,
};

irqreturn_t wireless_simu_irq_handler(int irq, void *dev);

#endif