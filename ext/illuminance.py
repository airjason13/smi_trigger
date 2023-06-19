#!/usr/bin/env python
# encoding: utf-8

import smbus
import time

#BH1750地址
__DEV_ADDR=0x23

#控制字
__CMD_PWR_OFF=0x00  #關機
__CMD_PWR_ON=0x01   #開機
__CMD_RESET=0x07    #重置
__CMD_CHRES=0x10    #持續高分辨率檢測
__CMD_CHRES2=0x11   #持續高分辨率模式2檢測
__CMD_CLHRES=0x13   #持續低分辨率檢測
__CMD_THRES=0x20    #一次高分辨率
__CMD_THRES2=0x21   #一次高分辨率模式2
__CMD_TLRES=0x23    #一次分辨率
__CMD_SEN100H=0x42  #靈敏度100%,高位
__CMD_SEN100L=0X65  #靈敏度100%，低位
__CMD_SEN50H=0x44   #50%
__CMD_SEN50L=0x6A   #50%
__CMD_SEN200H=0x41  #200%
__CMD_SEN200L=0x73  #200%

print("Init")

bus=smbus.SMBus(4)
bus.write_byte(__DEV_ADDR,__CMD_PWR_ON)
bus.write_byte(__DEV_ADDR,__CMD_RESET)
bus.write_byte(__DEV_ADDR,__CMD_SEN100H)
bus.write_byte(__DEV_ADDR,__CMD_SEN100L)
bus.write_byte(__DEV_ADDR,__CMD_PWR_OFF)
def getIlluminance():
    bus.write_byte(__DEV_ADDR,__CMD_PWR_ON)
    bus.write_byte(__DEV_ADDR,__CMD_THRES2)
    time.sleep(0.2)
    res=bus.read_word_data(__DEV_ADDR,0)
    #read_word_data
    res=((res>>8)&0xff)|(res<<8)&0xff00
    res=round(res/(2*1.2),2)
    result=str(res) + "lx"
    return result

if __name__ == '__main__':
    print("Run")
    while True:
        illum = getIlluminance()
        print("illum : %s" % illum)
        
        time.sleep(2)
