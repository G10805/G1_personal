/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>

#include "aidl_2_proto.h"
#include "arg_search.h"
#include "log.h"
#include "util.h"

#ifndef EXCLUDE_A2P
#ifdef _WIN32
#define DEFAULT_AIDL_PATH   "C:\\work\\a2p\\aidl"
#define DEFAULT_PROTO_PATH  "C:\\work\\a2p\\proto"
#elif ANDROID
#define DEFAULT_AIDL_PATH   "/data/a2p/aidl"
#define DEFAULT_PROTO_PATH  "/data/a2p/proto"
#else
#define DEFAULT_AIDL_PATH   "~/a2p/aidl"
#define DEFAULT_PROTO_PATH  "~/a2p/proto"
#endif

static void output_a2p_help()
{
    const char* a2pHelpMessage =
    "a2p usage \n"
    "  --aidl-path:  aidl path or aidl file (*.aidl)\n"
    "    e.g.\n"
    "    aidl path:  a2p --aidl-path /data/wifi\n"
    "    aidl file:  a2p --aidl-path /data/wifi/IWifi.aidl\n"
    "  --proto-path: proto path\n"
    "    e.g.\n"
    "    aidl path:  a2p --proto-path /data/wifi/proto\n"
    "  --makefile-type: makefile type\n"
    "    e.g.\n"
    "    android bp:     a2p --makefile-type bp\n"
    "    linux makefile: a2p --makefile-type makefile\n"
    "    default value is android bp\n"
    "  --remove-file: remove exist proto file or not\n"
    "    e.g.\n"
    "    true:  a2p --remove-file true\n"
    "    false: a2p --remove-file false\n"
    "    default value is true\n"
    "  --package-name: package name\n"
    "    e.g.\n"
    "    aidl path:  a2p --package-name nameVendor\n"
    "  --help:       output help\n";
    logd(a2pHelpMessage);
}

static void get_default_a2p_param(Aidl2ProtoParam& param)
{
    param.aidlPath = DEFAULT_AIDL_PATH;
    param.protoPath = DEFAULT_PROTO_PATH;
    param.genHalStatus = true;
    param.makefileType = MakefileType::ANDROID_BP;
    param.removeFile = false;
    param.packageName = "";
}

static bool parse_a2p_param(Aidl2ProtoParam& param)
{
    char *parameter;
    char *value;

    get_default_a2p_param(param);

    if (ArgSearch(NULL, AIDL_PATH_PARAM, &parameter, &value))
    {
        if (value == NULL)
        {
            return false;
        }
        param.aidlPath = value;
    }
    if (ArgSearch(NULL, PROTO_PATH_PARAM, &parameter, &value))
    {
        if (value == NULL)
        {
            return false;
        }
        param.protoPath = value;
    }
    if (ArgSearch(NULL, GEN_HAL_STATUS_PARAM, &parameter, &value))
    {
        if (value == NULL)
        {
            return false;
        }
        param.genHalStatus = !strcmp(value, "true");
    }
    if (ArgSearch(NULL, MAKEFILE_TYPE_PARAM, &parameter, &value))
    {
        if (value == NULL)
        {
            return false;
        }
        if (!strcmp(value, "bp")) {
            param.makefileType = MakefileType::ANDROID_BP;
        } else if (!strcmp(value, "makefile")) {
            param.makefileType = MakefileType::LINUX_MAKEFILE;
        } else {
            loge("Invalid makefile type!");
            return false;
        }
    }
    if (ArgSearch(NULL, REMOVE_FILE_PARAM, &parameter, &value))
    {
        if (value == NULL)
        {
            return false;
        }
        param.removeFile = !strcmp(value, "true");
    }
    if (ArgSearch(NULL, PACKAGE_NAME, &parameter, &value))
    {
        if (value != NULL)
        {
            param.packageName = value;
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    Aidl2ProtoParam param;

    log_init();
    logd("a2p start");
    ArgSearchInit((uint32_t) argc, (char **) argv);
    if (!ArgSearchValidate())
    {
        loge("invalid parameter");
        return -1;
    }

    if (ArgSearch(NULL, "--help", NULL, NULL))
    {
        output_a2p_help();
        return 0;
    }

    if (!parse_a2p_param(param))
    {
        loge("fail to parse parameter");
        output_a2p_help();
        return -1;
    }
    return aidl_2_proto_main(param);
}
#endif  /* EXCLUDE_A2P */
