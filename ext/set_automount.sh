#!/bin/sh

xfconf-query -c thunar-volman -np /automount-drives/enabled -t 'bool' -s true
xfconf-query -c thunar-volman -np /automount-media/enabled -t 'bool' -s true
xfconf-query -c thunar-volman -p /automount-drives/enabled -s true
xfconf-query -c thunar-volman -p /automount-media/enabled -s true
