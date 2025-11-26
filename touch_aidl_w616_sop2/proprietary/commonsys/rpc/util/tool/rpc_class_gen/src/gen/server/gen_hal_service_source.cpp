/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "apcc_gen.h"

#include "log.h"
#include "util.h"

static string hss_get_hal_default_instance()
{
    stringstream ss;
    ss << '"' << HAL_DEFAULT_INSTANCE << '"';
    return ss.str();
}

static string hss_get_create_hal_rpc_server_param(const string& aidlIntf, uint16_t instanceId)
{
    stringstream ss;
    ss << aidlIntf << ", " << instanceId;
    return ss.str();
}

static void hss_add_verify_shared_ptr(const string& prefix, const string& param, stringstream& ss)
{
    ss << prefix << CPP_IF << " (" << param << " == " << CPP_NULLPTR << ')' << endl;
    ss << prefix << SPACES << CPP_RETURN << " " << CPP_FALSE << ';' << endl;
}

static void hss_add_hal_api(const ApccGenInfo& genInfo, const string& prefix, stringstream& ss)
{
    if (genInfo.halApiFunc.empty())
        return;

    const FuncType& halApiFunc = genInfo.halApiFunc[0];

    ss << prefix << halApiFunc.returnType << " " << genInfo.halNameLower << " = ";
    apcc_add_func_call(halApiFunc.funcName, hss_get_hal_default_instance(), "", ";", ss);
    ss << endl;
}

static void hss_add_hal_rpc_server(const ApccGenInfo& genInfo, const string& prefix, stringstream& ss)
{
    if (genInfo.halServerInfo.halRpcServerApiFunc.empty())
        return;

    const FuncType& halRpcServerApiFunc = genInfo.halServerInfo.halRpcServerApiFunc[0];

    ss << prefix << apcc_get_std_shared_ptr_type(genInfo.halRpcServerClassInfo.className) <<
        " " << genInfo.halServerInfo.halRpcServerName << " = ";
    apcc_add_func_call(halRpcServerApiFunc.funcName,
        hss_get_create_hal_rpc_server_param(genInfo.halNameLower, 0), "", ";", ss);
    ss << endl;
}

static void hss_add_hal_rpc_server_init(const ApccGenInfo& genInfo, const string& prefix, stringstream& ss)
{
    if (genInfo.isSingleSomeip)
    {
        /* hal_rpc_server->setAddHeaderFlag(true); */
        ss << prefix << genInfo.halServerInfo.halRpcServerName << "->" <<
            SET_ADD_HEADER_FLAG_NAME << '(' << CPP_TRUE << ");" << endl;
    }
}

static void hss_add_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_func_impl(genInfo.halServerInfo.serviceInitFunc, "", "", ss);
    add_module_begin("", ss);

    hss_add_hal_api(genInfo, SPACES, ss);
    hss_add_verify_shared_ptr(SPACES, genInfo.halNameLower, ss);
    ss << endl;

    hss_add_hal_rpc_server(genInfo, SPACES, ss);
    hss_add_verify_shared_ptr(SPACES, genInfo.halServerInfo.halRpcServerName, ss);
    ss << endl;

    hss_add_hal_rpc_server_init(genInfo, SPACES, ss);
    apcc_add_return_line(SPACES, CPP_TRUE, ss);
    add_module_end(ss);
}

static void hss_add_using_type(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_using(genInfo.halServerInfo.halRpcServerUsingType, ss);
    ss << endl;
}

static void hss_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* #include "hal_service.h" */
    apcc_add_include_header(genInfo.halServerInfo.serviceHeader, ss);
    /* #include "hal_rpc_server.h" */
    apcc_add_include_header(genInfo.halServerInfo.halRpcServerHeader, ss);
    /* #include "hal_api.h" */
    apcc_add_include_header(genInfo.halServerInfo.halApiHeader, ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_service_source(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. include */
    hss_add_include(genInfo, ss);

    /* 3. using */
    hss_add_using_type(genInfo, ss);

    /* 3. function */
    hss_add_func(genInfo, ss);
    FUNC_LEAVE();
}