#!/bin/sh
default_con_FILE=/etc/gisled_default_connection
Hotspot_con_FILE=/etc/NetworkManager/system-connections/Hotspot.nmconnection

# MAC_ADDRESS=$(cat /sys/class/net/wlan0/address)
SN_FILE=/tmp/piusb/box_serial_number
if [[ -f $SN_FILE ]];then
	SN=$(cat $SN_FILE)
	echo "SN:"$SN
else
	mkdir -p /tmp/piusb
	mount /home/root/piusb.bin /tmp/piusb
	if [[ -f $SN_FILE ]];then
		SN=$(cat $SN_FILE)
	else
		SN="GISTLED"
	fi	
fi

# PREFIX="LED_Pi_Server"
#SSID="${PREFIX}_${MAC_ADDRESS}"
SSID=$SN
echo "SSID:"$SSID
echo $default_con_FILE
if [ -f $default_con_FILE ];then
	default_con=$(</etc/gisled_default_connection)
	echo $default_con	
fi

if [ "$default_con" == "Hotspot" ];then
	echo "setup_hotspot"
	nmcli con del Hotspot
        #rm /etc/NetworkManager/system-connections/Hotspot*.nmconnection
	nmcli con add type wifi ifname wlan0 con-name Hotspot autoconnect yes ssid $SSID
	nmcli con modify Hotspot 802-11-wireless.mode ap 802-11-wireless.band a ipv4.method shared
	nmcli con modify Hotspot wifi-sec.key-mgmt wpa-psk
	nmcli con modify Hotspot wifi-sec.psk "54098493"
	nmcli con modify Hotspot ipv4.addresses 192.168.1.7/24
	nmcli con modify Hotspot ipv4.gateway 192.168.1.7
	nmcli con up Hotspot
else
  	echo "connect to " $default_con 
	nmcli con up "$default_con"
	route del default gw GISLED
fi

exit
if [ -f "$FILE" ];then
	UUID=$(grep uuid $FILE | cut -d= -f2)
	echo UUID=$UUID
	nmcli con up uuid $UUID
else
        rm /etc/NetworkManager/system-connections/Hotspot-*.nmconnection
	nmcli con add type wifi ifname wlan0 con-name Hotspot autoconnect yes ssid $SSID
	nmcli con modify Hotspot 802-11-wireless.mode ap 802-11-wireless.band a ipv4.method shared
	nmcli con modify Hotspot wifi-sec.key-mgmt wpa-psk
	nmcli con modify Hotspot wifi-sec.psk "54098493"
	nmcli con modify Hotspot ipv4.addresses 192.168.1.7/24
	nmcli con modify Hotspot ipv4.gateway 192.168.1.7
	nmcli con up Hotspot
fi
