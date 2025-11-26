###############################################################################
#
# Copyright (c) 2024 Qualcomm Technologies, Inc.
#
# All Rights Reserved.
#
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#
###############################################################################

###############################################################################
#
# File: a2p.sh
#
# Description : AIDL-To-Proto script
#
# Version : 1.0
#
###############################################################################

#!/bin/bash
set -e

#############################################################
#
# Common Setting
#
#############################################################

android_root=$1
vendor_aidl_path=$2
# rpc util path
rpc_util_path="${android_root}/vendor/qcom/proprietary/commonsys/rpc/util"
# rpc util gen path
rpc_gen_path="${rpc_util_path}/gen"
# rpc proto gen path
rpc_proto_gen_path="${rpc_gen_path}/proto"
# A2P(AIDL-To-Proto) dir
a2p_dir="${rpc_util_path}/tool/aidl_2_proto"
# A2P bin
a2p_bin="${a2p_dir}/bin/a2p"

echo_info()
{
    echo "$1" > /dev/null 2>&1
}

exit_error()
{
    echo_info "error: $1"
    exit 1
}

gen_proto()
{
    aidl_path=$1
    proto_path=$2
    gen=$3
    package_name=$4

    if [ "$gen" = "true" ]; then
        if [ ! -d ${aidl_path} ]; then
            echo_info "not exist aidl_path: ${aidl_path}"
            return
        fi

        if [ ! -d ${proto_path} ]; then
            mkdir -p ${proto_path}
        fi
        echo_info "aidl_path:  ${aidl_path}"
        echo_info "proto_path: ${proto_path}"
        ${a2p_bin} "--aidl-path" ${aidl_path} "--proto-path" ${proto_path} "--package-name" ${package_name} > /dev/null 2>&1
    else
        echo_info "not gen proto for aidl in $aidl_path"
    fi
}

if [ ! -e ${a2p_bin} ]; then
    exit_error "not exist a2p ${a2p_bin}"
fi

#############################################################
#
# Bluetooth
#
#############################################################

gen_bluetooth=true
bluetooth_proto_gen_root="${rpc_proto_gen_path}/bluetooth"

# bluetooth
gen_bluetooth_proto()
{
    bluetooth_aidl_path="${android_root}/hardware/interfaces/bluetooth/aidl/android/hardware/bluetooth"
    bluetooth_proto_path="${bluetooth_proto_gen_root}/bluetooth"
    gen_proto ${bluetooth_aidl_path} ${bluetooth_proto_path} ${gen_bluetooth}
}

# bluetooth handsfree_audio
gen_bluetooth_handsfree_audio_proto()
{
    bluetooth_handsfree_audio_aidl_path="${android_root}/vendor/qcom/proprietary/interfaces/bluetooth/handsfree_audio/aidl/vendor/qti/hardware/bluetooth/handsfree_audio"
    bluetooth_handsfree_audio_proto_path="${bluetooth_proto_gen_root}/handsfree_audio"
    gen_proto ${bluetooth_handsfree_audio_aidl_path} ${bluetooth_handsfree_audio_proto_path} ${gen_bluetooth}
}

# bluetooth
gen_bluetooth_code()
{
    # bluetooth
    gen_bluetooth_proto

    # bluetooth handsfree_audio
    gen_bluetooth_handsfree_audio_proto
}

#############################################################
#
# Wifi
#
#############################################################

gen_wifi=true
wifi_proto_gen_root="${rpc_proto_gen_path}/wifi"

# wifi
gen_wifi_proto()
{
    wifi_aidl_path="${android_root}/hardware/interfaces/wifi/aidl/android/hardware/wifi"
    wifi_proto_path="${wifi_proto_gen_root}/wifi"
    gen_proto ${wifi_aidl_path} ${wifi_proto_path} ${gen_wifi}
}

# supplicant
gen_supplicant_proto()
{
    supplicant_aidl_path="${android_root}/hardware/interfaces/wifi/supplicant/aidl/android/hardware/wifi/supplicant"
    supplicant_proto_path="${wifi_proto_gen_root}/supplicant"
    gen_proto ${supplicant_aidl_path} ${supplicant_proto_path} ${gen_wifi}
}

# nlinterceptor
gen_nlinterceptor_proto()
{
    nlinterceptor_aidl_path="${android_root}/hardware/interfaces/wifi/netlinkinterceptor/aidl/android/hardware/net/nlinterceptor"
    nlinterceptor_proto_path="${wifi_proto_gen_root}/nlinterceptor"
    gen_proto ${nlinterceptor_aidl_path} ${nlinterceptor_proto_path} ${gen_wifi}
}

# vendor supplicant
gen_vendor_supplicant_proto()
{
    vendor_supplicant_aidl_path="${android_root}/${vendor_aidl_path}/wifi/supplicant/aidl/vendor/qti/hardware/wifi/supplicant"
    vendor_supplicant_proto_path="${wifi_proto_gen_root}/vendor_supplicant"
    gen_proto ${vendor_supplicant_aidl_path} ${vendor_supplicant_proto_path} ${gen_wifi}
}

# hostapd
gen_hostapd_proto()
{
    hostapd_aidl_path="${android_root}/hardware/interfaces/wifi/hostapd/aidl/android/hardware/wifi/hostapd"
    hostapd_proto_path="${wifi_proto_gen_root}/hostapd"
    gen_proto ${hostapd_aidl_path} ${hostapd_proto_path} ${gen_wifi}
}

# vendor hostapd
gen_vendor_hostapd_proto()
{
    vendor_hostapd_aidl_path="${android_root}/${vendor_aidl_path}/wifi/hostapd/aidl/vendor/qti/hardware/wifi/hostapd"
    vendor_hostapd_proto_path="${wifi_proto_gen_root}/vendor_hostapd"
    gen_proto ${vendor_hostapd_aidl_path} ${vendor_hostapd_proto_path} ${gen_wifi}
}

# qtiwifi
gen_qtiwifi_proto()
{
    qtiwifi_aidl_path="${android_root}/vendor/qcom/proprietary/interfaces/wifi/qtiwifi/aidl/vendor/qti/hardware/wifi/qtiwifi"
    qtiwifi_proto_path="${wifi_proto_gen_root}/qtiwifi"
    qtiwifi_package_name="qti-wifi-vendor"
    gen_proto ${qtiwifi_aidl_path} ${qtiwifi_proto_path} ${gen_wifi} ${qtiwifi_package_name}
}

# wifi
gen_wifi_code()
{
    # wifi
    gen_wifi_proto

    # supplicant
    gen_supplicant_proto

    # nlinterceptor
    gen_nlinterceptor_proto

    # vendor supplicant
    gen_vendor_supplicant_proto

    # hostapd
    gen_hostapd_proto

    # vendor hostapd
    gen_vendor_hostapd_proto

    # qtiwifi
    gen_qtiwifi_proto
}

#############################################################
#
# Main
#
#############################################################

chmod +x ${a2p_bin}

# wifi
gen_wifi_code

# bluetooth
gen_bluetooth_code

echo_info "a2p done"
