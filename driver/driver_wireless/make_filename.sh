CURRENT_DIR=$(pwd)

KERNEL_VERSION=$(uname -r)
KERNEL_DIR=/usr/src/linux-headers-$KERNEL_VERSION

set -x
filename_temp=$(basename -- "$1")
FILE_NAME=${filename_temp%.c}
make KERNELDIR=$KERNEL_DIR PWD=$CURRENT_DIR FILENAME=$FILE_NAME