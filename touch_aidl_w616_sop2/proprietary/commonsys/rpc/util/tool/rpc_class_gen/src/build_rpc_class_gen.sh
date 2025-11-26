###############################################################################
#
# Copyright (c) Qualcomm Technologies, Inc.  and/or its subsidiaries.
#
# All Rights Reserved.
#
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#
###############################################################################

###############################################################################
#
# File: build_rpc_class_gen.sh
#
# Description : Script to build RPC class generator
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
a2p_root="${android_root}/vendor/qcom/proprietary/commonsys/rpc/util/tool/aidl_2_proto"
workspace_root="${android_root}/vendor/qcom/proprietary/commonsys/rpc/util/tool/rpc_class_gen"

# source path (a2p)
a2p_source_path="${a2p_root}/src"
a2p_aidl_source_path="${a2p_source_path}/aidl/src"
a2p_common_source_path="${a2p_source_path}/common/src"
a2p_proto_source_path="${a2p_source_path}/proto/src"

# include path (a2p)
a2p_include_path="${a2p_source_path}"
a2p_aidl_include_path="${a2p_source_path}/aidl/include"
a2p_common_include_path="${a2p_source_path}/common/include"
a2p_proto_include_path="${a2p_source_path}/proto/include"

# source path
source_path="${workspace_root}/src"
gen_source_path="${source_path}/gen"
gen_source_server_path="${source_path}/gen/server"

# include path
include_path="${source_path}/include"
config_include_path="${include_path}/config"

rpc_class_gen_bin="${workspace_root}/bin/rpc_class_gen"

echo_info()
{
    echo "$1" >&2
}

echo_info "build rpc_class_gen"
echo_info "source_path: ${source_path}"

rm -rf ${rpc_class_gen_bin}

gcc ${source_path}/*.cpp \
    ${gen_source_path}/*.cpp \
    ${gen_source_server_path}/*.cpp \
    ${a2p_source_path}/*.cpp \
    ${a2p_aidl_source_path}/*.cpp \
    ${a2p_common_source_path}/*.cpp \
    ${a2p_proto_source_path}/*.cpp \
    -I${include_path} \
    -I${config_include_path} \
    -I${a2p_include_path} \
    -I${a2p_aidl_include_path} \
    -I${a2p_common_include_path} \
    -I${a2p_proto_include_path} \
    -lstdc++ \
    -lstdc++fs \
    -DEXCLUDE_A2P \
    -o ${rpc_class_gen_bin}

echo_info "build rpc_class_gen done, rpc_class_gen_bin: $rpc_class_gen_bin"
