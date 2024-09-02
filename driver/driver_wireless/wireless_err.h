#ifndef WIRELESS_SIMU_ERRCODE
#define WIRELESS_SIMU_ERRCODE

enum wireless_simu_err_code
{
    SUCCESS = 0,
    STOP,

    // dma_tx_ring_init
    TX_RING_INIT_MALLOC_ERR,
    TX_RING_FULL,

    // dma_rx_ring_init
    RX_RING_INIT_MALLOC_ERR,

    // mac80211_core
    MAC80211_ALLOC_ERR,
    MAC80211_REGISTER_ERR,
};
#endif /*WIRELESS_SIMU_ERRCODE*/