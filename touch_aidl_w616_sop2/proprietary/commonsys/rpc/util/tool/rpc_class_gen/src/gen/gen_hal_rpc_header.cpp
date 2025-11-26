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

#define COMMENT_HAL_AIDL_ROOT     "\
/**\n\
 * Root AIDL interface object used to control the Hal HAL.\n\
 */\n\
"

#define COMMENT_AIDL_METHODS_EXPOSED        "// AIDL methods exposed"
#define COMMENT_HAL_RPC_METHODS_EXPOSED     "// HalRpc methods exposed"
#define COMMENT_AIDL_INTERNAL_FUNC          "// Corresponding worker functions for the AIDL methods"

static bool exist_aidl_intf_return_type(const ApccGenInfo& genInfo)
{
    return genInfo.isMultiAidlIntf && !genInfo.halAidlIntfInfo.empty();
}

/* ------------------------------------------------------------------------------------ */

static void rph_add_hal_aidl_root_comment(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.isAidlIntfRoot)
        return;

    string comment = COMMENT_HAL_AIDL_ROOT;
    replace_string(comment, "Hal", genInfo.halName);
    ss << comment;
}

static void rph_add_class_derivation(const ApccGenInfo& genInfo, stringstream& ss)
{
    const string& className = genInfo.halRpcClassInfo.className;

    ss << CPP_CLASS << ' ' << className << " : " << CPP_PUBLIC << " " <<
        genInfo.halIntfFileInfo.halBnName << ", " << CPP_PUBLIC << " " << HAL_RPC_CLIENT_CLASS;

    ss << ", " << CPP_PUBLIC << " " << apcc_get_std_enable_shared_from_this(className);

    ss << MOD_BEGIN_STR << endl;
}

static void rph_add_public_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halRpcFunc.empty())
    {
        ss << SPACES << COMMENT_AIDL_METHODS_EXPOSED << endl;
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcFunc.size(); index++)
        {
            apcc_add_override_func_def(genInfo.halRpcFunc[index], SPACES, ss);
            ss << endl;
        }
        ss << endl;
    }

    if (!genInfo.halRpcOverrideFunc.empty())
    {
        ss << SPACES << COMMENT_HAL_RPC_METHODS_EXPOSED << endl;
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcOverrideFunc.size(); index++)
        {
            apcc_add_override_func_def(genInfo.halRpcOverrideFunc[index], SPACES, ss);
            ss << endl;
        }
        ss << endl;
    }

    if (!genInfo.halRpcStaticFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcStaticFunc.size(); index++)
        {
            apcc_add_static_func_def(genInfo.halRpcStaticFunc[index], SPACES, ss);
            ss << endl;
        }
        ss << endl;
    }
}

static void rph_add_private_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* private: */
    ss << "  " << CPP_PRIVATE << ':' << endl;

    /* Add internal func */
    if (!genInfo.halRpcInternalFunc.empty())
    {
        ss << SPACES << COMMENT_AIDL_INTERNAL_FUNC << endl;
        apcc_add_func_list(genInfo.halRpcInternalFunc, SPACES, ";", ss);
        ss << endl;
    }

    if (!genInfo.halRpcHandleIndFunc.empty())
    {
        apcc_add_func_list(genInfo.halRpcHandleIndFunc, SPACES, ";", ss);
        ss << endl;
    }

    if (!genInfo.halRpcHandleInd2Func.empty())
    {
        apcc_add_func_list(genInfo.halRpcHandleInd2Func, SPACES, ";", ss);
        ss << endl;
    }

    if (!genInfo.halRpcHandleUpReqFunc.empty())
    {
        apcc_add_func_list(genInfo.halRpcHandleUpReqFunc, SPACES, ";", ss);
        ss << endl;
    }

    if (!genInfo.halRpcCallbackUtilInfo.empty())
    {
        for (auto it: genInfo.halRpcCallbackUtilInfo)
        {
            apcc_add_func_def(it.funcType, SPACES, ";", ss);
            ss << endl;
        }
        ss << endl;
    }
}

