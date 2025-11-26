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

static string create_ndk_scoped_astatus(const string& statusCode)
{
    stringstream ss;
    ss << NDK_SCOPED_ASTATUS << "::" << FROM_SERVICE_SPECIFIC_ERROR_FUNC <<
        '(' << CPP_STATIC_CAST << '<' << CPP_INT32 << ">(" << statusCode << "))";
    return ss.str();
}

static string create_ndk_scoped_astatus(const string& statusCode, const string& info)
{
    stringstream ss;
    ss << NDK_SCOPED_ASTATUS << "::" << FROM_SERVICE_SPECIFIC_ERROR_WITH_MESSAGE_FUNC <<
        '(' << CPP_STATIC_CAST << '<' << CPP_INT32 << ">(" << statusCode << "), " <<
        info << ".c_str())";
    return ss.str();
}

static void sus_add_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    size_t paramSize = funcType.param.size();
    if (paramSize == 1)
    {
        apcc_add_return_line(SPACES, create_ndk_scoped_astatus(
            funcType.param[0].name), ss);
    }
    else if (paramSize == 2)
    {
        apcc_add_return_line(SPACES, create_ndk_scoped_astatus(
            funcType.param[0].name, funcType.param[1].name), ss);
    }
}

static void sus_add_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    for (auto rit = genInfo.halStatusInfo.createHalStatusFunc.crbegin();
        rit != genInfo.halStatusInfo.createHalStatusFunc.crend(); ++rit)
    {
        apcc_add_func_impl(*rit, "", MOD_BEGIN_STR, ss);

        sus_add_func(genInfo, *rit, ss);

        add_module_end(ss);
        ss << endl;
    }
}

static void sus_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlIntfName, APCC_TYPE_HAL_STATUS_UTIL_HEADER), ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_status_util_source(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. include */
    sus_add_include(genInfo, ss);

    /* 3. namespace prefix */
    apcc_add_namespace_prefix(genInfo.halRpcNamespace, ss);

    /* 4. func implementation */
    sus_add_func(genInfo, ss);

    /* 5. namespace prefix */
    apcc_add_namespace_suffix(genInfo.halRpcNamespace, ss);
    FUNC_LEAVE();
}
