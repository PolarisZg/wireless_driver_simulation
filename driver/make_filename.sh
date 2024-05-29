CURRENT_DIR=$(pwd)

KERNEL_DIR=/usr/src/linux-headers-6.5.0-28-generic
set -x
filename_temp=$(basename -- "$1")
FILE_NAME=${filename_temp%.c}
make KERNELDIR=$KERNEL_DIR PWD=$CURRENT_DIR FILENAME=$FILE_NAME