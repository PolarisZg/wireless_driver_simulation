# 启动物理机中网卡的ap模式并添加一个monitor interface
set -x

rm -r -f ./hostapd.conf

sudo systemctl mask wpa_supplicant
sudo systemctl stop wpa_supplicant

sudo ip addr flush dev wlan0
sleep 0.5
sudo ifconfig wlan0 down
sleep 1
sudo ifconfig wlan0 up

WIFI_CARD_NAME=wlx08beac26814c

sudo killall hostapd

sudo ifconfig $WIFI_CARD_NAME down

CONF_FILE_NAME="hostapd.conf"

# 写入配置内容
cat << EOF > $CONF_FILE_NAME
interface=$WIFI_CARD_NAME
driver=nl80211
ssid=$WIFI_CARD_NAME
hw_mode=g
channel=1
ieee80211n=1
EOF

sudo ifconfig apmon down

sudo iw dev apmon del

sudo iw dev $WIFI_CARD_NAME interface add apmon type monitor

sudo ifconfig apmon up

sudo hostapd ./hostapd.conf

rm -r -f ./hostapd.conf

# sudo ifconfig wlan0 10.5.5.1 netmask 255.255.255.0