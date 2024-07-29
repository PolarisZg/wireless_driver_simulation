obj-m += simu_wireless.o 
simu_wireless-objs := wireless.o wireless_dma.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order *.o.d *.mod *.cmd .*.cmd