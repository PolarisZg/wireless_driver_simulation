
# sudo apt install bridge-utils iptables dnsmasq
# cp qemu-ifup ../qemu_simudevice/build/qemu-bundle/usr/local/etc/
# sudo chmod 755 ../qemu_simudevice/build/qemu-bundle/usr/local/etc/qemu-ifup

timestamp=$(date +"%Y%m%d%H%M%S")

sudo ../qemu_simudevice/build/qemu-system-x86_64 \
-m 16G \
-drive format=qcow2,file=ubuntu.1.qcow2 \
-enable-kvm \
-net tap -net nic \
-vnc 10.129.19.237:21 \
-device wirelesssimu \
-device edu \
-nographic \
-D qemu.log -d int,guest_errors \
2>&1 | tee qemu_stdout.${timestamp}.log
# -cdrom ./ubuntu-22.04.4-desktop-amd64.iso

# 下方的暂时无用
# -device virtio-net-pci,netdev=lan \
# -netdev tap,id=lan,ifname=ubuntu_tap0,script=no,downscript=no \