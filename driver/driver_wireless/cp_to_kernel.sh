set -x

cp ./*.c ../../linux-stable/drivers/net/wireless/wirelesssimu/
cp ./*.h ../../linux-stable/drivers/net/wireless/wirelesssimu/

make clean

make aarch64

mkdir tmproofs

sudo mount -t ext3 ../../script/aarchrootfs.etx3 tmproofs

sudo cp ./*.ko tmproofs/root/modules

sudo umount tmproofs

rm -rf tmproofs

make clean