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
# File: apcc.sh
#
# Description : APCC(AIDL/Proto Class Conversion) script
#
# Version : 2.0
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
# rpc source gen path
rpc_source_gen_path="${rpc_gen_path}/source"
# APCC dir
apcc_dir="${rpc_util_path}/tool/apcc"
# APCC bin
apcc_bin="${apcc_dir}/apcc.py"
# Mandatory arguments for Android
argument_switch="-impl -id -bp -testcode --ndk_name"

echo_info()
{
    echo "$1" > /dev/null 2>&1
}

exit_error()
{
    echo_info "error: $1"
    exit 1
}

gen_apcc()
{
    echo_info "number of arguments: $#"
    echo_info "arguments: $* "
    local aidl_path
    local proto_path
    local gen_path
    local gen
    local ndk_name
    local base_intf
    local asyncc
    local module_name
    local msg_offset

    local OPTIND
    while getopts a:p:o:g:n:b:s:m:f: opt
    do
        case "${opt}" in
            a)
                aidl_path=${OPTARG}
                echo_info "aidl_path: ${aidl_path}"
                ;;
            p)
                proto_path=${OPTARG}
                echo_info "proto_path: ${proto_path}"
                ;;
            o)
                gen_path=${OPTARG}
                echo_info "gen_path: ${gen_path}"
                ;;
            g)
                gen=${OPTARG}
                echo_info "gen: ${gen}"
                ;;
            n)
                ndk_name=${OPTARG}
                echo_info "ndk_name: ${ndk_name}"
                ;;
            b)
                base_intf=${OPTARG}
                echo_info "base_inf: ${base_intf}"
                ;;
            s)
                asyncc=${OPTARG}
                echo_info "asyncc: ${OPTARG}"
                ;;
            m)
                module_name=${OPTARG}
                echo_info "package_name: ${module_name}"
                ;;
            f)
                msg_offset=${OPTARG}
                echo_info "msg_def_offset: ${msg_offset}"
                ;;
            \?)
                echo_info "unknow param ${OPTARG}"
                ;;
        esac
    done
    shift "$(( OPTIND - 1 ))"

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

        cmd="${apcc_bin} --aidl_path ${aidl_path} --proto_path ${proto_path} --out_path ${gen_path} ${argument_switch} ${ndk_name} --base_intf ${base_intf}"

        if [ -n "${asyncc}" ]; then
            cmd="${cmd} -async"
        fi

        if [ -n "${module_name}" ]; then
            cmd="${cmd} --package_name ${module_name}"
        fi

        if [ -n "${msg_offset}" ]; then
            cmd="${cmd} --msg_def_offset ${msg_offset}"
        fi

        ${cmd} > /dev/null 2>&1

    else
        echo_info "not gen apcc in ${gen_path}"
    fi
}

if [ ! -e ${apcc_bin} ]; then
    exit_error "not exist apcc ${apcc_bin}"
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
gen_bluetooth_apcc()
{
    bluetooth_aidl_path="${android_root}/hardware/interfaces/bluetooth/aidl/android/hardware/bluetooth"
    bluetooth_proto_path="${bluetooth_proto_gen_root}/bluetooth"
    bluetooth_gen_path="${bluetooth_source_gen_root}/bluetooth"
    bluetooth_ndk_lib=android.hardware.bluetooth-V1-ndk
    bluetooth_base_intf=IBluetoothHci
    bluetooth_async="-async"
    gen_apcc -a ${bluetooth_aidl_path} -p ${bluetooth_proto_path} -o ${bluetooth_gen_path} -g ${gen_bluetooth} -n ${bluetooth_ndk_lib}\
    -b ${bluetooth_base_intf} -s ${bluetooth_async}
}

