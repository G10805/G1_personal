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

#define COMMENT_HAL_RPC_METHODS_EXPOSED     "// HalRpc methods exposed"

static bool rsh_exist_aidl_intf_return(const ApccGenInfo& genInfo)
{
    return !genInfo.aidlIntfReturnInfo.empty();
}

static void rsh_add_attribute_line(const string& attr, stringstream& ss)
{
    ss << "  " << attr << ':' << endl;
}

static void rsh_add_class_def(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << CPP_CLASS << " " << genInfo.halRpcServerClassInfo.className << ';' << endl;
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

static void rsh_add_callback_class_public_func(const vector<FuncType>& halCallbackFunc, stringstream& ss)
{
    if (!halCallbackFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < halCallbackFunc.size(); index++)
        {
            apcc_add_override_func_def(halCallbackFunc[index], SPACES, ss);
            ss << endl;
        }
        ss << endl;
    }
}

static void rsh_add_callback_class_private_var(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << SPACES << genInfo.halRpcServerClassInfo.classType << " " <<
        genInfo.halRpcServerClassInfo.classVarInt << ';' << endl;
}

static void rsh_add_callback_class(const ApccGenInfo& genInfo, const string& aidlCallbackName,
    const string& className, const HalIntfFileInfo& halCallbackFileInfo,
    const vector<FuncType>& halCallbackFunc, stringstream& ss)
{
    if (!valid_str(aidlCallbackName))
        return;

    ss << CPP_CLASS << " " << className << " : " << CPP_PUBLIC << " " <<
        halCallbackFileInfo.halBnName << MOD_BEGIN_STR << endl;

    /* public: */
    rsh_add_attribute_line(CPP_PUBLIC, ss);

    /* constructor: HalCallback(shared_ptr<HalRpcServer> halRpcServer); */
    ss << SPACES << className << '(' << genInfo.halRpcServerClassInfo.classType <<
        ' ' << genInfo.halRpcServerClassInfo.classVar << ");" << endl;
    /* destructor: ~HalCallback(); */
    ss << SPACES << '~' << className << "();" << endl;
    ss << endl;

    rsh_add_callback_class_public_func(halCallbackFunc, ss);

    /* private: */
    rsh_add_attribute_line(CPP_PRIVATE, ss);
    rsh_add_callback_class_private_var(genInfo, ss);
    ss << MOD_END_STR << ';' << endl;
    ss << endl;
}

static void rsh_add_callback_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    rsh_add_callback_class(genInfo, genInfo.aidlCallbackName, genInfo.halCallbackClassInfo.className,
        genInfo.halCallbackFileInfo, genInfo.halCallbackFunc, ss);

    rsh_add_callback_class(genInfo, genInfo.aidlCallback2Name, genInfo.halCallback2ClassInfo.className,
        genInfo.halCallback2FileInfo, genInfo.halCallback2Func, ss);
}

/* ------------------------------------------------------------------------------------ */

static void rsh_add_intf_param(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_func_param(genInfo.halRpcServerClassInfo.intfParam, "", SPACES, ss);

    if (!genInfo.aidlCallbackInfo.empty())
    {
        for (auto it: genInfo.aidlCallbackInfo)
        {
            apcc_add_func_param(it.second.param, "", SPACES, ss);
        }
    }

    if (rsh_exist_aidl_intf_return(genInfo))
    {
        ss << endl;
        for (auto it: genInfo.aidlIntfReturnInfo)
        {
            const FuncParameter& halRpcServerInstListParam = it.second.halRpcServerInstListParam;
            apcc_add_variable(SPACES, halRpcServerInstListParam.type, halRpcServerInstListParam.name, ss);
        }
    }
}

static void rsh_add_class_derivation(const ApccGenInfo& genInfo, stringstream& ss)
{
    const string& className = genInfo.halRpcServerClassInfo.className;
    ss << CPP_CLASS << ' ' << className << " : " << CPP_PUBLIC << " " << HAL_RPC_SERVER_CLASS;

    ss << ", " << CPP_PUBLIC << " " << apcc_get_std_enable_shared_from_this(className);

    ss << MOD_BEGIN_STR << endl;
}

static void rsh_add_public_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halRpcServerOverrideFunc.empty())
    {
        ss << SPACES << COMMENT_HAL_RPC_METHODS_EXPOSED << endl;
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcServerOverrideFunc.size(); index++)
        {
            apcc_add_override_func_def(genInfo.halRpcServerOverrideFunc[index], SPACES, ss);
            ss << endl;
        }
        ss << endl;
    }

    if (!genInfo.halRpcServerPublicFunc.empty())
    {
        const FuncType& halRpcServerPublicFunc = genInfo.halRpcServerPublicFunc[0];

        /* const shared_ptr<IHal> getInterface() { return hal_; } */
        apcc_add_func_def(halRpcServerPublicFunc, SPACES, MOD_BEGIN_STR, ss);
        ss << " " << CPP_RETURN << " " << genInfo.halRpcServerClassInfo.intfParam.name << "; ";
        add_module_end(ss);

        ss << endl;
    }

    if (!genInfo.halRpcServerStaticFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcServerStaticFunc.size(); index++)
        {
            apcc_add_static_func_def(genInfo.halRpcServerStaticFunc[index], SPACES, ss);
            ss << endl;
        }
        ss << endl;
    }
}

static void rsh_add_private_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halRpcServerHandleReqFunc.empty())
    {
        apcc_add_func_list(genInfo.halRpcServerHandleReqFunc, SPACES, ";", ss);
        ss << endl;
    }
}

