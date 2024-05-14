set -x
gcc -o netlink_maxpayload netlink_maxpayload.c
./netlink_maxpayload
rm -r -f ./netlink_maxpayload