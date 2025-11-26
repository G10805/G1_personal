#!/system/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# All rights reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

echo "$0 start." > /dev/kmsg

/system/bin/ip link add link eth0 name la1emac0.110 type vlan id 110
/system/bin/ip link set la1emac0.110 type vlan egress 0:1
/system/bin/ip link set la1emac0.110 up
/system/bin/ip addr add 192.168.11.1 dev la1emac0.110

/system/bin/ip link add link eth0 name la1emac0.1100 type vlan id 1100
/system/bin/ip link set la1emac0.1100 type vlan egress 0:1
/system/bin/ip link set la1emac0.1100 up
/system/bin/ip addr add 192.168.74.1 dev la1emac0.1100

/system/bin/ip link add link eth0 name la1emac0.1400 type vlan id 1400
/system/bin/ip link set la1emac0.1400 type vlan egress 0:4
/system/bin/ip link set la1emac0.1400 up
/system/bin/ip addr add 192.168.104.1 dev la1emac0.1400

/system/bin/ip link add link eth0 name la1emac0.1500 type vlan id 1500
/system/bin/ip link set la1emac0.1500 type vlan egress 0:5
/system/bin/ip link set la1emac0.1500 up
/system/bin/ip addr add 192.168.114.1 dev la1emac0.1500

/system/bin/ip link add link eth0 name la1emac0.2110 type vlan id 2110
/system/bin/ip link set la1emac0.2110 type vlan egress 0:1
/system/bin/ip link set la1emac0.2110 up
/system/bin/ip addr add 192.168.139.1 dev la1emac0.2110

/system/bin/ip link add link eth0 name la1emac0.2120 type vlan id 2120
/system/bin/ip link set la1emac0.2120 type vlan egress 0:1
/system/bin/ip link set la1emac0.2120 up
/system/bin/ip addr add 192.168.140.1 dev la1emac0.2120

/system/bin/ip link add link eth0 name la1emac0.2550 type vlan id 2550
/system/bin/ip link set la1emac0.2550 type vlan egress 0:5
/system/bin/ip link set la1emac0.2550 up
/system/bin/ip addr add 192.168.183.1 dev la1emac0.2550

sysctl -w net.ipv4.ip_default_ttl=3

echo "$0 done." > /dev/kmsg

