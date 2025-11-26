/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "apcc_gen.h"

#include "log.h"
#include "util.h"

typedef struct
{
    FuncType apiFunc;
    FuncType internalFunc;
    FuncType commonFunc;
    string commonReturnType;
} HalFunc;

struct HalSourceInstance: HalBaseInstance
{
    vector<HalFunc> halFunc;
};

static HalSourceInstance *sHalSourceInstance = NULL;

static string get_valid_instance_name(const string& instanceName)
{
    stringstream ss;
    ss << instanceName << " ? " << instanceName << " : " << '"' << HAL_DEFAULT_INSTANCE << '"';
    return ss.str();
}

/* ------------------------------------------------------------------------------------ */

static void his_init_rpc_func(const FuncType& apiFunc, const FuncType& internalFunc,
    const FuncType& halCommonFunc, HalFunc& halFunc)
{
    halFunc.apiFunc = apiFunc;
    halFunc.internalFunc = internalFunc;
    halFunc.commonFunc = halCommonFunc;
    halFunc.commonReturnType = halCommonFunc.returnType;
}

static void his_init_rpc_func(const ApccGenInfo& genInfo, vector<HalFunc>& halFunc)
{
    for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcInternalFunc.size(); index++)
    {
        HalFunc rf;
        his_init_rpc_func(genInfo.halRpcFunc[index], genInfo.halRpcInternalFunc[index], genInfo.halCommonFunc[index], rf);
        halFunc.push_back(rf);
    }
}

static HalSourceInstance* his_create_instance()
{
    if (sHalSourceInstance != NULL)
    {
        delete sHalSourceInstance;
    }
    sHalSourceInstance = new HalSourceInstance;
    return sHalSourceInstance;
}

static void his_init_gen(const ApccGenInfo& genInfo)
{
    HalSourceInstance* inst = his_create_instance();
    his_init_rpc_func(genInfo, inst->halFunc);
}

static void his_add_static_variable(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalClassInfo& halClassInfo = genInfo.halClassInfo;

    if (genInfo.isAidlIntfRoot)
    {
        ss << halClassInfo.staticInst.type << ' ' << halClassInfo.className << "::" <<
            halClassInfo.staticInst.name << " = " << CPP_NULLPTR << ';' << endl;
        ss << endl;
    }
}

static void his_add_default_constructor(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.isAidlIntfRoot)
    {
        ss << genInfo.halClassInfo.className << "::" << genInfo.halClassInfo.className << "()" << endl;
        ss << SPACES << ": " << genInfo.halClassInfo.className << "(\"" << HAL_DEFAULT_INSTANCE << "\") {" << endl;
    }
    else
    {
        ss << genInfo.halClassInfo.className << "::" << genInfo.halClassInfo.className << "() {" << endl;
    }

    add_module_end(ss);
}

static void his_add_constructor(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalClassInfo& halClassInfo = genInfo.halClassInfo;

    if (genInfo.isAidlIntfRoot)
    {
        ss << endl;
        ss << halClassInfo.className << "::" << halClassInfo.className << '(' <<
            HAL_INST_PARAM_TYPE << " " << HAL_INST_PARAM_NAME <<") {" << endl;
        add_module_end(ss);
    }
}

static void his_add_destructor(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << endl;
    ss << genInfo.halClassInfo.className << "::" << '~' << genInfo.halClassInfo.className << "() {" << endl;
    add_module_end(ss);
}

static string his_check_ptr(const string& param)
{
    stringstream ss;
    ss << param << " == " << CPP_NULLPTR;
    return ss.str();
}

static void his_add_hal_rpc_instance(const string& prefix, const string& param,
    const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << prefix << CPP_SHARED_PTR << '<' << genInfo.halClassInfo.className << "> " <<
        genInfo.halClassInfo.classVar << " =" << endl;
    ss << prefix << SPACES << "ndk::SharedRefBase::make<" << genInfo.halClassInfo.className <<
        ">(" << param << ");" << endl;
}

static void his_add_aidl_intf_root_create_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalClassInfo& halClassInfo = genInfo.halClassInfo;
    const FuncType& createFunc = halClassInfo.createFunc;
    if (createFunc.param.empty())
        return;
    const string& instance_name = createFunc.param[0].name;

    ss << endl;
    apcc_add_class_func_impl(createFunc, halClassInfo.className, "", MOD_BEGIN_STR, ss);

    apcc_add_if_line(SPACES, his_check_ptr(genInfo.halClassInfo.staticInst.name), MOD_BEGIN_STR, ss);

    if (genInfo.halRpcServiceInfo.gen)
    {
        if (!genInfo.halRpcServiceInfo.func.empty())
        {
            apcc_add_class_func_call(genInfo.halRpcServiceInfo.className,
                genInfo.halRpcServiceInfo.func[0], SPACES_2, ss);
        }
    }

    his_add_hal_rpc_instance(SPACES_2, get_valid_instance_name(instance_name), genInfo, ss);

    ss << SPACES_2 << genInfo.halClassInfo.staticInst.name << " = " <<
        genInfo.halClassInfo.classVar << ";" << endl;

    add_module_end(SPACES, ss);
    apcc_add_return_line(SPACES, genInfo.halClassInfo.staticInst.name, ss);

    add_module_end(ss);
    ss << endl;
}

