#!/bin/sh
CONFIG_FILE='/home/root/led_role.conf'
ROLE=''
RA_TAG='RA'
CLIENT_TAG='Client'
SERVER_TAG='Server'
AIO_TAG='AIO'
PLAYER_TAG='Player'
TESTER_TAG='Tester'
#for auto-mount test
udiskie &

export DISPLAY=:0

xrandr_add_fhd.sh

x0vncserver -SecurityTypes None &

if [ -e /home/root/server_now ];then
    sudo rm /home/root/server_now
fi

if [ -e /home/root/server_ra_now ];then
    sudo rm /home/root/server_ra_now
fi

if [ -e /home/root/client_now ];then
    rm /home/root/client_now
fi

if [ -e /home/root/aio_now ];then
    rm /home/root/aio_now
fi

if [ -e /home/root/client_ra_now ];then
    rm /home/root/client_ra_now
fi
    
if [ -e /home/root/player_now ];then
    rm /home/root/player_now
fi

if [ -e /home/root/tester_now ];then
    rm /home/root/tester_now
fi

if [ -e $CONFIG_FILE ];then
    echo "config file exist"
    while read line; do
        echo $line
	line_size=${#line}
	echo "$line_size"
	if [[ $line_size == 0 ]];then
	    echo "no line"
        else	    
            if [[ $line =~ $SERVER_TAG ]];then
                if [[ $line =~ $RA_TAG ]];then
		            echo "Server_RA check"
		            ROLE=Server_RA
                else
		            ROLE=Server
		        fi
            elif [[ $line =~ $CLIENT_TAG ]];then
	            echo "Client check"
	            if [[ $line =~ $RA_TAG ]];then
	                echo "Client_RA check"
            	    ROLE=Client_RA
	            else
		            ROLE=Client
	            fi	
            elif [[ $line =~ $AIO_TAG ]];then
                echo "AIO check"
                ROLE=AIO
            elif [[ $line =~ $PLAYER_TAG ]];then
                ROLE=Player
            elif [[ $line =~ $TESTER ]];then
	            echo "Tester check"
                ROLE=Tester
            fi     
	fi    
    done < $CONFIG_FILE
    echo "ROLE:"$ROLE
else
    echo "gen a default config file as Client"
fi     

####++++move to role actions below++++######
# insmod /home/root/rtl8812au_module/88XXau.ko

# modprobe v4l2loopback
# modprobe v4l2loopback video_nr=3,4,5,6
####----move to role actions below----######

#for monitor temperature
b_measure_temp.sh &

####++++move to role actions below++++######
#launch flask-filemnager
#launch_flask-filemanager.sh &
####----move to role actions below----######
launch_i2c_lcd_server.sh &
sleep 3
echo "ROLE:"$ROLE
if [[ $ROLE == *$CLIENT_TAG* ]];then
    echo "Let's set client env"

    python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
    python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_1

    if [[ $ROLE == *$RA_TAG* ]];then
        ra_client.py &
    	check_client_peripheral_devices.sh &
        touch /home/root/client_ra_now
    else
        touch /home/root/client_now	    
    fi
    # check dnsmasq
    if cmp -s "/lib/systemd/system/dnsmasq.service" "/home/root/dnsmasq.service.no_run"
    then
	    echo "file_identical"
    else
    	echo "file different"
	    cp /home/root/dnsmasq.service.no_run /lib/systemd/system/dnsmasq.service
        sync
        sync
        sync
        reboot
    fi
    systemctl stop dnsmasq.service    
    systemctl disable dnsmasq.service    
    echo "1. set_br"
    python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
    python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_2
    set_br.sh 
    #turn the wifi off
    nmcli radio wifi off 
    echo "2. check ip and launch ledclient"
    python3 /home/root/i2c_lcd_server/lcd_show.py 0:0:LEDCLIENT
    python3 /home/root/i2c_lcd_server/lcd_show.py 0:1:SETUP_ENV_3
    sudo launch_led_client.sh
elif [[ $ROLE == *$PLAYER_TAG* ]];then
    touch /home/root/player_now
    echo "Just a simple player"
    nmcli radio wifi on 
    setup_hotspot.sh &
    run-filemanager.sh &
    launch_led_player.sh
elif [[ $ROLE == *$AIO_TAG* ]];then
    insmod /home/root/rtl8812au_module/88XXau.ko
    #modprobe v4l2loopback
    modprobe v4l2loopback video_nr=3,4,5,6
    launch_flask-filemanager.sh &

    touch /home/root/aio_now
    echo "AIO Now"
    nmcli con add type ethernet ifname eth0 con-name eth0
    nmcli con mod eth0 ipv4.addresses 192.168.0.3/24
    nmcli con mod eth0 ipv4.gateway 192.168.0.3
    nmcli con mod eth0 ipv4.dns "8.8.8.8"
    nmcli con mod eth0 ipv4.method manual
    nmcli con up eth0
    nmcli radio wifi on 
    setup_hotspot.sh &
    setup_hotspot_alfa.sh &
    # setup_eth0_static.sh &
    # run-filemanager.sh &
    launch_pyLedServer.sh &
    write_tc358743_edid.sh & 
    launch_led_aio_client.sh &
    
elif [[ $ROLE == *$SERVER_TAG* ]];then
    echo "Server Now"
    insmod /home/root/rtl8812au_module/88XXau.ko
    #modprobe v4l2loopback
    modprobe v4l2loopback video_nr=3,4,5,6
    launch_flask-filemanager.sh &
    nmcli con del eth0
    rm /etc/NetworkManager/system-connections/*
    nmcli con add type ethernet ifname eth0 con-name eth0
    nmcli con mod eth0 ipv4.addresses 192.168.0.3/24
    nmcli con mod eth0 ipv4.gateway 192.168.0.3
    nmcli con mod eth0 ipv4.dns "8.8.8.8"
    nmcli con mod eth0 ipv4.method manual
    nmcli con up eth0
    nmcli radio wifi on 
    setup_hotspot.sh &
    setup_hotspot_alfa.sh &
    setup_eth0_static.sh &
    run-filemanager.sh &
    launch_pyLedServer.sh &
    write_tc358743_edid.sh &
    pactl set-port-latency-offset alsa_card.platform-bcm2835_audio.2 analog-output-headphones 15000
    pulseaudio -k
    pulseaudio -D
    pactl set-default-sink alsa_output.platform-bcm2835_audio.stereo-fallback.2
    if [[ $ROLE == *$RA_TAG* ]];then
        ra_client.py &
        check_server_peripheral_devices.sh &
        launch_pyLedRATool.sh &
        touch /home/root/server_ra_now
    else
        touch /home/root/server_now
    fi	    
elif [[ $ROLE == *$TESTER* ]];then
    touch /home/root/tester_now
    echo "Tester Now"
    nmcli radio wifi on 
    setup_hotspot_tester.sh &
    # run-filemanager.sh &
    launch_pyLedTester.sh 
fi  
