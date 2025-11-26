/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <string.h>

#include "arg_search.h"
#include "log.h"
#include "util.h"

#include "apcc_main.h"

#define NDK_LIB_PARAM           "--ndk-lib"
#define AIDL_SYNC_API_PARAM     "--aidl-sync-api"
#define MULTI_CALLBACK_PARAM    "--multi-callback"
#define VALIDATE_FUNC_PARAM     "--validate-func"
#define REMOVE_FILE_PARAM       "--remove-file"
#define DEFINE_CFM_PARAM        "--define-cfm"
#define SERVER_PARAM            "--server"
#define HAL_IMPL_STUB_PARAM     "--hal-impl-stub"
#define SINGLE_SOMEIP_PARAM     "--single-someip"
#define REMOVE_INTF_PARAM       "--remove-intf"
#define TEST_PARAM              "--test"

static const char* get_bool_str(bool val)
{
    return val ? "true" : "false";
}

static bool parse_arg_string(const char* prefix, string& result)
{
    char* parameter;
    char* value;

    if (ArgSearch(NULL, prefix, &parameter, &value))
    {
        if (value == NULL)
        {
            return false;
        }
        result = value;
    }
    return true;
}

static bool parse_arg_bool(const char* prefix, bool& result)
{
    char* parameter;
    char* value;

    if (ArgSearch(NULL, prefix, &parameter, &value))
    {
        if (value == NULL)
        {
            return false;
        }
        result = !strcmp(value, "true");
    }
    return true;
}

static void output_apcc_help()
{
    logd("apcc usage ");
    logd("  --aidl-path:     aidl path");
    logd("  --proto-path:    proto path");
    logd("  --gen-path:      generated source code path");
    logd("  --ndk-lib:       hal ndk lib name");
    logd("  --aidl-sync-api: flag indicating aidl sync api or not");
    logd("  --multi-callback: flag indicating multi callback or not");
    logd("  --validate-func: flag whether to add validate func in aidl func or not");
    logd("  --remove-file:   flag whether to remove existing source file or not");
    logd("  --define-cfm:    flag whether to define cfm message or not");
    logd("  --server:        flag whether to generate source code for hal rpc server or not");
    logd("  --single-someip: flag whether to use single someip in multi AIDL intf or not");
    logd("  --remove-intf:   flag whether to remove hal rpc server instance or not");
    logd("  --test:          flag whether to test apcc in empty function or not");
    logd("  --help:          output help");
}

static void dump_apcc_param(ApccParam& param)
{
    logd("aidl-path:     %s", param.aidlPath.c_str());
    logd("proto-path:    %s", param.protoPath.c_str());
    logd("gen-path:      %s", param.genPath.c_str());
    logd("ndk-lib:       %s", param.ndkLib.c_str());
    logd("aidl-sync-api: %s", param.aidlSyncApi ? "sync" : "async");
    logd("multi-callback: %s", get_bool_str(param.multiCallback));
    logd("validate-func: %s", get_bool_str(param.validateFunc));
    logd("remove-file:   %s", get_bool_str(param.removeFile));
    logd("define-cfm:    %s", get_bool_str(param.defineCfm));
    logd("server:        %s", get_bool_str(param.server));
    logd("hal-impl-stub: %s", get_bool_str(param.halImplStub));
    logd("single-someip: %s", get_bool_str(param.singleSomeip));
    logd("remove-intf:   %s", get_bool_str(param.removeIntf));
    logd("test:          %s", get_bool_str(param.test));
}

static void init_apcc_param(ApccParam& param)
{
    param.aidlPath = DEFAULT_APCC_AIDL_PATH;
    param.protoPath = DEFAULT_APCC_PROTO_PATH;
    param.genPath = DEFAULT_APCC_GEN_SOURCE_PATH;
    param.ndkLib = "";
    param.aidlSyncApi = true;
    param.multiCallback = false;
    param.validateFunc = false;
    param.removeFile = false;
    param.defineCfm = false;
    param.server = false;
    param.halImplStub = false;
    param.singleSomeip = true;
    param.removeIntf = false;
    param.test = false;
}

static bool parse_apcc_param(ApccParam& param)
{
    init_apcc_param(param);

    if (!parse_arg_string(AIDL_PATH_PARAM, param.aidlPath))
        return false;

    if (!parse_arg_string(PROTO_PATH_PARAM, param.protoPath))
        return false;

    if (!parse_arg_string(GEN_PATH_PARAM, param.genPath))
        return false;

    if (!parse_arg_string(NDK_LIB_PARAM, param.ndkLib))
        return false;

    if (!parse_arg_bool(AIDL_SYNC_API_PARAM, param.aidlSyncApi))
        return false;

    if (!parse_arg_bool(MULTI_CALLBACK_PARAM, param.multiCallback))
        return false;

    if (!parse_arg_bool(VALIDATE_FUNC_PARAM, param.validateFunc))
        return false;

    if (!parse_arg_bool(REMOVE_FILE_PARAM, param.removeFile))
        return false;

    if (!parse_arg_bool(DEFINE_CFM_PARAM, param.defineCfm))
        return false;

    if (!parse_arg_bool(SERVER_PARAM, param.server))
        return false;

    if (!parse_arg_bool(HAL_IMPL_STUB_PARAM, param.halImplStub))
        return false;

    if (!parse_arg_bool(SINGLE_SOMEIP_PARAM, param.singleSomeip))
        return false;

    if (!parse_arg_bool(REMOVE_INTF_PARAM, param.removeIntf))
        return false;

    if (!parse_arg_bool(TEST_PARAM, param.test))
        return false;

    dump_apcc_param(param);
    return true;
}

int main(int argc, char *argv[])
{
    ApccParam& param = apcc_get_param();

    log_init();
    logd("apcc start");
    ArgSearchInit((uint32_t) argc, (char **) argv);
    if (!ArgSearchValidate())
    {
        loge("invalid parameter");
        return -1;
    }

    if (ArgSearch(NULL, "--help", NULL, NULL))
    {
        output_apcc_help();
        return 0;
    }

    if (!parse_apcc_param(param))
    {
        loge("fail to parse parameter");
        output_apcc_help();
        return -1;
    }
    return apcc_main(param);
}
