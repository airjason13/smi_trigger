#!/usr/bin/env bash
echo "band:"$1
echo "channel:"$2

ACS=(36 40 44 48 52 56 60 64 100 116 132 149 165)
BGCS=(1 2 3 4 5 6 7 8 9 10 11 12 13 14)

CACS=${#ACS[@]}
CBGCS=${#BGCS[@]}

#echo "5G_CHANNELS_COUNTS:"$CACS
echo "2.4G_CHANNELS_COUNTS:"$CBGCS

BAND=$(echo $1 | tr [a-z] [A-Z])
CHECK="NG"
if [ x"$BAND" = x"A" ];then
	echo "5G operation"
	for ((i=0;i<$CACS;i++));do
		if [ x"$2" = x"${ACS[$i]}" ];then
			echo "${ACS[$i]}:"${ACS[$i]}
			CHECK="OK"
			break
		fi
	done
elif [ x"$BAND" = x"BG" ];then
	echo "2.4G operation"
	for ((i=0;i<$CBGCS;i++));do
		if [ x"$2" = x"${BGCS[$i]}" ];then
			echo "${BGCS[$i]}:"${BGCS[$i]}
			CHECK="OK"
			break
		fi
	done
else
	echo "unknown Band, ready to exit!"
fi

if [ x"$CHECK" = x"NG" ];then
	echo "EXIT!"
	exit
fi


ALFA_WIFI_ADAPTER="NONE"
if [ -f /sys/class/net/wlp1s0u1u1/address ];then
	echo "got alfa wifi adapter at wlp1s0u1u1"
	ALFA_WIFI_ADAPTER="wlp1s0u1u1"
elif [ -f /sys/class/net/wlp1s0u1u2/address ];then
	echo "got alfa wifi adapter at wlp1s0u1u2"
	ALFA_WIFI_ADAPTER="wlp1s0u1u2"
elif [ -f /sys/class/net/wlp1s0u1u3/address ];then
	echo "got alfa wifi adapter at wlp1s0u1u3"
	ALFA_WIFI_ADAPTER="wlp1s0u1u3"
elif [ -f /sys/class/net/wlp1s0u1u4/address ];then
	echo "got alfa wifi adapter at wlp1s0u1u2"
	ALFA_WIFI_ADAPTER="wlp1s0u1u2"
else 
	echo "no alfa wifi adapter"
	exit
fi


nmcli con delete Hotspot_Alfa

sleep 1

FILE=/etc/NetworkManager/system-connections/Hotspot_Alfa.nmconnection
MAC_ADDRESS=$(cat /sys/class/net/${ALFA_WIFI_ADAPTER}/address)
PREFIX="LED_Alfa"
SSID="${PREFIX}_${MAC_ADDRESS}"
if [ -f "$FILE" ];then
	echo "remove old nmconnection"
	rm $FILE
fi

#rm /etc/NetworkManager/system-connections/Hotspot_Alfa*

sync
sync

#exit

echo "SSID="$SSID
nmcli con add type wifi ifname $ALFA_WIFI_ADAPTER con-name Hotspot_Alfa autoconnect yes ssid $SSID
nmcli con modify Hotspot_Alfa 802-11-wireless.mode ap ipv4.method shared

nmcli con modify Hotspot_Alfa 802-11-wireless.band $1
nmcli con modify Hotspot_Alfa 802-11-wireless.channel $2

nmcli con modify Hotspot_Alfa wifi-sec.key-mgmt wpa-psk
nmcli con modify Hotspot_Alfa wifi-sec.psk "12345678"
nmcli con modify Hotspot_Alfa ipv4.addresses 192.168.1.9/24
nmcli con modify Hotspot_Alfa ipv4.gateway 192.168.1.9
nmcli con up Hotspot_Alfa
