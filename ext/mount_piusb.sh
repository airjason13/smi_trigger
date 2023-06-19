#!/bin/sh
fatlabel /home/root/piusb.bin gisled

mkdir -p /tmp/piusb
mount /home/root/piusb.bin /tmp/piusb
machine_type=Client
machine_type_file="/tmp/piusb/machine_type"
need_reboot='false'

if [[ -f $machine_type_file ]]; then
    echo "got file"
    line=$(cat $machine_type_file)
    echo $line
    if [[ $line == *"Client"* ]]; then
        machine_type=Client
    elif [[ $line == *"Server"* ]]; then
        machine_type=Server
    elif [[ $line == *"AIO"* ]];then
        machine_type=AIO
    else
        machine_type=Client
    fi
else
    echo no machine_type_file
fi

echo machine_type:$machine_type

led_role_file="/home/root/led_role.conf"
if [[ -f $led_role_file ]]; then
    while IFS= read -r line
    do
        line_size=${#line}
        echo line_size:$line_size
        if [[ $line_size == 0 ]]; then
            echo "no line"
        else
            if [[ $line == *"Client"* ]]; then
                led_role=Client
            elif [[ $line == *"Server"* ]]; then
                led_role=Server
            elif [[ $line == *"AIO"* ]];then
                led_role=AIO
            else
                led_role=Client
            fi
        fi 
    done < "$led_role_file"
    
fi

echo led_role:$led_role

if [[ $led_role == $machine_type ]]; then
    echo "machine type setting ok!"
else
    echo $machine_type > $led_role_file
    need_reboot='true'
fi




if [[ $need_reboot == 'true' ]]; then
    sync
    sync
    sync
    reboot
fi 