static void his_add_aidl_intf_create_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalClassInfo& halClassInfo = genInfo.halClassInfo;

    ss << endl;
    apcc_add_class_func_impl(halClassInfo.createFunc, halClassInfo.className, "", MOD_BEGIN_STR, ss);

    his_add_hal_rpc_instance(SPACES, "", genInfo, ss);

    ss << SPACES << CPP_RETURN << " " << halClassInfo.classVar << ';' << endl;

    add_module_end(ss);
    ss << endl;
}

static void his_add_create_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.isAidlIntfRoot)
    {
        his_add_aidl_intf_root_create_func(genInfo, ss);
    }
    else
    {
        his_add_aidl_intf_create_func(genInfo, ss);
    }
}

static void his_add_static_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. shared_ptr<BnHal> create(const char* instance_name) */
    his_add_create_func(genInfo, ss);
}

static void his_add_api_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.isAidlIntfRoot || genInfo.halApiFunc.empty())
        return;

    const FuncType& funcType = genInfo.halApiFunc[0];

    ss << endl;
    /* shared_ptr<IHal> create_hal(const char* instance_name) */
    apcc_add_func_impl(funcType, "", MOD_BEGIN_STR, ss);

    if (apcc_is_gen_hal_impl_stub_enabled())
    {
        ss << SPACES << CPP_RETURN << " ";
        ss << genInfo.halRpcNamespace[0];
        for (vector<string>::size_type index = 1; index < genInfo.halNamespace.size(); index++)
        {
            ss << "::" << genInfo.halNamespace[index];
        }
        ss << "::" << genInfo.halClassInfo.className << "::" << CREATE_NAME << '(' << HAL_INST_PARAM_NAME << ");" << endl;
    }
    else
    {
        ss << SPACES << COMMENT_TODO << endl;
        /* return nullptr; */
        apcc_add_return_line(SPACES, CPP_NULLPTR, ss);
    }

    add_module_end(ss);
}

static void his_add_func(const ApccGenInfo& genInfo, HalFunc& halFunc, stringstream& ss)
{
    apcc_add_class_func_impl(halFunc.apiFunc, genInfo.halClassInfo.className, "", MOD_BEGIN_STR, ss);

    apcc_add_return_func_call(halFunc.internalFunc.funcName, apcc_get_param_name(halFunc.apiFunc), SPACES, ss);

    add_module_end(ss);
    ss << endl;
}

static bool his_is_add_callback_required(const FuncType& funcType, const string& aidlCallbackName, string& aidlCallbackVar)
{
    for (auto param : funcType.param)
    {
        if (contain_string(param.type, aidlCallbackName))
        {
            aidlCallbackVar = param.name;
            return true;
        }
    }
    return false;
}

static void his_add_callback_calling(const ApccGenInfo& genInfo, HalFunc& halFunc, stringstream& ss)
{
    string aidlCallbackVar;

    if (genInfo.halRpcClassInfo.existCallback)
    {
        if (his_is_add_callback_required(halFunc.internalFunc, genInfo.aidlCallbackName, aidlCallbackVar))
        {
            apcc_add_func_call(ADD_HAL_CALLBACK_FUNC, aidlCallbackVar, SPACES, ss);
        }
    }
    if (genInfo.halRpcClassInfo.existCallback2)
    {
        if (his_is_add_callback_required(halFunc.internalFunc, genInfo.aidlCallback2Name, aidlCallbackVar))
        {
            apcc_add_func_call(ADD_HAL_CALLBACK2_FUNC, aidlCallbackVar, SPACES, ss);
        }
    }
    if (genInfo.halRpcClassInfo.existCallbackResult)
    {
        if (his_is_add_callback_required(halFunc.internalFunc, genInfo.aidlCallbackResultName, aidlCallbackVar))
        {
            apcc_add_func_call(ADD_HAL_CALLBACK_RESULT_FUNC, aidlCallbackVar, SPACES, ss);
        }
    }
}

