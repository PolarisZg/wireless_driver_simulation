# 设计思路

## 需求：

1. qemu端wifi驱动，用于qemu中操作系统与qemu虚拟设备通信

    1. 相当于面向实际设备进行开发，可能需设备树支持

2. host - qemu 边界处的虚拟设备，用于qemu与host通信

3. host端处理程序，用于虚拟设备与外界环境间交互；
    
    1. 此处处理程序暂时使用如下的设计：

    2. 与qemu端wifi驱动对应，qemu为sta，host为ap，对于host接收到的数据，使用host系统进行转发（网络层ip-port转发）

    3. 相当于面向一个虚拟的设备进行开发，甚至可能是面向一个file进行开发，如何与边界处设备进行交互暂时还没考虑

    4. qemu为ap的话如何实现暂时没想法

## 已实现

### driver - host

虽然802.11协议中规定最大的mac帧长度是2346个字节，但是对聚合mac帧的限制放的很开，802.11ax中AMPDU长度可以达到1048575个字节

使用何种方式实现host kernel - host userspace simulation device之间的通信？

根据ieee80211_alloc_hw中的规定，最少需要实现下面的这些回调函数：

```c
static const struct ieee80211_ops simuwifi_ops = {
    .start = simuwifi_start,
    .stop = simuwifi_stop,
    .tx = simuwifi_tx,
    .config = simuwifi_config,
    .add_interface = simuwifi_add_interface,
    .remove_interface = simuwifi_remove_interface,
    .configure_filter = simuwifi_configure_filter
};
```

另外还需要实现当虚拟网卡被host端kernel识别并拉起对应驱动时被自动调用的两个函数：

```c
static struct platform_driver simuwifi_dev_driver = {
    .driver = {
        .name = HOST_WIFI_NAME,
        .owner = THIS_MODULE},
    .probe = simuwifi_dev_probe,
    .remove = simuwifi_dev_remove};
```

1. `simuwifi_dev_probe`

    网卡设备被正确识别后第一个被调用的函数，需要再该方法中对platform device进行初始化。其中包括对ieee80211设备的初始化、......

    在对ieee80211设备进行初始化时，需要向kernel上报hardware的feature，具体包括以下的几个部分：

    * 硬件支持的频率范围

    * ....

### driver - qemu

### device - qemu

### device - host 

## 工具

1. [get_netlink_maxpayload.sh](./get_netlink_maxpayload.sh)

    获得当前系统最大支持的netlink数据长度，依赖gcc编译[netlink_maxpayload.c](./netlink_maxpayload.c)文件

2. [hostapd_start_real.sh](./hostapd_start_real.sh)

    将host上的某个实际的物理网卡设置为无密码的ap模式，需自行修改脚本文件中的网卡名称，可使用下述命令来查看当前系统上安装的网卡：

    ```shell
    iw dev
    ```

## to do

1. openwrt 源码编译


2. qemu 源码编译

3. qemu 虚拟设备编译

4. openwrt 驱动编译