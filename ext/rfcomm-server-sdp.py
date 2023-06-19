#!/usr/bin/python2

import bluetooth
import os
import time
server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )

port = 0 
#bluetooth.get_available_port( bluetooth.RFCOMM )
server_sock.bind(("",port))
server_sock.listen(1)
print ("listening on port %d" % port)

#uuid = "1e0ca4ea-299d-4335-93eb-27fcfe7fa848"
uuid = "00000001-0000-1000-8000-00805F9B34FB"
bluetooth.advertise_service( server_sock, "BT SD Service", uuid )

client_sock,address = server_sock.accept()
print ("Accepted connection from ",address)

#if current con is Hotspot, disconnect
nmcli_get_current_con_cmd = "nmcli -t -f DEVICE,NAME c show --active"
nmcli_get_current_con_process = os.popen(nmcli_get_current_con_cmd)
result = nmcli_get_current_con_process.read()
nmcli_get_current_con_process.close()
con_list = result.split("\n")

current_con = None
for i in con_list:
    if "wlan0" in i:
        current_con = i.split(":")[1]
if current_con is not None:
    if "Hotspot" in current_con:
        nmcli_disconnect_cmd = "nmcli con down id " + current_con
        nmcli_disconnect_process = os.popen(nmcli_disconnect_cmd)
        print("res: %s" % nmcli_disconnect_process.read())
        nmcli_disconnect_process.close()
        # time for search ssid
        time.sleep(2)
while True:
    data = client_sock.recv(1024)
    print ("received [%s]" % data)
    if "SSID" in data:

        try:
            #get current ssid list
            nmcli_search_ssid_cmd = "nmcli -t -f SSID device wifi"
            nmcli_search_ssid_process = os.popen(nmcli_search_ssid_cmd)
            ssid_list = nmcli_search_ssid_process.read().split("\n")
            tmp = data.split(",")
            ssid = tmp[0].split(":")[1]
            pwd = tmp[1].split(":")[1]
            print("ssid : %s" % ssid)
            print("pwd : %s" % pwd)

            b_found = False
            for k in ssid_list:
                if ssid == k:
                    b_found = True
            if b_found is False:
                print("no such ssid")
                client_sock.send("Failed!No such ssid\n")
                continue
            #find current con
            nmcli_get_current_con_cmd = "nmcli -t -f DEVICE,NAME c show --active"
            nmcli_get_current_con_process = os.popen(nmcli_get_current_con_cmd)
            result = nmcli_get_current_con_process.read()
            nmcli_get_current_con_process.close()
            con_list = result.split("\n")
            current_con = None
            for i in con_list:
                if "wlan0" in i:
                    current_con = i.split(":")[1]
            #disconnect current con first
            if current_con is not None:
                nmcli_disconnect_cmd = "nmcli con down id " + current_con
                nmcli_disconnect_process = os.popen(nmcli_disconnect_cmd)
                print("res: %s" % nmcli_disconnect_process.read())
                nmcli_disconnect_process.close()
            #connect new connection
            nmcli_connect_cmd = "nmcli dev wifi connect " + ssid + " password " + pwd
            nmcli_connect_process = os.popen(nmcli_connect_cmd)
            connect_ret = nmcli_connect_process.read()
            nmcli_connect_process.close()
            if "failed" in connect_ret:
                print("connect failed!")
                nmcli_cmd = "nmcli con up id Hotspot"
                nmcli_process = os.popen(nmcli_cmd)
                print("res: %s" % nmcli_process.read())
                nmcli_process.close()
                client_sock.send("Failed!Connection Failed!\n")
            elif "success" in connect_ret:
                print("connect success!")
                client_sock.send("Success!Connection successfully!\n")


        except Exception as e:
            print(e)
            #if current con is Hotspot, disconnect
            nmcli_get_current_con_cmd = "nmcli -t -f DEVICE,NAME c show --active"
            nmcli_get_current_con_process = os.popen(nmcli_get_current_con_cmd)
            result = nmcli_get_current_con_process.read()
            nmcli_get_current_con_process.close()
            con_list = result.split("\n")

            current_con = None
            for i in con_list:
                if "wlan0" in i:
                    current_con = i.split(":")[1]
            if current_con is None:
                nmcli_cmd = "nmcli con up id Hotspot"
                nmcli_process = os.popen(nmcli_cmd)
                print("res: %s" % nmcli_process.read())
            nmcli_process.close()

            client_sock.send("Failed!Connection Failed!\n")
    elif "SetDefault" in data:
        try:
            default_connection = data.split(":")[1]

            print("default_connection = %s" % default_connection)

            f = open("/etc/gisled_default_connection", "w")
            f.write(default_connection)
            f.close()
            client_sock.send("Success!Set default Connection Success!\n")
        except Exception as e:
            print(e)
            client_sock.send("Failed!Set default Connection Failed!\n")


#if current con is Hotspot, disconnect
nmcli_get_current_con_cmd = "nmcli -t -f DEVICE,NAME c show --active"
nmcli_get_current_con_process = os.popen(nmcli_get_current_con_cmd)
result = nmcli_get_current_con_process.read()
nmcli_get_current_con_process.close()
con_list = result.split("\n")

current_con = None
for i in con_list:
    if "wlan0" in i:
        current_con = i.split(":")[1]
if current_con is None:
    nmcli_cmd = "nmcli con up id Hotspot"
    nmcli_process = os.popen(nmcli_cmd)
    print("res: %s" % nmcli_process.read())
nmcli_process.close()
    
client_sock.close()
server_sock.close()
