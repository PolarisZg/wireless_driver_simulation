obj-m = $(FILENAME).o

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.order *.o.d *.mod *.cmd .*.cmd