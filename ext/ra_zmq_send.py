#!/usr/bin/python3

import zmq
import sys
import os
ra_server_info="/home/root/.ra_server_info"
ip="192.168.0.2"
port=17688
if __name__ == '__main__':
    check_time = 10
    if os.path.isfile(ra_server_info):
        try:
            f=open(ra_server_info, 'r')
            info=f.read()
            ip=info.split(":")[0]
            port=info.split(":")[1]
            print("read from file, ip:", ip)
            print("read from file, port:", port)
        except:
            print("no ip&port in .ra_server_info")
    else:
        sys.exit(check_time)
    n = len(sys.argv)

    '''for i in sys.argv[1:]:
        print("i:", i)'''
    send_msg = sys.argv[1]
    check_time = sys.argv[2]
    print("check time :", check_time)
     
    context = zmq.Context()
    socket = context.socket(zmq.REQ)
    ra_server_target = "tcp://" + ip + ":" + port
    socket.connect(ra_server_target)
    socket.send(send_msg.encode(), zmq.NOBLOCK)

    p = zmq.Poller()
    p.register(socket, zmq.POLLIN)
    socks = dict(p.poll(1000))
    if socket in socks and socks[socket] == zmq.POLLIN:
        pass
    else:
        #timeout
        print("timeout!")
        print("check time :", check_time)
        sys.exit(10)
    print("ready to recv")
    message = socket.recv()
    #message = b'OK,55'
    print("recv %s" % message)
    str_message = message.decode()
    check_time = int(str_message.split(",")[1])

    sys.exit(check_time)