static void rsh_add_private_param(const ApccGenInfo& genInfo, stringstream& ss)
{
    rsh_add_intf_param(genInfo, ss);
}

static void rsh_add_rpc_server_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    rsh_add_class_derivation(genInfo, ss);

    /* public: */
    rsh_add_attribute_line(CPP_PUBLIC, ss);

    /* constructor: HalRpc(); */
    apcc_add_class_constructor_def(genInfo.halRpcServerClassInfo.constructorDefault, SPACES, ss);
    ss << endl;
    /* constructor: HalRpc(const shared_ptr<IHal>& hal, uint16_t instanceId); */
    apcc_add_class_constructor_def(genInfo.halRpcServerClassInfo.constructor, SPACES, ss);
    ss << endl;
    /* destructor: ~HalRpc(); */
    apcc_add_class_constructor_def(genInfo.halRpcServerClassInfo.deconstructor, SPACES, ss);
    ss << endl;
    ss << endl;

    /* public func */
    rsh_add_public_func(genInfo, ss);

    /* private: */
    rsh_add_attribute_line(CPP_PRIVATE, ss);
    /* private func */
    rsh_add_private_func(genInfo, ss);

    /* func param */
    rsh_add_private_param(genInfo, ss);

    ss << MOD_END_STR << ';' << endl;
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

static void rsh_add_api_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << endl;

    if (genInfo.halServerInfo.halRpcServerApiFunc.empty())
        return;

    const FuncType& funcType = genInfo.halServerInfo.halRpcServerApiFunc[0];
    const vector<FuncParameter>& param = funcType.param;
    if (param.size() < 2)
        return;

    ss << funcType.returnType << " " << funcType.funcName << '(' << endl;
    ss << SPACES << param[0].type << " " << param[0].name << ", " <<
        param[1].type << " " << param[1].name << ");" << endl;
}

/* ------------------------------------------------------------------------------------ */

static void rsh_add_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    rsh_add_class_def(genInfo, ss);

    rsh_add_callback_class(genInfo, ss);

    rsh_add_rpc_server_class(genInfo, ss);
}

static void rsh_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> usingType;

    apcc_add_using(aidl_get_using_type(genInfo.aidlPackageName, genInfo.aidlIntfName), ss);
    if (valid_str(genInfo.aidlCallbackName))
    {
        apcc_add_using(aidl_get_using_type(genInfo.aidlPackageName, genInfo.halCallbackFileInfo.halBnName), ss);
    }

    if (valid_str(genInfo.aidlCallback2Name))
    {
        apcc_add_using(aidl_get_using_type(genInfo.aidlPackageName, genInfo.halCallback2FileInfo.halBnName), ss);
    }

    apcc_get_all_using(genInfo, usingType);
    if (!contain_string(usingType, genInfo.halStatusInfo.halStatusUsing))
    {
        if (genInfo.halStatusInfo.existHalStatusType)
        {
            apcc_add_using(genInfo.halStatusInfo.halStatusUsing, ss);
        }
    }
    if (!usingType.empty())
    {
        apcc_add_using(usingType, ss);
        ss << endl;
    }

    if (rsh_exist_aidl_intf_return(genInfo))
    {
        /* using std::map; */
        apcc_add_std_using(CPP_MAP, ss);
        /* using std::pair; */
        apcc_add_std_using(CPP_PAIR, ss);
    }

    /* using qti::hal::rpc::HalRpc; */
    apcc_add_using(QTI_HAL_RPC, ss);
    /* using qti::hal::rpc::HalRpcServer; */
    apcc_add_using(QTI_HAL_RPC_SERVER, ss);
    ss << endl;
}

static void rsh_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> includeHeader;

    apcc_add_sys_include_header(genInfo.halIntfFileInfo.halBnHeaderFile, ss);
    if (valid_str(genInfo.aidlCallbackName))
    {
        apcc_add_sys_include_header(genInfo.halCallbackFileInfo.halBnHeaderFile, ss);
    }

    if (valid_str(genInfo.aidlCallback2Name))
    {
        apcc_add_sys_include_header(genInfo.halCallback2FileInfo.halBnHeaderFile, ss);
    }

    apcc_get_all_include(genInfo, includeHeader);
    if (!contain_string(includeHeader, genInfo.halStatusInfo.halStatusHeader))
    {
        if (genInfo.halStatusInfo.existHalStatusType)
        {
            apcc_add_sys_include_header(genInfo.halStatusInfo.halStatusHeader, ss);
        }
    }
    apcc_add_sys_include_header(includeHeader, ss);

    if (rsh_exist_aidl_intf_return(genInfo))
    {
        apcc_add_sys_include_header(CPP_MAP, ss);
        ss << endl;
        for (auto it: genInfo.aidlIntfReturnInfo)
        {
            apcc_add_include_header(it.second.includeHeader, ss);
        }
    }

    if (genInfo.isAidlIntfRoot)
    {
        apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlIntfName, APCC_TYPE_HAL_SERVER_API_HEADER), ss);
    }

    apcc_add_include_header(HAL_RPC_HEADER, ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_rpc_server_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    rsh_add_include(genInfo, ss);

    /* 4. namespace prefix */
    apcc_add_namespace_prefix(genInfo.halRpcNamespace, ss);

    /* 5. using */
    rsh_add_using(genInfo, ss);

    /* 6. class definition */
    rsh_add_class(genInfo, ss);

    /* 7. namespace prefix */
    apcc_add_namespace_suffix(genInfo.halRpcNamespace, ss);

    /* 8. api func */
    rsh_add_api_func(genInfo, ss);
    FUNC_LEAVE();
}
