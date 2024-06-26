# uboot.elf download from sudo apt install uboot
# cp /usr/lib/u-boot/qemu-riscv64_smode/uboot.elf ./

sudo ../qemu_simudevice/build/qemu-system-riscv64 \
-machine virt -nographic -m 2048 -smp 4 \
-drive file=ubuntu-24.04-preinstalled-server-riscv64.img,format=raw,if=virtio \
-kernel ./uboot.elf \
-device virtio-net-device,netdev=eth0 -netdev tap,id=eth0 \
-device virtio-rng-pci \
-device hellodev-pci