#include "wireless.h"

irqreturn_t wireless_simu_irq_handler(int irq, void *dev)
{
    struct pci_dev *pdev = dev;
    struct wireless_simu *priv = pci_get_drvdata(pdev);

    u32 irq_status = wireless_hif_read32(priv, HAL_BASIC_REG(WIRELESS_REG_BASIC_IRQ_STATUS));
    pr_info("%s : interrupt start, irq_status %08x \n", WIRELESS_SIMU_DEVICE_NAME, irq_status);

    switch (irq_status)
    {
    case WIRELESS_SIMU_IRQ_STATU_SRNG_DST_DMA_TEST_RING_0:
        tasklet_schedule(&priv->st_dst.pipes[0].intr_tq);
        // pr_info("%s : interrupt dst sche end \n", WIRELESS_SIMU_DEVICE_NAME);
        break;
    case WIRELESS_SIMU_IRQ_STATUS_MGMT_TX_END ... WIRELESS_SIMU_IRQ_STATUS_MGMT_TX_END_TAIL:
        // 暂时只有 pipe0 发送 mgmt 帧
        tasklet_schedule(&priv->ce.ce_pipe[irq_status - WIRELESS_SIMU_IRQ_STATUS_MGMT_TX_END].intr_tq);
        break;
    default:
        break;
    }

    /* 清中断 向状态寄存器写0 */
    wireless_hif_write32(priv, HAL_BASIC_REG(WIRELESS_REG_BASIC_IRQ_STATUS), 0);

    return IRQ_HANDLED;
}