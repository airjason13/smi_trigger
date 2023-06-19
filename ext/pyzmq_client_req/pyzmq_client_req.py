#!/usr/bin/python3

import zmq
import time
import os
import log_utils

log = log_utils.logging_init(__file__)

def kill_ledclient():
    # ledclient_pid = os.popen("pgrep ledclient").read()
    # print("ledclient_pid : %s" % ledclient_pid)
    try:
        log.debug("ready to kill ledclient")
        os.popen("pkill ledclient")
    except Exception as e:
        log.debug("pkill ledclient :%s" % e)


if __name__ == '__main__':
    error_count = 0
    server_arch = "arm64"
    log.debug("ready to pyzmq_client_req")
    ledclient_pid = os.popen("pgrep ledclient").read()
    log.debug("ledclient_pid : %s" % ledclient_pid)
    # sock = zmq.Context().socket(zmq.REQ)
    try_connect_count=0
    while True:
        log.debug("try to connect")
        try:
            context = zmq.Context()
            sock = context.socket(zmq.REQ)
            log.debug("try to connect arm64")
            sock.setsockopt(zmq.RCVTIMEO, 1000)
            sock.setsockopt(zmq.LINGER, 0)
            sock.connect("tcp://192.168.0.3:9527")
            sock.setsockopt(zmq.RCVTIMEO, 1000)
            sock.send_string("request server status", flags=zmq.NOBLOCK) 
            rep_message = sock.recv_string()
            log.debug("rep_message : %s" % rep_message)
            break
        except zmq.ZMQError as e:
            log.debug(e)
            sock.disconnect("tcp://192.168.0.3:9527")
            sock.close()
            context.term()
            
            
        time.sleep(2)
        try:
            log.debug("try to connect x86")
            context = zmq.Context()
            sock = context.socket(zmq.REQ)
            sock.setsockopt(zmq.RCVTIMEO, 1000)
            sock.setsockopt(zmq.LINGER, 1)
            sock.connect("tcp://192.168.0.2:9527")
            log.debug("A")
            sock.setsockopt(zmq.RCVTIMEO, 1000)
            log.debug("B")
            sock.send_string("request server status", flags=zmq.NOBLOCK)
            rep_message = sock.recv_string()
            log.debug("rep_message : %s" % rep_message)
            log.debug("ready for continue to request")
            break
        except zmq.ZMQError as e:
            log.debug(e) 
            sock.disconnect("tcp://192.168.0.2:9527")
            sock.close()
            context.term()

        try_connect_count += 1
        if try_connect_count > 3:
            log.debug("Cannot connect to pyzmq server. kill ledclient")
            kill_ledclient()
            try_connect_count = 0
        time.sleep(2)
        
    

    while True:
        log.debug("ready to send req")
        try:
            sock.send_string("request server status", flags=zmq.NOBLOCK)
            error_count = 0
        except Exception as e:
            log.debug("send error %s" % e)
            error_count += 1

        try:
            rep_message = sock.recv_string()
            error_count = 0
            log.debug("rep_message : %s" % rep_message)
        except Exception as e:
            log.debug("recv error %s" % e)
            error_count += 1

        if error_count > 4:
            log.debug("ready to kill ledclient")
            kill_ledclient()
        time.sleep(2)
        
    
