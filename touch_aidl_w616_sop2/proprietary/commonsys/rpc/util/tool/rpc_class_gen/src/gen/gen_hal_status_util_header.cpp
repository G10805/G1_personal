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

static void suh_add_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    for (auto rit = genInfo.halStatusInfo.createHalStatusFunc.crbegin();
        rit != genInfo.halStatusInfo.createHalStatusFunc.crend(); ++rit)
    {
        apcc_add_func_def(*rit, "", ";", ss);
        ss << endl;
    }
    ss << endl;
}

static void suh_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_using(genInfo.halStatusInfo.halStatusUsing, ss);
    apcc_add_using(STD_NAMESPACE "::" CPP_STRING, ss);
    ss << endl;
}

static void suh_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_sys_include_header(genInfo.aidlHalRoot.headerFile, ss);
    apcc_add_sys_include_header(genInfo.halStatusInfo.halStatusHeader, ss);
    apcc_add_sys_include_header(CPP_STRING, ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_status_util_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    suh_add_include(genInfo, ss);

    /* 4. namespace prefix */
    apcc_add_namespace_prefix(genInfo.halRpcNamespace, ss);

    /* 5. using */
    suh_add_using(genInfo, ss);

    /* 6. func definition */
    suh_add_func(genInfo, ss);

    /* 7. namespace prefix */
    apcc_add_namespace_suffix(genInfo.halRpcNamespace, ss);
    FUNC_LEAVE();
}
