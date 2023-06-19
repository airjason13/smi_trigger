#!/bin/sh

#sleep for wait vcgencmd ok
#sleep 30

function throttleCodeMask {
  perl -e "printf \"%s\", $1 & $2 ? \"$3\" : \"$4\""
}


function throttledToText {
  throttledCode=$1
  throttleCodeMask $throttledCode 0x80000 "Soft temperature limit has occurred, " ""
  throttleCodeMask $throttledCode 0x40000 "Throttling has occurred, " ""
  throttleCodeMask $throttledCode 0x20000 "Arm frequency capping has occurred, " ""
  throttleCodeMask $throttledCode 0x10000 "Under-voltage has occurred, " ""
  throttleCodeMask $throttledCode 0x8 "Soft temperature limit active, " ""
  throttleCodeMask $throttledCode 0x4 "Currently throttled, " ""
  throttleCodeMask $throttledCode 0x2 "Arm frequency capped, " ""
  throttleCodeMask $throttledCode 0x1 "Under-voltage detected, " ""
}


fps_fifo_uri="/tmp/fps_fifo"
[ -p $fps_fifo_uri ] || mkfifo $fps_fifo_uri

FILE_URI='/home/root/check_client_peripheral_devices.dat'
#echo Start to check > 
IP="$(ifconfig | grep -A 1 'eth0' | tail -1 | cut -d ':' -f 2 | cut -d ' ' -f 1)"
echo $IP
SLEEP_TIME=10

#while :
#do 
#    throttled_text=$(vcgencmd get_throttled | cut -f2 -d=)
#    echo "throttled:"$throttled_text

#done

while :
do
    IP="$(ifconfig | grep -A 1 'eth0' | tail -1 | cut -d ':' -f 2 | cut -d ' ' -f 1)"
    MSG="ip=$IP"



    #check ethernet
    ETH_LOG=$(ethtool eth0 | grep Link)
    if [[ $ETH_LOG == *"yes"* ]]; then
	echo "br0 get link"
	MSG="$MSG,eth0_status=OK"
    else
	echo "br0 link missing"
	MSG="$MSG,eth0_status=NG"
        TEMP=$(vcgencmd measure_temp | cut -c 6-7)
	CPU_CLOCK=$(vcgencmd measure_clock arm | cut -c 15-)
        echo TEMP=$TEMP at $CPU_CLOCK hz
        echo ETH Missing, TEMP=$TEMP at $CPU_CLOCK hz >> /home/root/check_client_peripheral_devices.dat
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
        echo lcd1602 Missing, TEMP=$TEMP at $CPU_CLOCK hz >> /home/root/check_client_peripheral_devices.dat
    fi
    TEMP=$(vcgencmd measure_temp | cut -c 6-7)
    MSG="$MSG,temp_status=$TEMP"
    voltage=$(vcgencmd measure_volts | cut -f2 -d= | sed 's/000//')
    echo $voltage
    throttled_text=$(vcgencmd get_throttled | cut -f2 -d=)
    echo "throttled:"$throttled_text
    MSG="$MSG,throttled=$throttled_text"
    echo $MSG
    ra_zmq_send.py $MSG $SLEEP_TIME
    SLEEP_TIME=$?
    echo SLEEP_TIME=$SLEEP_TIME
    sleep $SLEEP_TIME
done
