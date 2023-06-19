#!/bin/sh
FILE=/etc/NetworkManager/system-connections/eth0-static.nmconnection
if [ -f "$FILE" ];then
	UUID=$(grep uuid $FILE | cut -d= -f2)
	echo UUID=$UUID
	nmcli con up uuid $UUID
else
    nmcli con add con-name eth0-static ifname eth0 type Ethernet
    nmcli con mod eth0-static ipv4.address "192.168.0.3"
    nmcli con mod eth0-static ipv4.gateway "192.168.0.3"
    nmcli con mod eth0-static ipv4.dns "192.168.0.3 8.8.8.8"
    nmcli con mod eth0-static ipv4.method manual
    nmcli con mod eth0-static ipv4.connection.autoconnect yes
	nmcli con up eth0-static
fi
