# This is a sample Python script.
import os
import time


# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.


def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    gain_value = 0
    max_gain = 15
    mode = 0
    cmd = "./prog/smi_trigger -t anapex -b 24 -c w -l 32 "
    os.popen(cmd)
    time.sleep(0.5)
    while True:
        str_rgb_gain = str(gain_value) + ":" + str(gain_value) + ":" + str(gain_value)
        cmd = "./prog/smi_trigger -t anapex -b 24 -g " + str_rgb_gain
        print("cmd = "+ cmd)
        os.popen(cmd)
        if mode == 0:
            gain_value += 1
            if gain_value == 15:
                mode = 1
        elif mode == 1:
            gain_value -= 1
            if gain_value == 0:
                mode = 0

        time.sleep(0.5)


# See PyCharm help at https://www.jetbrains.com/help/pycharm/
