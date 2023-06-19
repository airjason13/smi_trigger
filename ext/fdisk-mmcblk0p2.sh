#!/bin/sh
echo "p
d
2
n
p
2
155648

w" | fdisk /dev/mmcblk0

resize2fs /dev/mmcblk0p2
