#ifndef WIRELESS_SIMU_ERRCODE
#define WIRELESS_SIMU_ERRCODE

enum wireless_simu_err_code
{
    SUCCESS = 0,

    // dma_tx_ring_init
    TX_RING_INIT_MALLOC_ERR,
    TX_RING_FULL,
};
#endif /*WIRELESS_SIMU_ERRCODE*/