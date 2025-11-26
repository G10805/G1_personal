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
# File: build_a2p.sh
#
# Description : Script to build A2P(AIDL-To-Proto) bin
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

default_android_root="~/android"
android_root=${1:-${default_android_root}}
workspace_root="${android_root}/vendor/qcom/proprietary/commonsys/rpc/util/tool/aidl_2_proto"

# source path
source_path="${workspace_root}/src"
aidl_source_path="${source_path}/aidl/src"
common_source_path="${source_path}/common/src"
proto_source_path="${source_path}/proto/src"

# include path
include_path="${source_path}"
aidl_include_path="${source_path}/aidl/include"
common_include_path="${source_path}/common/include"
proto_include_path="${source_path}/proto/include"

a2p_bin="${workspace_root}/bin/a2p"

echo_info()
{
    echo "$1" >&2
}

echo_info "build a2p"
echo_info "aidl_source_path: ${aidl_source_path}"

rm -rf ${a2p_bin}

gcc ${source_path}/*.cpp \
    ${aidl_source_path}/*.cpp \
    ${common_source_path}/*.cpp \
    ${proto_source_path}/*.cpp \
    -I${include_path} \
    -I${aidl_include_path} \
    -I${common_include_path} \
    -I${proto_include_path} \
    -lstdc++ \
    -lstdc++fs \
    -o ${a2p_bin}

echo_info "build a2p done, a2p_bin: $a2p_bin"
