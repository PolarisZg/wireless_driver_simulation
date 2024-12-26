set -x

ORIGINAL_DIR=$(pwd)

cd ../linux-stable
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- all -j$(nproc)
cd $ORIGINAL_DIR

cd ../driver/driver_wireless/cp_to_kernel.sh
./cp_to_kernel.sh
cd $ORIGINAL_DIR

cd ../qemu_simudevice//build
ninja
cd $ORIGINAL_DIR

../qemu_simudevice/build/qemu-system-aarch64 -M virt -cpu cortex-a53 -m 16G \
-nographic \
-kernel ../linux-stable/arch/arm64/boot/Image \
-append "nokaslr console=ttyAMA0 root=/dev/vda rw" \
-device virtio-blk-device,drive=disk \
-drive file=aarchrootfs.etx3,format=raw,if=none,id=disk \
-device edu \
-device wirelesssimu