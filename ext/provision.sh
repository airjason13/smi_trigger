#!/bin/sh
PROVISION_FILE=/home/root/.provision

HOSTNAME_PREFIX=GIS_LED
GOT_ETH_HUB=FALSE
GOT_PICO=FLASE
MACHINE_TYPE_FILE="/tmp/piusb/machine_type"

if [ -f "$PROVISION_FILE" ];then
   echo "provisioned" 
else
    fdisk-mmcblk0p2.sh
    while read line; do
        if [[ $line == *"raspberry4"* ]];then
            sed  -i "s|raspberrypi4-64|${HOSTNAME_PREFIX}|"   /etc/hostname  
        fi
    done < /etc/hostname  
    generate_machine_info.sh 
    # hw check machine type
    lsusb | grep 1a40:0101 > /dev/null
    if [ $? -eq 0 ]; then
	GOT_ETH_HUB=TRUE
    fi
    lsusb | grep 0000:0001 > /dev/null
    if [ $? -eq 0 ]; then
	GOT_PICO=TRUE
    fi
    echo "GOT_PICO:"$GOT_PICO
    echo "GOT_ETH_HUB:"$GOT_ETH_HUB
    if [ $GOT_PICO == "TRUE" ]; then
	if [ $GOT_ETH_HUB == "TRUE" ]; then
	    echo "Client" > $MACHINE_TYPE_FILE
	else
	    echo "AIO" > $MACHINE_TYPE_FILE
	fi 
    else
        echo "Server" > $MACHINE_TYPE_FILE
    fi	
    timedatectl set-timezone Asia/Taipei #set Taipei timezone

    touch ${PROVISION_FILE}
    sync
    sync
    sync
    reboot
fi

