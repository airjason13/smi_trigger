#!/bin/sh


ALFA_WIFI_ADAPTER=""
while true ; do
    if [ -f /sys/class/net/wlp1s0u1u1/address ];then
        echo "got alfa wifi adapter"
        ALFA_WIFI_ADAPTER="wlp1s0u1u1"
        break
    elif [ -f /sys/class/net/wlp1s0u1u2/address ];then
        echo "got alfa wifi adapter"
        ALFA_WIFI_ADAPTER="wlp1s0u1u2"
        break
    elif [ -f /sys/class/net/wlp1s0u1u3/address ];then
        echo "got alfa wifi adapter"
        ALFA_WIFI_ADAPTER="wlp1s0u1u3"
        break
    elif [ -f /sys/class/net/wlp1s0u1u4/address ];then
        echo "got alfa wifi adapter"
        ALFA_WIFI_ADAPTER="wlp1s0u1u4"
        break
    else
        echo "no alfa wifi adapter"  
        sleep 5
    fi
done


sudo iptables -F
sudo iptables -X


sudo iptables -t nat -A POSTROUTING -o ${ALFA_WIFI_ADAPTER} -j MASQUERADE

sudo sysctl net.ipv4.ip_forward=1
sudo sysctl -p

