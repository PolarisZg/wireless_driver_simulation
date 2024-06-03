# set -x

CURRENT_PATH=$(pwd)

BUILDROOT_PATH=$CURRENT_PATH/../buildroot/

echo "buildroot path : $BUILDROOT_PATH"

if [ -f "$BUILDROOT_PATH/configs/qemu_mac80211_hwsim_demo_defconfig" ]; then
    echo "build root has"
else
    echo "build root nohas; download"
    git clone -b PolarisZg git@github.com:PolarisZg/buildroot.git
fi


if [ -d "$BUILDROOT_PATH/output/build/host-qemu-9.0.0" ]; then 
    echo "qemu has"
else
    echo "no qemu"
    make qemu_mac80211_hwsim_demo_defconfig
    make
fi

if [ -d "$BUILDROOT_PATH/output/build/linux-5.10.7" ]; then
    echo "linux has"
else
    echo "no linux"
    make qemu_mac80211_hwsim_demo_defconfig
    make
fi

if [ -d "$CURRENT_PATH/../qemu_simudevice" ]; then
    echo "qemu has"
else
    echo "no qemu "
    cd $CURRENT_PATH/../
    git clone -b PolarisZg_9.0.0 git@github.com:PolarisZg/qemu_simudevice.git
    cd qemu_simudevice
    mkdir build
    cd build
    ../configure --target-list=x86_64-softmmu --enable-debug
    ninja
    cd $CURRENT_PATH
fi