CURRENT_PATH=$(pwd)

BUILDROOT_QEMU=$CURRENT_PATH/../qemu_simudevice/build/qemu-system-x86_64

BUILDROOT_LINUX=$CURRENT_PATH/../buildroot/output/build/linux-5.10.7/arch/x86_64/boot/bzImage

BUILDROOT_LINUXFS=$CURRENT_PATH/../buildroot/output/images/rootfs.ext2

$BUILDROOT_QEMU -M pc \
-nographic \
-kernel $BUILDROOT_LINUX \
-drive file=$BUILDROOT_LINUXFS,if=virtio,format=raw \
-append "rootwait root=/dev/vda console=tty1 console=ttyS0" \
-net nic,model=virtio 