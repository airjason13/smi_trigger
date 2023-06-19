#!/usr/bin/python3
# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import os
import string

# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    try:
        sn_file_uri = "/tmp/piusb/box_serial_number"
        if os.path.isfile(sn_file_uri) is True:
            sn_file = open(sn_file_uri, "r")
            lines = sn_file.readlines()
            sn_file.close()
            for line in lines:
                if "GISTLED" in line:
                    sn_str = line.strip("\n")
            mac_addr = "00:60:2f" + ":" + sn_str[8:10] + ":" + sn_str[14:16] + ":" + sn_str[17:19]
            # print("mac_addr = ", mac_addr)
            cmd = "ifconfig br0 hw ether " + mac_addr
            print(cmd)
            os.popen(cmd)

        else:
            print("set random mac")
            os.popen("ifconfig br0 hw ether 00:60:2f$(hexdump -n3 -e '/1 \":%02X\"' /dev/random)")
    except Exception as e:
        os.popen("ifconfig br0 hw ether 00:60:2f$(hexdump -n3 -e '/1 \":%02X\"' /dev/random)")

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
