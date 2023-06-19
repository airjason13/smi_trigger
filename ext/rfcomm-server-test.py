#!/usr/bin/python2

import os
        
#get current ssid list
nmcli_search_ssid_cmd = "nmcli -t -f SSID device wifi"
nmcli_search_ssid_process = os.popen(nmcli_search_ssid_cmd)
ssid_list = nmcli_search_ssid_process.read().split("\n")
ssid = "airjason13"#tmp[0].split(":")[1]
pwd = "workout13"#tmp[1].split(":")[1]
print("ssid : %s" % ssid)
print("pwd : %s" % pwd)
b_found = False
for k in ssid_list:
    if ssid == k:
        b_found = True
if b_found is False:
    print("no such ssid")
    client_sock.send("no such ssid")
#find current con 
nmcli_get_current_con_cmd = "nmcli -t -f DEVICE,NAME c show --active"
nmcli_get_current_con_process = os.popen(nmcli_get_current_con_cmd)
result = nmcli_get_current_con_process.read()
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
elif "success" in connect_ret:
    print("connect success!")
                     


