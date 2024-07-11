#ifndef WIRELESS_SIMU
#define WIRELESS_SIMU

struct wireless_simu {
    struct pci_dev *pci_dev;
    void __iomem *mmio_addr;
    u32 cdev_num;
    struct cdev *char_dev;
    struct class *dev_class;
    struct msi_desc *msi_desc;
    int irq_vectors_num;
};

#endif