# bluetooth handsfree_audio
gen_bluetooth_handsfree_audio_apcc()
{
    bluetooth_handsfree_audio_aidl_path="${android_root}/vendor/qcom/proprietary/interfaces/bluetooth/handsfree_audio/aidl/vendor/qti/hardware/bluetooth/handsfree_audio"
    bluetooth_handsfree_audio_proto_path="${bluetooth_proto_gen_root}/handsfree_audio"
    bluetooth_handsfree_audio_gen_path="${bluetooth_source_gen_root}/handsfree_audio"
    bluetooth_handsfree_audio_ndk_lib=vendor.qti.hardware.bluetooth.handsfree_audio-V1-ndk
    bluetooth_handsfree_audio_base_intf=IBluetoothHandsfreeAudio
    bluetooth_handsfree_audio_module_name=bluetooth-handsfree-audio
    bluetooth_handsfree_audio_async="-async"
    gen_apcc -a ${bluetooth_handsfree_audio_aidl_path} -p ${bluetooth_handsfree_audio_proto_path} -o ${bluetooth_handsfree_audio_gen_path} -g ${gen_bluetooth}\
     -n ${bluetooth_handsfree_audio_ndk_lib} -b ${bluetooth_handsfree_audio_base_intf} -m ${bluetooth_handsfree_audio_module_name} -s ${bluetooth_handsfree_audio_async}
}

# bluetooth
gen_bluetooth_code()
{
    # bluetooth
    gen_bluetooth_apcc

    # bluetooth handsfree_audio
    gen_bluetooth_handsfree_audio_apcc
}

#############################################################
#
# Wifi
#
#############################################################

gen_wifi=true
wifi_proto_gen_root="${rpc_proto_gen_path}/wifi"
wifi_source_gen_root="${rpc_source_gen_path}/wifi"

# wifi
gen_wifi_apcc()
{
    wifi_aidl_path="${android_root}/hardware/interfaces/wifi/aidl/android/hardware/wifi"
    wifi_proto_path="${wifi_proto_gen_root}/wifi"
    wifi_gen_path="${wifi_source_gen_root}/wifi"
    wifi_ndk_lib=android.hardware.wifi-V1-ndk
    wifi_base_intf=IWifi
    gen_apcc -a ${wifi_aidl_path} -p ${wifi_proto_path} -o ${wifi_gen_path} -g ${gen_wifi} -n ${wifi_ndk_lib} -b ${wifi_base_intf}
}

# supplicant
gen_supplicant_apcc()
{
    supplicant_aidl_path="${android_root}/hardware/interfaces/wifi/supplicant/aidl/android/hardware/wifi/supplicant"
    supplicant_proto_path="${wifi_proto_gen_root}/supplicant"
    supplicant_gen_path="${wifi_source_gen_root}/supplicant"
    supplicant_ndk_lib=android.hardware.wifi.supplicant-V2-ndk
    supplicant_base_intf=ISupplicant
    gen_apcc -a ${supplicant_aidl_path} -p ${supplicant_proto_path} -o ${supplicant_gen_path} -g ${gen_wifi} -n ${supplicant_ndk_lib} -b ${supplicant_base_intf}
}

# netlinkinterceptor
gen_nlinterceptor_apcc()
{
    nlinterceptor_aidl_path="${android_root}/hardware/interfaces/wifi/netlinkinterceptor/aidl/android/hardware/net/nlinterceptor"
    nlinterceptor_proto_path="${wifi_proto_gen_root}/nlinterceptor"
    nlinterceptor_gen_path="${wifi_source_gen_root}/nlinterceptor"
    nlinterceptor_ndk_lib=android.hardware.net.nlinterceptor-V1-ndk
    nlinterceptor_base_intf=IInterceptor
    gen_apcc -a ${nlinterceptor_aidl_path} -p ${nlinterceptor_proto_path} -o ${nlinterceptor_gen_path} -g ${gen_wifi} -n ${nlinterceptor_ndk_lib}\
    -b ${nlinterceptor_base_intf}
}

