#!/usr/bin/python3
import os
import socket
import fcntl
import time

ra_server_info="/home/root/.ra_server_info"

if __name__ == '__main__':
    if os.path.isfile(ra_server_info):
        os.remove(ra_server_info)
    s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(('', 19999))
    while True:
        msg = s.recv(4096)
        if msg is not None:
            print("msg : ", msg)

            s_msg_list = msg.decode().split(",")
            ip=s_msg_list[0].split("=")[1]
            port=s_msg_list[1].split("=")[1]
            print("ip=", ip)
            print("port=", port)
            f = open(ra_server_info, 'w')
            f.write(ip + ":" + port)
            f.close()
