set -x

ORIGINAL_DIR=$(pwd)
MODULES_DIR=$ORIGINAL_DIR/modules
TMPROOTFS_DIR=$ORIGINAL_DIR/tmprootfs

mkdir -p $MODULES_DIR
mkdir -p $TMPROOTFS_DIR

cd ../driver/driver_wireless
./cp_to_kernel.sh
cd $ORIGINAL_DIR

cd ../linux-stable
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- all -j$(nproc)
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules_install INSTALL_MOD_PATH=$MODULES_DIR > /dev/null
cd $ORIGINAL_DIR


cd ../qemu_simudevice/build
ninja
cd $ORIGINAL_DIR

sudo mount -t ext3 aarchrootfs.etx3 $TMPROOTFS_DIR
sudo cp -r $MODULES_DIR/lib/modules/* $TMPROOTFS_DIR/lib/modules/
sudo umount $TMPROOTFS_DIR
rm -rf $TMPROOTFS_DIR
rm -rf $MODULES_DIR

../qemu_simudevice/build/qemu-system-aarch64 -M virt -cpu cortex-a53 -m 16G \
-nographic \
-kernel ../linux-stable/arch/arm64/boot/Image \
-append "nokaslr console=ttyAMA0 root=/dev/vda rw" \
-device virtio-blk-device,drive=disk \
-drive file=aarchrootfs.etx3,format=raw,if=none,id=disk \
-device edu \
-device wirelesssimu