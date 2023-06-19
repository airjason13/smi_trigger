#!/bin/sh

#sleep for wait vcgencmd ok
#sleep 30

fps_fifo_uri="/tmp/fps_fifo"
[ -p $fps_fifo_uri ] || mkfifo $fps_fifo_uri

FILE_URI='/home/gis/check_client_peripheral_devices.dat'
#echo Start to check > 
IP="$(ifconfig | grep -A 1 'br0' | tail -1 | cut -d ':' -f 2 | cut -d ' ' -f 1)"
echo $IP
SLEEP_TIME=10

while :
do
    IP="$(ifconfig | grep -A 1 'br0' | tail -1 | cut -d ':' -f 2 | cut -d ' ' -f 1)"
    MSG="ip=$IP"

    #read fps from fifo
    read -t 1 line <> $fps_fifo_uri
    if [ ${#line} == 0 ];then
	echo no fps
	MSG="$MSG,fps=NG"
    else
    	echo fps=$line
	MSG="$MSG,fps=$line"
    fi

    #check usb pico
    USB_LOG=$(lsusb -d 0x0000:0x0001)
    echo USB_LOG=$USB_LOG
    if [[ $USB_LOG == *"Pico"* ]]; then
	echo "pico alive"
	MSG="$MSG,pico_status=OK"
    else
    	echo "pico missing"
	MSG="$MSG,pico_status=NG"
        TEMP=$(vcgencmd measure_temp | cut -c 6-7)
	    CPU_CLOCK=$(vcgencmd measure_clock arm | cut -c 15-)
        echo TEMP=$TEMP at $CPU_CLOCK hz
        echo Pico Missing, TEMP=$TEMP at $CPU_CLOCK hz >> /home/gis/check_client_peripheral_devices.dat
    fi	

    #check usb terminus
    USB_LOG=$(lsusb -d 0x1a40:0x0101)
    if [[ $USB_LOG == *"Terminus"* ]]; then
	echo "eth/usb hub alive"
	MSG="$MSG,terminus_status=OK"
    else
    	echo "eth/usb hub missing"
	MSG="$MSG,terminus_status=NG"
        TEMP=$(vcgencmd measure_temp | cut -c 6-7)
	    CPU_CLOCK=$(vcgencmd measure_clock arm | cut -c 15-)
        echo TEMP=$TEMP at $CPU_CLOCK hz
        echo Terminus Missing, TEMP=$TEMP at $CPU_CLOCK hz >> /home/gis/check_client_peripheral_devices.dat
    fi	

    #check ethernet
    ETH_LOG=$(ethtool br0 | grep Link)
    if [[ $ETH_LOG == *"yes"* ]]; then
	echo "br0 get link"
	MSG="$MSG,br0_status=OK"
    else
	echo "br0 link missing"
	MSG="$MSG,br0_status=NG"
        TEMP=$(vcgencmd measure_temp | cut -c 6-7)
	    CPU_CLOCK=$(vcgencmd measure_clock arm | cut -c 15-)
        echo TEMP=$TEMP at $CPU_CLOCK hz
        echo ETH Missing, TEMP=$TEMP at $CPU_CLOCK hz >> /home/gis/check_client_peripheral_devices.dat
    fi
    #check lcd1602
    ETH_LOG=$(i2cdetect -y 1 | grep 27)
    if [[ $ETH_LOG == *"27"* ]]; then
	echo "lcd1602 alive"
	MSG="$MSG,lcd1602_status=OK"
    else
	echo "lcd1602 missing"
	MSG="$MSG,lcd1602_status=NG"
        TEMP=$(vcgencmd measure_temp | cut -c 6-7)
	CPU_CLOCK=$(vcgencmd measure_clock arm | cut -c 15-)
        echo TEMP=$TEMP at $CPU_CLOCK hz
        echo lcd1602 Missing, TEMP=$TEMP at $CPU_CLOCK hz >> /home/gis/check_client_peripheral_devices.dat
    fi
    TEMP=$(vcgencmd measure_temp | cut -c 6-7)
    MSG="$MSG,temp_status=$TEMP"

    echo $MSG
    ra_zmq_send.py $MSG $SLEEP_TIME
    SLEEP_TIME=$?
    echo SLEEP_TIME=$SLEEP_TIME
    sleep $SLEEP_TIME
done
