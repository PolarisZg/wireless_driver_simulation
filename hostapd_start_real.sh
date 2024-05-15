set -x

rm -r -f ./hostapd.conf

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