obj-m += simu_wireless.o 

simu_wireless-objs := \
wireless.o \
wireless_dma.o \
wireless_mac80211.o \
wireless_dptx.o \
wireless_dprx.o \
wireless_wmi.o \
wireless_htc.o \
wireless_ce.o \
wireless_hal.o \
wireless_hif.o \
wireless_irq.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

wsl:
	make -C ../../linux-stable M=$(shell pwd) modules

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order *.o.d *.mod *.cmd .*.cmd