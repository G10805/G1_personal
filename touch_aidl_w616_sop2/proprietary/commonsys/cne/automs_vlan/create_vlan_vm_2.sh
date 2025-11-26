#!/system/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# All rights reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

echo "$0 start." > /dev/kmsg

/system/bin/ip link add link eth0 name emac0.110 type vlan id 110
/system/bin/ip link set emac0.110 type vlan egress 0:1
/system/bin/ip link set emac0.110 up

/system/bin/ip link add link eth0 name emac0.1020 type vlan id 1020
/system/bin/ip link set emac0.1020 up

/system/bin/ip link add link eth0 name emac0.1100 type vlan id 1100
/system/bin/ip link set emac0.1100 type vlan egress 0:1
/system/bin/ip link set emac0.1100 up

/system/bin/ip link add link eth0 name emac0.1140 type vlan id 1140
/system/bin/ip link set emac0.1140 type vlan egress 0:1
/system/bin/ip link set emac0.1140 up

/system/bin/ip link add link eth0 name emac0.1150 type vlan id 1150
/system/bin/ip link set emac0.1150 type vlan egress 0:1
/system/bin/ip link set emac0.1150 up

/system/bin/ip link add link eth0 name emac0.1160 type vlan id 1160
/system/bin/ip link set emac0.1160 type vlan egress 0:1
/system/bin/ip link set emac0.1160 up

/system/bin/ip link add link eth0 name emac0.1400 type vlan id 1400
/system/bin/ip link set emac0.1400 type vlan egress 0:4
/system/bin/ip link set emac0.1400 up

/system/bin/ip link add link eth0 name emac0.1500 type vlan id 1500
/system/bin/ip link set emac0.1500 type vlan egress 0:5
/system/bin/ip link set emac0.1500 up

/system/bin/ip link add link eth0 name emac0.2110 type vlan id 2110
/system/bin/ip link set emac0.2110 type vlan egress 0:1
/system/bin/ip link set emac0.2110 up

/system/bin/ip link add link eth0 name emac0.2120 type vlan id 2120
/system/bin/ip link set emac0.2120 type vlan egress 0:1
/system/bin/ip link set emac0.2120 up

sysctl -w net.ipv4.ip_default_ttl=3
 
echo "$0 done." > /dev/kmsg

