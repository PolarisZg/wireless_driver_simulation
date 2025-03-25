set -x

# wget https://mirrors.aliyun.com/openwrt/releases/21.02.0/targets/armvirt/64/openwrt-21.02.0-armvirt-64-Image-initramfs

# wget https://mirrors.aliyun.com/openwrt/releases/21.02.0/targets/armvirt/64/openwrt-21.02.0-armvirt-64-rootfs-ext4.img.gz

# gzip -d openwrt-21.02.0-armvirt-64-rootfs-ext4.img.gz 

timestamp=$(date +"%Y%m%d%H%M%S")

sudo ../qemu_simudevice/build/qemu-system-aarch64 \
-m 16G \
-smp 2 \
-cpu cortex-a57 \
-M virt \
-nographic \
-kernel openwrt-21.02.0-armvirt-64-Image-initramfs \
-drive if=none,file=openwrt-21.02.0-armvirt-64-rootfs-ext4.img,id=hd0 \
-device virtio-blk-device,drive=hd0 \
-net tap -net nic \
-device wirelesssimu \
2>&1 | tee qemu_stdout.${timestamp}.log

# QEMUPATH=$1

# sudo \
# $QEMUPATH -M virt -nographic \
# -m 1024 \
# -cpu cortex-a57 -smp 2 \
# -kernel openwrt-21.02.0-armvirt-64-Image-initramfs \
# -blockdev driver=raw,node-name=hd0,cache.direct=on,file.driver=file,file.filename=openwrt-21.02.0-armvirt-64-rootfs-ext4.img \
# -device virtio-blk-pci,drive=hd0 \
# -device virtio-net-pci,netdev=lan \
# -netdev tap,id=lan,ifname=ubuntu_tap0,script=no,downscript=no \
# -netdev type=tap,id=nic1,ifname=ubuntu_tap0,script=no,downscript=no \
# -device virtio-net-pci,disable-legacy=on,disable-modern=off,netdev=nic1,mac=ba:ad:1d:ea:01:02 