#!/bin/sh
IP_ARM='192.168.0.3'
IP_X86='192.168.0.2'
no_ip_count=0
no_ip_count_max=10
ledclient_res=0
python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:CONNECTING
while :
do
	# python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
	# python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:CONNECTING
	br0_ip=$(sudo /sbin/ifconfig br0 | grep 'inet addr:' | cut -d: -f2| cut -d' ' -f1)
	echo "br0 ip:"$br0_ip >> /home/root/.ledclient_network.log

	if [[ $br0_ip == 192.168.0.* ]];then
		no_ip_count=0
		# start pyzmq_client_req in background
		#pkill -f pyzmq_client_req.py

		#python3 /home/root/pyzmq_client_req/pyzmq_client_req.py &
		
		#test reboot		
		# jreboot.sh &

		echo "Abr0 ip:"$br0_ip >> /home/root/.ledclient_network.log
		sleep 1
		lcsmi -fflags nobuffer -probesize 10000 udp://239.11.11.11:15000 > /dev/null
		ledclient_res=$?			
		echo "ledclient result :"$ledclient_res
		echo "ledclient result :"$ledclient_res >> /home/root/.ledclient_network.log
		#if [ "$ledclient_res" == "0" ];then
		if ping -c 2 192.168.0.3 &> /dev/null
		then
			echo "still could ping server"
			python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
			python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SERVER_ALIVE
		else
			if ping -c 2 192.168.0.2 &> /dev/null
			then
				echo "still could ping server"
				python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
				python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SERVER_ALIVE
			else
				echo "ledclient terminated with kill signal" >> /home/root/.ledclient_network.log
				python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
				python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:IP_MISS
				set_br.sh
			fi
		fi
		sleep 1 #5 for look message
        	# if ledclient stop, let the br0 setup again to check eth hub 
		# echo "set br again!"
		# set_br.sh
	else
		no_ip_count=$((no_ip_count+1))
		python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
		python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:Wait_IP_$no_ip_count
		echo "br0 no ip obtain cuont:"$no_ip_count >> /home/root/.ledclient_network.log
		sleep 1	
	fi
	if [ "$no_ip_count" = "$no_ip_count_max" ];then
		echo "br0 no ip obtain cuont reach max:"$no_ip_count >> /home/root/.ledclient_network.log
		no_ip_count=0
		set_br.sh
		sleep 1
	fi
done	

