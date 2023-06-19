#!/bin/sh

xfconf-query -c xfce4-panel -np /panels/panel-1/position-locked -t 'bool' -s false
xfconf-query -c xfce4-panel -np /panels/panel-1/autohide-behavior -t 'int' -s 2
xfconf-query -c xfce4-panel -p /panels/panel-1/position-locked -s false
xfconf-query -c xfce4-panel -p /panels/panel-1/autohide-behavior -s 2
