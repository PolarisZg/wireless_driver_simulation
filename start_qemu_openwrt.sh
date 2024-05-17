set -x

QEMUPATH=$1

$QEMUPATH -m 1024 -smp 2 -cpu cortex-a57 -M virt -nographic -kernel openwrt-21.02.0-armvirt-64-Image-initramfs -drive if=none,file=openwrt-21.02.0-armvirt-64-rootfs-ext4.img,id=hd0 -device virtio-blk-device,drive=hd0 