static void rph_add_func_param(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalRpcClassInfo& halRpcClassInfo = genInfo.halRpcClassInfo;

    if (!genInfo.aidlIntfReturnInfo.empty())
    {
        for (auto it: genInfo.aidlIntfReturnInfo)
        {
            const FuncParameter& halRpcInstListParam = it.second.halRpcInstListParam;
            apcc_add_variable(SPACES, halRpcInstListParam.type, halRpcInstListParam.name, ss);
        }
        ss << endl;
    }

    if (halRpcClassInfo.existCallback)
    {
        apcc_add_func_param(halRpcClassInfo.callbackVar, "", SPACES, ss);
        ss << endl;
    }
    if (halRpcClassInfo.existCallback2)
    {
        apcc_add_func_param(halRpcClassInfo.callbackVar2, "", SPACES, ss);
        ss << endl;
    }
    if (halRpcClassInfo.existCallbackResult)
    {
        apcc_add_func_param(halRpcClassInfo.callbackVarResult, "", SPACES, ss);
        ss << endl;
    }

    if (genInfo.isAidlIntfRoot)
    {
        apcc_add_func_param(halRpcClassInfo.staticInst, CPP_STATIC " ", SPACES, ss);
        ss << endl;

        if (genInfo.isSingleSomeip)
        {
            apcc_add_func_param(halRpcClassInfo.staticProxy, CPP_STATIC " ", SPACES, ss);
            ss << endl;
        }
    }

    ss << SPACES << DISALLOW_COPY_AND_ASSIGN_MACRO << '(' << genInfo.halRpcClassInfo.className << ");" << endl;
}

/* ------------------------------------------------------------------------------------ */

static void rph_add_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    const string& paramType = genInfo.isAidlIntfRoot ? HAL_INST_PARAM_TYPE : INSTANCE_ID_TYPE;
    const string& paramName = genInfo.isAidlIntfRoot ? HAL_INST_PARAM_NAME : INSTANCE_ID_NAME;

    rph_add_hal_aidl_root_comment(genInfo, ss);

    rph_add_class_derivation(genInfo, ss);

    /* public: */
    ss << "  " << CPP_PUBLIC << ':' << endl;

    /* constructor: HalRpc(); */
    ss << SPACES << genInfo.halRpcClassInfo.className << "();" << endl;
    /* constructor: HalRpc(const char* instance_name); */
    ss << SPACES << genInfo.halRpcClassInfo.className << '(' << paramType << ' ' << paramName << ");" << endl;
    /* destructor: ~HalRpc(); */
    ss << SPACES << '~' << genInfo.halRpcClassInfo.className << "();" << endl;
    ss << endl;

    if (apcc_is_validate_func_supported())
    {
        /* bool isValid(); */
        ss << SPACES << CPP_BOOL << " " << IS_VALID_FUNC << ';' << endl;
        ss << endl;
    }

    /* public func */
    rph_add_public_func(genInfo, ss);

    /* private func */
    rph_add_private_func(genInfo, ss);

    /* func param */
    rph_add_func_param(genInfo, ss);

    ss << "};" << endl;
    ss << endl;
}

static void rph_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> usingType;

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

    if (exist_aidl_intf_return_type(genInfo))
    {
        /* using std::map; */
        apcc_add_std_using(CPP_MAP, ss);
    }

    /* using std::pair; */
    apcc_add_std_using(CPP_PAIR, ss);

    /* using qti::hal::rpc::HalRpc; */
    apcc_add_using(QTI_HAL_RPC, ss);
    /* using qti::hal::rpc::HalRpcClient; */
    apcc_add_using(QTI_HAL_RPC_CLIENT, ss);
    /* using qti::hal::rpc::SomeipMap; */
    if (apcc_is_aidl_sync_api())
        apcc_add_using(QTI_SOMEIP_MAP, ss);

    ss << endl;
}

static void rph_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_sys_include_header(genInfo.halIntfFileInfo.halBnHeaderFile, ss);
    apcc_add_sys_include_header(ANDROID_BASE_MACROS_HEADER, ss);
    ss << endl;

    apcc_add_intf_include(genInfo, ss);

    if (exist_aidl_intf_return_type(genInfo))
    {
        apcc_add_sys_include_header(CPP_MAP, ss);
        ss << endl;
    }

    apcc_add_include_header(HAL_RPC_HEADER, ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_rpc_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    rph_add_include(genInfo, ss);

    /* 4. namespace prefix */
    apcc_add_namespace_prefix(genInfo.halRpcNamespace, ss);

    /* 5. using */
    rph_add_using(genInfo, ss);

    /* 6. class definition */
    rph_add_class(genInfo, ss);

    /* 7. namespace prefix */
    apcc_add_namespace_suffix(genInfo.halRpcNamespace, ss);
    FUNC_LEAVE();
}
