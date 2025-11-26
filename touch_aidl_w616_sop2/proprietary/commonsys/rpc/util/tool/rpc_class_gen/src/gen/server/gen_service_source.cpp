/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "apcc_gen.h"

#include "log.h"
#include "util.h"

static void svc_add_init_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << SPACES << CPP_IF << " (!";
    apcc_add_func_call(genInfo.halServerInfo.serviceInitFunc.funcName, "", "", "", ss);
    ss << ')' << endl;
    apcc_add_return_line(SPACES_2, "-1", ss);
    ss << endl;
}

static void svc_add_sleep_infinite(const string& prefix, stringstream& ss)
{
    /* std::this_thread::sleep_for(std::chrono::duration<long long>::max()); */
    ss << prefix << STD_NAMESPACE << "::" << "this_thread" << "::" << "sleep_for" <<
        '(' << STD_NAMESPACE << "::" << CPP_CHRONO << "::" << "duration" << "<long long>" <<
        "::" << "max()" << ");";
    ss << endl;
}

static void svc_init_main_func(FuncType& funcType)
{
    funcType.returnType = CPP_INT;
    funcType.funcName = CPP_MAIN;

    // The 1st param: int /* argc */
    funcType.param.push_back({CPP_INT, unused_param_name("argc")});
    // The 2nd param: char** /* argv */
    funcType.param.push_back({CPP_CHAR_POINTER_POINTER, unused_param_name("argv")});
}

static void svc_add_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    FuncType mainFunc;
    svc_init_main_func(mainFunc);

    apcc_add_func_impl(mainFunc, "", MOD_BEGIN_STR, ss);

    svc_add_init_func(genInfo, ss);

    svc_add_sleep_infinite(SPACES, ss);
    apcc_add_return_line(SPACES, "0", ss);
    add_module_end(ss);
}

static void svc_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* #include <thread> */
    apcc_add_sys_include_header(CPP_THREAD, ss);
    /* #include <chrono> */
    apcc_add_sys_include_header(CPP_CHRONO, ss);
    ss << endl;
    /* #include "hal_service.h" */
    apcc_add_include_header(genInfo.halServerInfo.serviceHeader, ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_service_source(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. include */
    svc_add_include(genInfo, ss);

    /* 3. function */
    svc_add_func(genInfo, ss);
    FUNC_LEAVE();
}