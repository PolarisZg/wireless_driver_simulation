#ifndef TEST_PCI
#define TEST_PCI
#define HELLOPCI_BUFF_SIZE 0x100

struct test_pci {
    struct pci_dev *pci_dev;
    void __iomem *mmio_addr;
    u32 cdev_num;
    struct cdev *char_dev;
    struct class *dev_class;
};

#endif