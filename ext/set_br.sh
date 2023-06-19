#!/bin/sh

COUNT_MAX=5
COUNT=0

python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_1


#if [ -e /sys/class/net/enp1s0u1u1 ];then
#    echo "got usbtoeth"
#elif [ -e /sys/class/net/enp1s0u1u1u4 ];then
#    echo "got usbtoeth"
#else
#    reboot
#fi 

while [ $COUNT -le $COUNT_MAX ]
do
    # The usb power on/off seq does not match microsnow eth hub,
    # only davicom udb dongle could work.
    # Sometimes the microsnow eth hub could not send the TX packet after power seq 
	if [ -e /sys/class/net/enp1s0u1u1 ];then
	    echo "power off usb hub"
	    uhubctl -l 1-1 -p 3 -a 0 -r 10
	    echo "power on usb hub A"
	    sleep 1
	    uhubctl -l 1-1 -p 3 -a 1
	    sleep 1
            sync
	    echo "power seq end"
	elif [ -e /sys/class/net/enp1s0u1u1u4 ];then
	    echo "power off usb hub"
	    echo 0 > sudo tee /sys/bus/usb/devices/1-1/authorized
	    #sleep 1
        # if only power off the hub 1-1 port 1, the power will be immediately on, 
        # so we power off the hub 1-1
	    uhubctl -l 1-1 -a 0 -r 100
	    echo "power on usb hub A"
	    sleep 1
	    uhubctl -l 1-1 -a 1
	    sleep 2
        sync
	    echo "power seq end"
        fi
	(( COUNT ++ ))
	echo "COUNT:$COUNT" >> /home/root/br_tmp.txt
	if [ -e /sys/class/net/enp1s0u1u1u4 ];then
		echo "find enp1s0u1u1u4"
		break
	elif [ -e /sys/class/net/enp1s0u1u1 ];then
		echo "find enp1s0u1u1"
		break
	else
		echo "no usb2eth" >> /home/root/br_tmp.txt
		# test @1215
	    	echo "power off usb hub"
	    	echo 0 > sudo tee /sys/bus/usb/devices/1-1/authorized
		    #sleep 1
        	# if only power off the hub 1-1 port 1, the power will be immediately on, 
        	# so we power off the hub 1-1
	    	uhubctl -l 1-1 -a 0 -r 100
	    	echo "power on usb hub A"
	   	    sleep 1
	    	uhubctl -l 1-1 -a 1
	    	sleep 2
            sync
	    	echo "power seq end"
	fi
	echo "COUNT:$COUNT" >> /home/root/br_tmp.txt
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_1_$COUNT
done


if [ -e /sys/class/net/enp1s0u1u1 ];then
    echo "got usbtoeth"
elif [ -e /sys/class/net/enp1s0u1u1u4 ];then
    echo "got usbtoeth"
else
    python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
    python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:NO_USB2ETH
    sleep 1
    sudo sync && sudo reboot -f
fi

echo "find usb2eth" >> /home/root/br_tmp.txt

python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_2

if [ -e /sys/class/net/enp1s0u1u1u4 ];then
	nmcli con del eth0
	nmcli con del enp1s0u1u1u4
	rm /etc/NetworkManager/system-connections/*
	# rm /etc/NetworkManager/system-connections/*
    	if [ -e /sys/class/net/br0 ];then
		pkill udhcpc	
        	ifconfig br0 down
        	ifconfig eth0 down
        	ifconfig enp1s0u1u1u4 down
        	brctl delbr br0
	fi
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_3
    	#ifconfig eth0 up
    	#ifconfig enp1s0u1u1u4 up
	echo "enp1s0u1u1u4 exists" >> /home/root/br_tmp.txt
	pkill dnsmasq #for sure that all client request ip from server
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_3_1
	brctl addbr br0
    	nmcli con add con-name eth0 type ethernet ifname eth0 ip4 0.0.0.0/24 gw4 0.0.0.0 ipv4.method manual
    	nmcli con add con-name enp1s0u1u1u4 type ethernet ifname enp1s0u1u1u4 ip4 0.0.0.0/24 gw4 0.0.0.0 ipv4.method manual
    	ifconfig eth0 up
    	ifconfig enp1s0u1u1u4 up
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_3_2
    	nmcli con up eth0
    	nmcli con up enp1s0u1u1u4
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_3_3
	brctl addif br0 eth0 
	brctl addif br0 enp1s0u1u1u4 
	ifconfig br0 up
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
	python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_4
	gen_mac_with_sn.py
	ifconfig eth0 0.0.0.0
	ifconfig enp1s0u1u1u4 0.0.0.0
	udhcpc -i br0 &
elif [ -e /sys/class/net/enp1s0u1u1 ];then
    if [ -e /sys/class/net/br0 ];then
        pkill udhcpc	
        ifconfig br0 down
        ifconfig eth0 down
        ifconfig enp1s0u1u1 down
        brctl delbr br0
	fi
    	ifconfig eth0 up
    	ifconfig enp1s0u1u1 up
	echo "enp1s0u1u1 exists" >> /home/root/br_tmp.txt
	pkill dnsmasq #for sure that all client request ip from server
	brctl addbr br0
    	nmcli con add con-name eth0 type ethernet ifname eth0 ip4 0.0.0.0/24 gw4 0.0.0.0 ipv4.method manual
    nmcli con add con-name enp1s0u1u1 type ethernet ifname enp1s0u1u1 ip4 0.0.0.0/24 gw4 0.0.0.0 ipv4.method manual
    	nmcli con up eth0
    	nmcli con up enp1s0u1u1
	brctl addif br0 eth0 
	brctl addif br0 enp1s0u1u1
	ifconfig br0 up
	gen_mac_with_sn.py
	udhcpc -i br0 &
else
	echo "enp1s0u1u1u4 do not exists"
	udhcpc -i eth0 &
fi	
python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2_5
