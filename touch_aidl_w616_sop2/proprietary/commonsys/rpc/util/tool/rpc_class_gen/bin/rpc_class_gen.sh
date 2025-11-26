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
# File: rpc_class_gen.sh
#
# Description : rpc class generator script
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
# rpc util path
rpc_util_path="${android_root}/vendor/qcom/proprietary/commonsys/rpc/util"
# rpc util gen path
rpc_gen_path="${rpc_util_path}/gen"
# rpc proto gen path
rpc_proto_gen_path="${rpc_gen_path}/proto"
# rpc source gen path
rpc_source_gen_path="${rpc_gen_path}/source"
# RPC class generator dir
rpc_class_gen_dir="${rpc_util_path}/tool/rpc_class_gen"
# rpc class generator bin
rpc_class_gen_bin="${rpc_class_gen_dir}/bin/rpc_class_gen"
# true: aidl sync api, false: aidl async api
aidl_sync_api=true
# true: add validate func in hal rpc source, false: not
validate_func=false

echo_info()
{
    echo "$1" > /dev/null 2>&1
}

exit_error()
{
    echo_info "error: $1"
    exit 1
}

gen_rpc_class()
{
    aidl_path=$1
    proto_path=$2
    gen_path=$3
    gen=$4
    ndk_lib=$5
    sync_api=$6
    validate=$7

    if [ "$gen" = "true" ]; then
        if [ ! -d ${aidl_path} ]; then
            echo_info "not exist aidl_path: ${aidl_path}"
            return
        fi

        if [ ! -d ${proto_path} ]; then
            echo_info "not exist proto: ${proto_path}"
            return
        fi

        if [ ! -d ${gen_path} ]; then
            mkdir -p ${gen_path}
        fi
        echo_info "aidl_path:  ${aidl_path}"
        echo_info "proto_path: ${proto_path}"
        echo_info "gen_path:   ${gen_path}"
        ${rpc_class_gen_bin} "--aidl-path" ${aidl_path} "--proto-path" ${proto_path} "--gen-path" ${gen_path} "--ndk-lib" ${ndk_lib} "--aidl-sync-api" ${sync_api} \
            "--validate-func" ${validate} "--server" false "--hal-impl-stub" true "--remove-intf" true "--single-someip" true > /dev/null 2>&1
    else
        echo_info "not gen rpc class in ${gen_path}"
    fi
}

if [ ! -e ${rpc_class_gen_bin} ]; then
    exit_error "not exist rpc_class_gen ${rpc_class_gen_bin}"
fi

#############################################################
#
# Bluetooth
#
#############################################################

gen_bluetooth=true
bluetooth_proto_gen_root="${rpc_proto_gen_path}/bluetooth"
bluetooth_source_gen_root="${rpc_source_gen_path}/bluetooth"

# bluetooth
gen_bluetooth_rpc_class()
{
    bluetooth_aidl_path="${android_root}/hardware/interfaces/bluetooth/aidl/android/hardware/bluetooth"
    bluetooth_proto_path="${bluetooth_proto_gen_root}/bluetooth"
    bluetooth_gen_path="${bluetooth_source_gen_root}/bluetooth"
    bluetooth_ndk_lib=android.hardware.bluetooth-V1-ndk
    bluetooth_aidl_sync_api=false
    gen_rpc_class ${bluetooth_aidl_path} ${bluetooth_proto_path} ${bluetooth_gen_path} ${gen_bluetooth} ${bluetooth_ndk_lib} ${bluetooth_aidl_sync_api} ${validate_func}
}

# bluetooth handsfree_audio
gen_bluetooth_handsfree_audio_rpc_class()
{
    bluetooth_handsfree_audio_aidl_path="${android_root}/vendor/qcom/proprietary/interfaces/bluetooth/handsfree_audio/aidl/vendor/qti/hardware/bluetooth/handsfree_audio"
    bluetooth_handsfree_audio_proto_path="${bluetooth_proto_gen_root}/handsfree_audio"
    bluetooth_handsfree_audio_gen_path="${bluetooth_source_gen_root}/handsfree_audio"
    bluetooth_handsfree_audio_ndk_lib=vendor.qti.hardware.bluetooth.handsfree_audio-V1-ndk
    bluetooth_handsfree_audio_aidl_sync_api=false
    gen_rpc_class ${bluetooth_handsfree_audio_aidl_path} ${bluetooth_handsfree_audio_proto_path} ${bluetooth_handsfree_audio_gen_path} \
    ${gen_bluetooth} ${bluetooth_handsfree_audio_ndk_lib} ${bluetooth_handsfree_audio_aidl_sync_api} ${validate_func}
}

# bluetooth
gen_bluetooth_code()
{
    # bluetooth
    gen_bluetooth_rpc_class

    # bluetooth handsfree_audio
    gen_bluetooth_handsfree_audio_rpc_class
}

#############################################################
#
# Wifi
#
#############################################################

gen_wifi=false
wifi_proto_gen_root="${rpc_proto_gen_path}/wifi"
wifi_source_gen_root="${rpc_source_gen_path}/wifi"

# wifi
gen_wifi_rpc_class()
{
    wifi_aidl_path="${android_root}/hardware/interfaces/wifi/aidl/android/hardware/wifi"
    wifi_proto_path="${wifi_proto_gen_root}/wifi"
    wifi_gen_path="${wifi_source_gen_root}/wifi"
    wifi_ndk_lib=android.hardware.wifi-V1-ndk
    wifi_validate_func=false
    gen_rpc_class ${wifi_aidl_path} ${wifi_proto_path} ${wifi_gen_path} ${gen_wifi} ${wifi_ndk_lib} ${aidl_sync_api} ${wifi_validate_func}
}

# supplicant
gen_supplicant_rpc_class()
{
    supplicant_aidl_path="${android_root}/hardware/interfaces/wifi/supplicant/aidl/android/hardware/wifi/supplicant"
    supplicant_proto_path="${wifi_proto_gen_root}/supplicant"
    supplicant_gen_path="${wifi_source_gen_root}/supplicant"
    supplicant_ndk_lib=android.hardware.wifi.supplicant-V2-ndk
    supplicant_validate_func=false
    gen_rpc_class ${supplicant_aidl_path} ${supplicant_proto_path} ${supplicant_gen_path} ${gen_wifi} ${supplicant_ndk_lib} ${aidl_sync_api} ${supplicant_validate_func}
}

# nlinterceptor
gen_nlinterceptor_rpc_class()
{
    nlinterceptor_aidl_path="${android_root}/hardware/interfaces/wifi/netlinkinterceptor/aidl/android/hardware/net/nlinterceptor"
    nlinterceptor_proto_path="${wifi_proto_gen_root}/nlinterceptor"
    nlinterceptor_gen_path="${wifi_source_gen_root}/nlinterceptor"
    nlinterceptor_ndk_lib=android.hardware.net.nlinterceptor-V1-ndk
    nlinterceptor_validate_func=false
    gen_rpc_class ${nlinterceptor_aidl_path} ${nlinterceptor_proto_path} ${nlinterceptor_gen_path} ${gen_wifi} ${nlinterceptor_ndk_lib} ${aidl_sync_api} ${nlinterceptor_validate_func}
}

# wifi
gen_wifi_code()
{
    # wifi
    gen_wifi_rpc_class

    # supplicant
    gen_supplicant_rpc_class

    # nlinterceptor
    gen_nlinterceptor_rpc_class
}

#############################################################
#
# Main
#
#############################################################

chmod +x ${rpc_class_gen_bin}

# wifi
gen_wifi_code

# bluetooth
gen_bluetooth_code

echo_info "rpc class gen done"
