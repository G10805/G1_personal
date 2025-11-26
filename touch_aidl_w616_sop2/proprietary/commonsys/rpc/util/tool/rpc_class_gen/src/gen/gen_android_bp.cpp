/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "aidl.h"
#include "aidl_util.h"

#include "apcc_gen.h"

#include "proto.h"
#include "proto_util.h"

#include "log.h"
#include "util.h"

#define HAL_MESSAGE_ANDROID_BP        "\
cc_library_shared {\n\
    name: \"libqti-hal-message\",\n\
    owner: \"qti\",\n\
    proprietary: true,\n\
\n\
    srcs: [\n\
        \"message/src/*.cpp\",\n\
    ],\n\
\n\
    cflags: [\"-Wall\", \"-Werror\"],\n\
\n\
    static_libs: [\n\
        \"libqti-hal-proto\",\n\
    ],\n\
\n\
    shared_libs: [\n\
        \"android.hardware.hal-V1-ndk\",\n\
        \"libbase\",\n\
        \"libbinder_ndk\",\n\
        \"libcutils\",\n\
        \"liblog\",\n\
        \"libprotobuf-cpp-lite\",\n\
        \"libqti_hal_rpc_util\",\n\
        \"libqti_proto_util\",\n\
        \"libutils\",\n\
    ],\n\
\n\
    export_include_dirs: [\n\
        \"include\",\n\
        \"message/include\",\n\
    ],\n\
\n\
    sanitize: {\n\
        integer_overflow: true,\n\
    },\n\
}"

#define HAL_RPC_CLIENT_ANDROID_BP        "\
cc_library_shared {\n\
    name: \"libqti-hal-rpc\",\n\
    owner: \"qti\",\n\
    proprietary: true,\n\
\n\
    srcs: [\n\
        \"aidl/*.cpp\",\n\
        \"aidl/util/*.cpp\",\n\
    ],\n\
\n\
    cflags: [\"-Wall\", \"-Werror\"],\n\
\n\
    static_libs: [\n\
        \"libqti-hal-proto\",\n\
    ],\n\
\n\
    shared_libs: [\n\
        \"android.hardware.hal-V1-ndk\",\n\
        \"libbase\",\n\
        \"libbinder_ndk\",\n\
        \"libcutils\",\n\
        \"liblog\",\n\
        \"libqti_hal_rpc_util\",\n\
        \"libqti_log_util\",\n\
        \"libqti_message_sched_util\",\n\
        \"libqti_proto_util\",\n\
        \"libqti_rpc_common_util\",\n\
        \"libqti_someip_util\",\n\
        \"libqti-hal-message\",\n\
        \"libutils\",\n\
        \"libvsomeip3\",\n\
    ],\n\
\n\
    export_include_dirs: [\n\
        \"aidl\",\n\
        \"aidl/api\",\n\
        \"aidl/util\",\n\
    ],\n\
\n\
    sanitize: {\n\
        integer_overflow: true,\n\
    },\n\
}"

#define HAL_IMPL_LIB_ANDROID_BP        "\
cc_library_shared {\n\
    name: \"libqti-hal-impl\",\n\
    owner: \"qti\",\n\
    proprietary: true,\n\
\n\
    srcs: [\n\
        \"impl/*.cpp\",\n\
    ],\n\
\n\
    cflags: [\"-Wall\", \"-Werror\"],\n\
\n\
    shared_libs: [\n\
        \"android.hardware.hal-V1-ndk\",\n\
        \"libbase\",\n\
        \"libbinder_ndk\",\n\
        \"libcutils\",\n\
        \"liblog\",\n\
    ],\n\
\n\
    export_include_dirs: [\n\
        \"api\",\n\
    ],\n\
\n\
    sanitize: {\n\
        integer_overflow: true,\n\
    },\n\
}"

#define HAL_SERVICE_LIB_ANDROID_BP        "\
cc_library_shared {\n\
    name: \"libqti-hal-service\",\n\
    owner: \"qti\",\n\
    proprietary: true,\n\
\n\
    srcs: [\n\
        \"*.cpp\",\n\
        \"util/*.cpp\",\n\
    ],\n\
\n\
    cflags: [\"-Wall\", \"-Werror\"],\n\
\n\
    static_libs: [\n\
        \"libqti-hal-proto\",\n\
    ],\n\
\n\
    shared_libs: [\n\
        \"android.hardware.hal-V1-ndk\",\n\
        \"libbase\",\n\
        \"libbinder_ndk\",\n\
        \"libcutils\",\n\
        \"liblog\",\n\
        \"libqti_hal_rpc_util\",\n\
        \"libqti_log_util\",\n\
        \"libqti_message_sched_util\",\n\
        \"libqti_proto_util\",\n\
        \"libqti_rpc_common_util\",\n\
        \"libqti_someip_util\",\n\
        \"libqti-hal-impl\",\n\
        \"libqti-hal-message\",\n\
        \"libutils\",\n\
        \"libvsomeip3\",\n\
    ],\n\
\n\
    export_include_dirs: [\n\
        \"include\",\n\
        \"util\",\n\
    ],\n\
\n\
    sanitize: {\n\
        integer_overflow: true,\n\
    },\n\
}"

#define HAL_SERVICE_BIN_ANDROID_BP        "\
cc_binary {\n\
    name: \"qti-hal-service\",\n\
    relative_install_path: \"hw\",\n\
    vendor: true,\n\
    srcs: [\n\
        \"*.cpp\",\n\
    ],\n\
    shared_libs: [\n\
        \"libbase\",\n\
        \"libbinder_ndk\",\n\
        \"libhidlbase\",\n\
        \"libutils\",\n\
        \"liblog\",\n\
        \"libqti-hal-service\",\n\
    ],\n\
}"

static bool get_android_bp_str(ApccType apccType, string& str)
{
    switch (apccType)
    {
        case APCC_TYPE_ANDROID_BP:
        {
            str = HAL_MESSAGE_ANDROID_BP;
            return true;
        }
        case APCC_TYPE_CLIENT_ANDROID_BP:
        {
            str = HAL_RPC_CLIENT_ANDROID_BP;
            return true;
        }
        case APCC_TYPE_SERVER_ANDROID_BP:
        {
            str = HAL_IMPL_LIB_ANDROID_BP;
            return true;
        }
        case APCC_TYPE_SERVER_LIB_ANDROID_BP:
        {
            str = HAL_SERVICE_LIB_ANDROID_BP;
            return true;
        }
        case APCC_TYPE_SERVER_MAIN_ANDROID_BP:
        {
            str = HAL_SERVICE_BIN_ANDROID_BP;
            return true;
        }
        default:
            /* unknown apcc type */
            return false;
    }
}

static string get_hal_ndk_lib(const string& aidlPackageName, const string& ndkLib = "")
{
    stringstream ss;
    if (is_empty_string(ndkLib))
        ss << '"' << aidlPackageName << "-V" << "1" << "-ndk" << '"';
    else
        ss << '"' << ndkLib << '"';
    return ss.str();
}

static string get_hal_bin_name(const string& halBinName, const string& halNameLower)
{
    string str = halBinName;
    if (contain_suffix(halNameLower, "rpc") &&
        contain_string(halBinName, "hal_rpc"))
        replace_string(str, "hal_rpc", halNameLower);
    else
        replace_string(str, "hal", halNameLower);

    return str;
}

static bool is_hal_server(ApccType apccType)
{
    return (apccType == APCC_TYPE_SERVER_ANDROID_BP) ||
            (apccType == APCC_TYPE_SERVER_LIB_ANDROID_BP) ||
            (apccType == APCC_TYPE_SERVER_MAIN_ANDROID_BP);
}

static void update_hal_server_android_bp(const ApccGenInfo& genInfo, const string hal_name_lower, string& str)
{
    replace_string(str, QTI_HAL_IMPL_LIB, get_hal_bin_name(QTI_HAL_IMPL_LIB, hal_name_lower));
    replace_string(str, QTI_HAL_SERVICE_LIB, get_hal_bin_name(QTI_HAL_SERVICE_LIB, hal_name_lower));
    replace_string(str, QTI_HAL_SERVICE_BIN, get_hal_bin_name(QTI_HAL_SERVICE_BIN, hal_name_lower));

    if (!genInfo.halStatusInfo.existHalStatusType)
    {
        logd("%s: not exist hal status type in server", __func__);
        replace_string(str, QTI_HAL_SERVER_UTIL_SOURCE, "");
        replace_string(str, QTI_HAL_SERVER_UTIL_HEADER, "");
    }
}

/* ------------------------------------------------------------------------------------ */

void gen_android_bp(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();
    bool server = is_hal_server(genInfo.apccType);
    string str;
    if (!get_android_bp_str(genInfo.apccType, str))
    {
        /* ignore due to unknown apcc type */
        return;
    }

    string aidlIntfRoot = aidl_get_intf_root();
    if (is_empty_string(aidlIntfRoot))
    {
        /* ignore due to empty aidl intf root */
        return;
    }
    logd("%s: aidl intf root name: %s", __func__, aidlIntfRoot.c_str());
    string aidlPackageName = aidl_get_package_name(aidlIntfRoot);
    logd("%s: aidl package name: %s", __func__, aidlPackageName.c_str());
    string hal_name_lower = contain_string(aidlPackageName, AIDL_HAL_PACKAGE_PREFIX) ?
                            proto_get_hal_name_lower(aidlPackageName) :
                            get_format_string_in_lower(aidl_get_interface_short_name(aidlIntfRoot), '-');
    logd("%s: hal name: %s", __func__, hal_name_lower.c_str());

    string ndkLib = apcc_get_ndk_lib();
    logd("%s: ndk lib: %s", __func__, ndkLib.c_str());

    replace_string(str, HAL_NDK_LIB, get_hal_ndk_lib(aidlPackageName, apcc_get_ndk_lib()));
    replace_string(str, QTI_HAL_MESSAGE_LIB, get_hal_bin_name(QTI_HAL_MESSAGE_LIB, hal_name_lower));
    replace_string(str, QTI_HAL_RPC_LIB, get_hal_bin_name(QTI_HAL_RPC_LIB, hal_name_lower));
    replace_string(str, QTI_HAL_PROTO_LIB, get_hal_bin_name(QTI_HAL_PROTO_LIB, hal_name_lower));

    if (!genInfo.halStatusInfo.existHalStatusType)
    {
        logd("%s: not exist hal status type", __func__);
        replace_string(str, QTI_HAL_AIDL_UTIL_SOURCE, "");
        replace_string(str, QTI_HAL_AIDL_UTIL_HEADER, "");
    }

    if (server)
    {
        update_hal_server_android_bp(genInfo, hal_name_lower, str);
    }
    ss << str;
}
