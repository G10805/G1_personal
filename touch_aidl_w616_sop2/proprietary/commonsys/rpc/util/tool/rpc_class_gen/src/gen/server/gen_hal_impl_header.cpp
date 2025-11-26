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
#define COMMENT_AIDL_INTERNAL_FUNC          "// Corresponding worker functions for the AIDL methods"

static bool exist_aidl_intf_return_type(const ApccGenInfo& genInfo)
{
    return genInfo.isMultiAidlIntf && !genInfo.halAidlIntfInfo.empty();
}

/* ------------------------------------------------------------------------------------ */

static void hih_add_hal_aidl_root_comment(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.isAidlIntfRoot)
        return;

    string comment = COMMENT_HAL_AIDL_ROOT;
    replace_string(comment, "Hal", genInfo.halName);
    ss << comment;
}

static void hih_add_class_derivation(const ApccGenInfo& genInfo, stringstream& ss)
{
    const string& className = genInfo.halClassInfo.className;

    ss << CPP_CLASS << ' ' << className << " : " << CPP_PUBLIC << " " <<
        genInfo.halIntfFileInfo.halBnName << MOD_BEGIN_STR << endl;
}

static void hih_add_callback_func(const vector<FuncType>& callbackFunc, stringstream& ss)
{
    if (!callbackFunc.empty())
    {
        for (auto it: callbackFunc)
        {
            apcc_add_func_def(it, SPACES, ";", ss);
            ss << endl;
        }
        ss << endl;
    }
}

static void hih_add_public_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalClassInfo& halClassInfo = genInfo.halClassInfo;

    /* Hal Impl override the same func with Hal Rpc */
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

    apcc_add_static_func_def(halClassInfo.createFunc, SPACES, ss);
    ss << endl;
    ss << endl;

    hih_add_callback_func(halClassInfo.callbackFunc, ss);
    hih_add_callback_func(halClassInfo.callback2Func, ss);
}

static void hih_add_private_func(const ApccGenInfo& genInfo, stringstream& ss)
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

static void hih_add_func_param(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalClassInfo& halClassInfo = genInfo.halClassInfo;

    if (!genInfo.aidlIntfReturnInfo.empty())
    {
        for (auto it: genInfo.aidlIntfReturnInfo)
        {
            const FuncParameter& halRpcInstListParam = it.second.halRpcInstListParam;
            apcc_add_variable(SPACES, halRpcInstListParam.type, halRpcInstListParam.name, ss);
        }
        ss << endl;
    }

    if (halClassInfo.existCallback)
    {
        apcc_add_func_param(halClassInfo.callbackVar, "", SPACES, ss);
        ss << endl;
    }
    if (halClassInfo.existCallback2)
    {
        apcc_add_func_param(halClassInfo.callbackVar2, "", SPACES, ss);
        ss << endl;
    }
    if (halClassInfo.existCallbackResult)
    {
        apcc_add_func_param(halClassInfo.callbackVarResult, "", SPACES, ss);
        ss << endl;
    }

    if (genInfo.isAidlIntfRoot)
    {
        apcc_add_func_param(halClassInfo.staticInst, CPP_STATIC " ", SPACES, ss);
        ss << endl;
    }

    ss << SPACES << DISALLOW_COPY_AND_ASSIGN_MACRO << '(' << genInfo.halClassInfo.className << ");" << endl;
}

/* ------------------------------------------------------------------------------------ */

static void hih_add_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    hih_add_hal_aidl_root_comment(genInfo, ss);

    hih_add_class_derivation(genInfo, ss);

    /* public: */
    ss << "  " << CPP_PUBLIC << ':' << endl;

    /* constructor: Hal(); */
    ss << SPACES << genInfo.halClassInfo.className << "();" << endl;
    if (genInfo.isAidlIntfRoot)
    {
        /* constructor: Hal(const char* instance_name); */
        ss << SPACES << genInfo.halClassInfo.className << '(' << HAL_INST_PARAM_TYPE << ' ' << HAL_INST_PARAM_NAME << ");" << endl;
    }
    /* destructor: ~Hal(); */
    ss << SPACES << '~' << genInfo.halClassInfo.className << "();" << endl;
    ss << endl;

    if (apcc_is_validate_func_supported())
    {
        /* bool isValid(); */
        ss << SPACES << CPP_BOOL << " " << IS_VALID_FUNC << ';' << endl;
        ss << endl;
    }

    /* public func */
    hih_add_public_func(genInfo, ss);

    /* private func */
    hih_add_private_func(genInfo, ss);

    /* func param */
    hih_add_func_param(genInfo, ss);

    ss << "};" << endl;
    ss << endl;
}

static void hih_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> usingType;

    apcc_get_all_using(genInfo, usingType);

    if (!usingType.empty())
    {
        apcc_add_using(usingType, ss);
        ss << endl;
    }

    if (genInfo.isAidlIntfRoot)
    {
        /* using std::string; */
        apcc_add_std_using(CPP_STRING, ss);
    }

    if (exist_aidl_intf_return_type(genInfo))
    {
        /* using std::map; */
        apcc_add_std_using(CPP_MAP, ss);
    }

    /* using std::pair; */
    apcc_add_std_using(CPP_PAIR, ss);
    ss << endl;

    apcc_add_intf_internal_using(genInfo, ss);
    apcc_add_callback_internal_using(genInfo, ss);
}

static void hih_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_sys_include_header(genInfo.halIntfFileInfo.halBnHeaderFile, ss);
    apcc_add_sys_include_header(ANDROID_BASE_MACROS_HEADER, ss);
    ss << endl;

    apcc_add_intf_include(genInfo, ss);

    if (genInfo.isAidlIntfRoot)
    {
        apcc_add_sys_include_header(CPP_STRING, ss);
        ss << endl;
    }

    if (exist_aidl_intf_return_type(genInfo))
    {
        apcc_add_sys_include_header(CPP_MAP, ss);
        ss << endl;
    }
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_impl_header(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. pragma */
    apcc_add_pragma(ss);

    /* 3. include */
    hih_add_include(genInfo, ss);

    /* 4. namespace prefix */
    apcc_add_namespace_prefix(genInfo.halNamespace, ss);

    /* 5. using */
    hih_add_using(genInfo, ss);

    /* 6. class definition */
    hih_add_class(genInfo, ss);

    /* 7. namespace prefix */
    apcc_add_namespace_suffix(genInfo.halNamespace, ss);
    FUNC_LEAVE();
}