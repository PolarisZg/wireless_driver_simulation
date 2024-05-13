CURRENT_DIR=$(pwd)

KERNEL_DIR=/usr/src/linux-headers-6.5.0-28-generic

set -x
for file in *.c; do
    FILE_NAME=$(basename -- "$file" .c)
    make KERNELDIR=$KERNEL_DIR PWD=$CURRENT_DIR FILENAME=$FILE_NAME
done