#!/bin/python3
import socket
import sys

"""
        Usage:
        python3 lcd_show.py x:y:str
"""

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

server_address = '/tmp/uds_socket_i2clcd7'

try:
    sock.connect(server_address)
    #message = '0:0:UPDATING'
    message = str(sys.argv[1])
    sock.sendall(message.encode())
except Exception as e:
    print(e)

