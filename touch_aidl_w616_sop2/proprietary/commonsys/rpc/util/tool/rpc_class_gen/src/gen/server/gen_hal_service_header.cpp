/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "apcc_gen.h"

#include "log.h"
#include "util.h"

static void hsh_add_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_func_def(genInfo.halServerInfo.serviceInitFunc, "", ";", ss);
}

static void hsh_add_include(stringstream& ss)
{
    /* #include <cstdlib> */
    apcc_add_sys_include_header("cstdlib", ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_service_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    hsh_add_include(ss);

    /* 4. function */
    hsh_add_func(genInfo, ss);
    FUNC_LEAVE();
}