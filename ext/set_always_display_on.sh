#!/bin/sh
xfconf-query -c xfce4-power-manager -np /xfce4-power-manager/dpms-enabled -t 'bool' -s false
xfconf-query -c xfce4-power-manager -np /xfce4-power-manager/blank-on-ac -t 'int' -s 0
xfconf-query -c xfce4-power-manager -p /xfce4-power-manager/dpms-enabled -s false
xfconf-query -c xfce4-power-manager -p /xfce4-power-manager/blank-on-ac -s 0
