#!/bin/sh
bluetoothctl <<EOF
power on
discoverable on
pairable on
EOF

