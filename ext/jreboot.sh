#!/bin/sh

sleep 3
pkill -f launch_led_client.sh
pkill lcsmi

brctl delif br0 enp1s0u1u1u4
brctl delif br0 eth0
ifconfig br0 down
brctl delbr br0

echo 0 > sudo tee /sys/bus/usb/devices/1-1/authorized
uhubctl -l 1-1 -a 0 -r 100
python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:USBDOWN
sleep 10
python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:REBOOT

pywatchdog test-reboot -t 5 -d /dev/watchdog

