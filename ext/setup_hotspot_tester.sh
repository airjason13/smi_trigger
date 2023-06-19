#!/bin/sh
FILE=/etc/NetworkManager/system-connections/Tester_Hotspot.nmconnection
MAC_ADDRESS=$(cat /sys/class/net/wlan0/address)
PREFIX="LED_Tester"
SSID="${PREFIX}_${MAC_ADDRESS}"
if [ -f "$FILE" ];then
	UUID=$(grep uuid $FILE | cut -d= -f2)
	echo UUID=$UUID
	nmcli con up uuid $UUID
else
	#nmcli con add type wifi ifname wlan0 con-name Tester_Hotspot autoconnect yes ssid LED-Pi
	nmcli con add type wifi ifname wlan0 con-name Tester_Hotspot autoconnect yes ssid $SSID
	nmcli con modify Tester_Hotspot 802-11-wireless.mode ap 802-11-wireless.band a ipv4.method shared
	nmcli con modify Tester_Hotspot wifi-sec.key-mgmt wpa-psk
	nmcli con modify Tester_Hotspot wifi-sec.psk "54098493"
	nmcli con modify Tester_Hotspot ipv4.addresses 192.168.0.7/24
	nmcli con modify Tester_Hotspot ipv4.gateway 192.168.0.7
	nmcli con up Tester_Hotspot
fi
