CURRENT_DIR=$(pwd)

KERNEL_VERSION=(uname -r)
KERNEL_DIR=/usr/src/linux-headers-$KERNEL_VERSION

set -x
for file in *.c; do
    FILE_NAME=$(basename -- "$file" .c)
    make KERNELDIR=$KERNEL_DIR PWD=$CURRENT_DIR FILENAME=$FILE_NAME
done