# vendor supplicant
gen_vendor_supplicant_apcc()
{
    vendor_supplicant_aidl_path="${android_root}/${vendor_aidl_path}/wifi/supplicant/aidl/vendor/qti/hardware/wifi/supplicant"
    vendor_supplicant_proto_path="${wifi_proto_gen_root}/vendor_supplicant"
    vendor_supplicant_gen_path="${wifi_source_gen_root}/vendor_supplicant"
    vendor_supplicant_ndk_lib_name=vendor.qti.hardware.wifi.supplicant-V2-ndk
    vendor_supplicant_base_intf=ISupplicantVendor
    vendor_supplicant_module_name=supplicant-vendor
    vendor_supplicant_msg_offset="0x1000"
    gen_apcc -a ${vendor_supplicant_aidl_path} -p ${vendor_supplicant_proto_path} -o ${vendor_supplicant_gen_path} -g ${gen_wifi} -n ${vendor_supplicant_ndk_lib_name}\
    -b ${vendor_supplicant_base_intf} -m ${vendor_supplicant_module_name} -f ${vendor_supplicant_msg_offset}
}

# hostapd
gen_hostapd_apcc()
{
    hostapd_aidl_path="${android_root}/hardware/interfaces/wifi/hostapd/aidl/android/hardware/wifi/hostapd"
    hostapd_proto_path="${wifi_proto_gen_root}/hostapd"
    hostapd_gen_path="${wifi_source_gen_root}/hostapd"
    hostapd_ndk_lib=android.hardware.wifi.hostapd-V1-ndk
    hostapd_base_intf=IHostapd
    gen_apcc -a ${hostapd_aidl_path} -p ${hostapd_proto_path} -o ${hostapd_gen_path} -g ${gen_wifi} -n ${hostapd_ndk_lib} -b ${hostapd_base_intf}
}

# vendor hostapd
gen_vendor_hostapd_apcc()
{
    vendor_hostapd_aidl_path="${android_root}/${vendor_aidl_path}/wifi/hostapd/aidl/vendor/qti/hardware/wifi/hostapd"
    vendor_hostapd_proto_path="${wifi_proto_gen_root}/vendor_hostapd"
    vendor_hostapd_gen_path="${wifi_source_gen_root}/vendor_hostapd"
    vendor_hostapd_ndk_lib_name=vendor.qti.hardware.wifi.hostapd-V1-ndk
    vendor_hostapd_base_intf=IHostapdVendor
    vendor_hostapd_module_name=hostapd-vendor
    vendor_hostapd_msg_offset="0x1000"
    gen_apcc -a ${vendor_hostapd_aidl_path} -p ${vendor_hostapd_proto_path} -o ${vendor_hostapd_gen_path} -g ${gen_wifi} -n ${vendor_hostapd_ndk_lib_name}\
    -b ${vendor_hostapd_base_intf} -m ${vendor_hostapd_module_name} -f ${vendor_hostapd_msg_offset}
}

# qtiwifi
gen_qtiwifi_apcc()
{
    qtiwifi_aidl_path="${android_root}/vendor/qcom/proprietary/interfaces/wifi/qtiwifi/aidl/vendor/qti/hardware/wifi/qtiwifi"
    qtiwifi_proto_path="${wifi_proto_gen_root}/qtiwifi"
    qtiwifi_gen_path="${wifi_source_gen_root}/qtiwifi"
    qtiwifi_ndk_lib=vendor.qti.hardware.wifi.qtiwifi-V1-ndk
    qtiwifi_base_intf=IQtiWifi
    qtiwifi_module_name=wifi-vendor
    gen_apcc -a ${qtiwifi_aidl_path} -p ${qtiwifi_proto_path} -o ${qtiwifi_gen_path} -g ${gen_wifi} -n ${qtiwifi_ndk_lib} -b ${qtiwifi_base_intf}\
    -m ${qtiwifi_module_name}
}

# wifi
gen_wifi_code()
{
    # wifi
    gen_wifi_apcc

    # supplicant
    gen_supplicant_apcc

    # nlinterceptor
    gen_nlinterceptor_apcc

    # vendor supplicant
    gen_vendor_supplicant_apcc

    # hostapd
    gen_hostapd_apcc

    # vendor hostapd
    gen_vendor_hostapd_apcc

    # qtiwifi
    gen_qtiwifi_apcc
}

#############################################################
#
# Main
#
#############################################################

chmod +x ${apcc_bin}

# wifi
gen_wifi_code

# bluetooth
gen_bluetooth_code

echo_info "apcc done"
