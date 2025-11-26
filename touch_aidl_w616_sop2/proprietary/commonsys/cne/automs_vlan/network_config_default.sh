#!/system/bin/sh
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# All rights reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

echo "$0 start." > /dev/kmsg

/system/bin/ip link add link eth0 name eth0.4 type vlan id 4
/system/bin/ip link set eth0.4 up
/system/bin/ip addr add 192.168.4.3/24 broadcast 192.168.4.255 dev eth0.4

sysctl -w net.core.rmem_max=33554432
sysctl -w net.core.wmem_max=262144
sysctl -w net.ipv4.udp_mem="786432 1048576 16777216"
sysctl -w net.ipv4.tcp_mem="8388608 1048576 16777216"

sysctl -w net.core.rmem_default=16777216
sysctl -w net.core.wmem_default=16777216
sysctl -w net.core.optmem_max=25165824
sysctl -w net.ipv4.tcp_rmem="8192 87380 16777216"
sysctl -w net.ipv4.tcp_wmem="8192 87380 16777216"

sysctl -w net.core.somaxconn=10000
sysctl -w net.core.netdev_max_backlog=10000

sysctl -w net.ipv4.ipfrag_high_thresh=50000000
sysctl -w net.ipv6.ip6frag_high_thresh=50000000
sysctl -w net.ipv4.ipfrag_time=2
sysctl -w net.ipv6.ip6frag_time=2

echo "$0 done." > /dev/kmsg

