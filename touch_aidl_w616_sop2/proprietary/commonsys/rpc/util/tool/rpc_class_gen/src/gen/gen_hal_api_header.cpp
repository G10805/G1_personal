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

#define COMMENT_HAL_RPC_API   "\
/**\n\
 * Create Hal AIDL instance for RPC\n\
 *\n\
 *   instance_name: Hal HAL instance name (e.g. \"default\")\n\
 *\n\
 *   return: Hal AIDL instance for RPC\n\
 */\
"

#define COMMENT_HAL_SERVER_API   "\
/**\n\
 * Create Hal AIDL instance\n\
 *\n\
 *   instance_name: Hal HAL instance name (e.g. \"default\")\n\
 *\n\
 *   return: Hal AIDL instance\n\
 */\
"

struct HalApiHeaderInstance: HalBaseInstance
{
};

static HalApiHeaderInstance *sHalApiHeaderInstance = NULL;

static void hah_add_api_func_comment(const ApccGenInfo& genInfo, stringstream& ss)
{
    string str = (genInfo.apccType == APCC_TYPE_HAL_SERVER_API_HEADER) ?
        COMMENT_HAL_SERVER_API : COMMENT_HAL_RPC_API;
    replace_string(str, "Hal", genInfo.halName);
    ss << str << endl;
}

static void hah_add_api_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    hah_add_api_func_comment(genInfo, ss);
    apcc_add_func_list(genInfo.halApiFunc, "", ";", ss);
}

static void hah_init_include_header(const ApccGenInfo& genInfo, vector<string>& includeHeader)
{
    includeHeader.push_back(aidl_get_include_header(genInfo.aidlPackageName, genInfo.aidlIntfName));
}

static void hah_init_using_type(const ApccGenInfo& genInfo, vector<string>& usingType)
{
    usingType.push_back(aidl_get_using_type(genInfo.aidlPackageName, genInfo.aidlIntfName));
}

static HalApiHeaderInstance* hah_create_instance()
{
    if (sHalApiHeaderInstance != NULL)
    {
        delete sHalApiHeaderInstance;
    }
    sHalApiHeaderInstance = new HalApiHeaderInstance;
    return sHalApiHeaderInstance;
}

static void hah_init_gen(const ApccGenInfo& genInfo)
{
    HalApiHeaderInstance* inst = hah_create_instance();
    hah_init_include_header(genInfo, inst->includeHeader);
    hah_init_using_type(genInfo, inst->usingType);
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_api_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    hah_init_gen(genInfo);

    HalApiHeaderInstance* inst = sHalApiHeaderInstance;

    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    apcc_add_sys_include_header(inst->includeHeader[0], ss);
    ss << endl;

    /* 4. using */
    apcc_add_using(inst->usingType[0], ss);
    ss << endl;

    /* 5. func */
    hah_add_api_func(genInfo, ss);
    FUNC_LEAVE();
}