static void his_add_internal_func(const ApccGenInfo& genInfo, HalFunc& halFunc, stringstream& ss)
{
    apcc_add_class_func_impl(halFunc.internalFunc, genInfo.halClassInfo.className, "", MOD_BEGIN_STR, ss);

    his_add_callback_calling(genInfo, halFunc, ss);

    ss << SPACES << COMMENT_TODO << endl;
    /* return status ok */
    apcc_add_return_line(SPACES, SCOPED_ASTATUS_OK, ss);

    add_module_end(ss);
    ss << endl;
}

static void his_add_callback_func(const FuncType& callbackFunc, const string& className,
    const FuncParameter& callbackVar, stringstream& ss)
{
    const string& callbackVarName = callbackVar.name;

    apcc_add_class_func_impl(callbackFunc, className, "", MOD_BEGIN_STR, ss);

    ss << SPACES << CPP_IF << " (" << callbackVarName << " != " << CPP_NULLPTR << ")" << endl;
    ss << SPACES_2 << callbackVarName << "->";
    apcc_add_func_call(callbackFunc, "", ";", ss);
    ss << endl;

    add_module_end(ss);
    ss << endl;
}

static void his_add_callback_func(const vector<FuncType>& callbackFunc, const string& className,
    const FuncParameter& callbackVar, stringstream& ss)
{
    if (!callbackFunc.empty())
    {
        for (auto it: callbackFunc)
        {
            his_add_callback_func(it, className, callbackVar, ss);
        }
    }
}

static void his_add_callback_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalClassInfo& halClassInfo = genInfo.halClassInfo;

    his_add_callback_func(halClassInfo.callbackFunc, halClassInfo.className, halClassInfo.callbackVar, ss);

    his_add_callback_func(halClassInfo.callback2Func, halClassInfo.className, halClassInfo.callbackVar2, ss);
}

static void his_add_util_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halRpcCallbackUtilInfo.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcCallbackUtilInfo.size(); index++)
        {
            const HalRpcCallbackUtilInfo& info = genInfo.halRpcCallbackUtilInfo[index];
            const FuncType& funcType = info.funcType;
            const string& funcName = funcType.funcName;
            apcc_add_class_func_impl(funcType, genInfo.halClassInfo.className, "", MOD_BEGIN_STR, ss);

            if (is_same_string(funcName, ADD_HAL_CALLBACK_FUNC) ||
                is_same_string(funcName, ADD_HAL_CALLBACK2_FUNC) ||
                is_same_string(funcName, ADD_HAL_CALLBACK_RESULT_FUNC))
            {
                /* callback_ = callback; */
                add_def_val_line(SPACES, info.aidlCallbackVar, info.funcType.param[0].name, ss);
            }

            add_module_end(ss);
            ss << endl;
        }
    }
}

static void his_add_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    HalSourceInstance* inst = sHalSourceInstance;

    /* 1. static variable */
    his_add_static_variable(genInfo, ss);

    /* 2. default constructor */
    his_add_default_constructor(genInfo, ss);

    /* 3. constructor */
    his_add_constructor(genInfo, ss);

    /* 4. destructor */
    his_add_destructor(genInfo, ss);

    /* 6. static func */
    his_add_static_func(genInfo, ss);

    for (vector<FuncType>::size_type index = 0; index < inst->halFunc.size(); index++)
    {
        /* 7. public func */
        his_add_func(genInfo, inst->halFunc[index], ss);
    }

    for (vector<FuncType>::size_type index = 0; index < inst->halFunc.size(); index++)
    {
        /* 8. internal func */
        his_add_internal_func(genInfo, inst->halFunc[index], ss);
    }

    /* 9. callback func */
    his_add_callback_func(genInfo, ss);

    /* 10. util func */
    his_add_util_func(genInfo, ss);
}

static void his_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
}

static void his_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* hal_api.h */
    if (genInfo.isAidlIntfRoot)
    {
        apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlIntfName, APCC_TYPE_HAL_API_HEADER), ss);
    }

    /* hal.h */
    if (apcc_is_gen_hal_impl_stub_enabled())
    {
        apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlIntfName, APCC_TYPE_HAL_IMPL_HEADER), ss);
        ss << endl;
    }
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_impl_source(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    his_init_gen(genInfo);

    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. include */
    his_add_include(genInfo, ss);

    if (apcc_is_gen_hal_impl_stub_enabled())
    {
        /* 3. namespace prefix */
        apcc_add_namespace_prefix(genInfo.halNamespace, ss);

        /* 4. using */
        his_add_using(genInfo, ss);

        /* 5. class implementation */
        his_add_class(genInfo, ss);

        /* 6. namespace prefix */
        apcc_add_namespace_suffix(genInfo.halNamespace, ss);
    }

    /* 7. func */
    his_add_api_func(genInfo, ss);
    FUNC_LEAVE();
}