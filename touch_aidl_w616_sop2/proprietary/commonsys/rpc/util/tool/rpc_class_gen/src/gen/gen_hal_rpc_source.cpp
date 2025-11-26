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

#define COMMENT_HAL_STATUS              "/* create hal status for cfm */"

typedef struct
{
    FuncType apiFunc;
    FuncType internalFunc;
    FuncType halReqFunc;
    FuncType commonFunc;
    string commonReturnType;
    string halReqName;
    string halCfmName;
    string validateFuncName;
} HalRpcFunc;

struct HalRpcSourceInstance: HalBaseInstance
{
    vector<HalRpcFunc> rpcFunc;
    bool addDefaultHalStatus;
    bool existValidUsing;
    bool verifyPtr;
};

static HalRpcSourceInstance *sHalRpcSourceInstance = NULL;

static string cpp_reinterpret_ptr(const string& cppType, const string& paramName)
{
    stringstream ss;
    /* e.g. reinterpret_cast<T*>(halStatus) */
    ss << CPP_REINTERPRET_CAST << '<' << cpp_type_ptr(cppType) << ">(" << paramName << ")";
    return ss.str();
}

static string cpp_ref(const string& cppType, const string& paramName)
{
    stringstream ss;
    /* e.g. *(reinterpret_cast<T*>(halStatus)) */
    ss << CPP_POINTER << '(' << cpp_reinterpret_ptr(cppType, paramName) << ')';
    return ss.str();
}

static string get_struct_field(const string& varName, const string& fieldName)
{
    stringstream ss;
    ss << varName << '.' << fieldName;
    return ss.str();
}

static bool is_validate_and_call_with_lock_func(const string& validateFuncName)
{
    return is_same_string(validateFuncName, VALIDATE_AND_CALL_WITH_LOCK_FUNC);
}

static bool exist_return_type(const string& returnType)
{
    return !is_same_string(returnType, CPP_VOID);
}

static bool exist_aidl_return_param(const FuncType& funcType)
{
    if (!funcType.param.empty())
    {
        /* check last param */
        const FuncParameter& param = funcType.param[funcType.param.size() - 1];
        if (is_same_string(param.name, AIDL_RETURN_NAME))
            return true;
    }
    return false;
}

static string get_callback_result_param_name(const string& halStatusCode,
    const string& statusParam, const string& aidlReturnParam)
{
    /* (SupplicantStatusCode)status.getStatus(), "", _aidl_return */
    stringstream ss;
    ss << '(' << halStatusCode << ')' << statusParam << '.' << "getStatus(), \"\", " << aidlReturnParam;
    return ss.str();
}

static string get_validate_func_align_str(const string& validateFuncName)
{
    stringstream ss;
    ss << SPACES << CPP_RETURN << " " << validateFuncName << '(';
    string str = ss.str();
    str.replace(0, str.length(), str.length(), ' ');
    return str;
}

static bool exist_valid_using(const ApccGenInfo& genInfo)
{
    for (auto it: genInfo.halAidlIntfInfo)
    {
        if (valid_str(it.usingType))
            return true;
    }
    return false;
}

static bool is_add_callback_required(const FuncType& funcType, const string& aidlCallbackName, string& aidlCallbackVar)
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

static void add_var_def_line(const string& prefix, const string& paramType, const string& paramName, stringstream& ss)
{
    ss << prefix << paramType << " " << paramName << ';' << endl;
}

static void add_message_type_var_line(const string& halMsgName, const string& prefix, stringstream& ss)
{
    ss << prefix << CPP_UINT16 << " " << MESSAGE_TYPE_NAME << " = " << halMsgName << ";" << endl;
}

static void add_payload_var_line(const string& prefix, stringstream& ss)
{
    add_var_def_line(prefix, CPP_BYTE_VECTOR, PAYLOAD_NAME, ss);
}

static void add_session_id_var_line(const string& prefix, stringstream& ss)
{
    if (apcc_is_aidl_sync_api())
        ss << prefix << CPP_UINT16 << " " << SESSION_ID_NAME << " = 0;" << endl;
}

static void add_instance_id_var_line(const ApccGenInfo& genInfo, HalRpcFunc& rpcFunc,
    const string& prefix, stringstream& ss)
{
    bool existAidlIntfParam = false;
    for (auto it: rpcFunc.commonFunc.param)
    {
        if (aidl_is_interface(it.type) &&
            !aidl_is_callback(it.type))
        {
            existAidlIntfParam = true;
            break;
        }
    }

    if (existAidlIntfParam)
    {
        ss << prefix << INSTANCE_ID_TYPE << " " << INSTANCE_ID_NAME << " = 0;" << endl;
    }
}

static bool is_clear_callback_required(const ApccGenInfo& genInfo, const FuncType& funcType)
{
    /* TODO */
    return false;
}

static void add_wait_hal_status_line(const string& halMsgName, const string& halStatusName,
    const string& prefix, stringstream& ss)
{
    vector<string> paramName;
    paramName.push_back(halMsgName);
    paramName.push_back(SESSION_ID_NAME);
    paramName.push_back(cpp_type_ref_ptr(halStatusName));

    ss << prefix << CPP_IF << " (";
    apcc_add_func_call(WAIT_HAL_STATUS_FUNC, paramName, "", "", ss);
    ss << ") {" << endl;
}

static void add_remove_hal_status_line(const string& halMsgName, const string& prefix, stringstream& ss)
{
    vector<string> paramName;
    paramName.push_back(halMsgName);
    paramName.push_back(SESSION_ID_NAME);

    apcc_add_func_call(REMOVE_HAL_STATUS_FUNC, paramName, prefix, ";", ss);
    ss << endl;
}

static void add_hal_status_var_line(const string& prefix, stringstream& ss)
{
    ss << prefix << CPP_VOID_POINTER << " " << HAL_STATUS_VAR << " = NULL;" << endl;
}

static string get_if_line_prefix(const string& prefix, bool negative = true)
{
    stringstream ss;
    ss << prefix << CPP_IF << " (";
    if (negative)
    {
        ss << "!";
    }
    return ss.str();
}

static string get_if_line_suffix()
{
    return ")";
}

static void add_hal_msg_func_call(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    apcc_add_func_call(funcType, get_if_line_prefix(prefix), get_if_line_suffix(), ss);
    ss << endl;
}

static void add_send_msg_line(const string& halMsgName, const string& prefix, const string& suffix, stringstream& ss)
{
    ss << prefix << HAL_RPC_CLIENT_SEND_FUNC << '(' << halMsgName << ", " << PAYLOAD_NAME;
    if (apcc_is_aidl_sync_api())
    {
        ss << ", " << cpp_type_ref_ptr(SESSION_ID_NAME);
    }
    ss << ')' << suffix << endl;
}

static string get_hal_result_type(const ApccGenInfo& genInfo, const string& halMsgName)
{
    for (auto it: genInfo.halResultInfo)
    {
        if (is_same_string(it.halCfm, halMsgName))
            return it.halResultName;
    }
    return "";
}

static string get_hal_status_param(const string& varName)
{
    stringstream ss;
    ss << get_struct_field(varName, STATUS_NAME) << ", " << get_struct_field(varName, INFO_NAME);
    return ss.str();
}

static void add_hal_result_line(const string& prefix, const string& paramName,
    const string& halResultType, const string& halStatusName, stringstream& ss)
{
    /* e.g. T& res = *(reinterpret_cast<T*>(halStatus)); */
    ss << prefix << cpp_type_ref(halResultType) << " " << paramName << " = " <<
        cpp_ref(halResultType, halStatusName) << ';' << endl;
}

static void add_new_hal_result_line(const string& prefix, const string& paramName,
    const string& halResultType, stringstream& ss)
{
    /* e.g. res = new T; */
    ss << prefix << paramName << " = " << CPP_NEW << " " << halResultType << ';' << endl;
}

static void add_output_param_line(const string& prefix, const string& paramName,
    const string& resultVarName, uint32_t fieldIndex, stringstream& ss)
{
    ss << prefix << cpp_type_deref(paramName) << " = " <<
        resultVarName << '.' << paramName << ';' << endl;
}

static void add_cfm_param_assignment_line(const string& prefix, const string& paramType,
    const string& paramName, const string& resultVarName, const string& fieldName, stringstream& ss)
{
    ss << prefix << paramType << " " << paramName << " = " <<
        get_struct_field(resultVarName, fieldName) << ';' << endl;
}

static void add_cfm_param_assignment_line(const string& prefix, const string& paramName,
    const string& resultVarName, const string& fieldName, stringstream& ss)
{
    ss << prefix << paramName << " = " <<
        get_struct_field(resultVarName, fieldName) << ';' << endl;
}

static void add_copy_hal_result_line(const string& prefix, const string& srcParam,
    const string& dstParamPtr, stringstream& ss)
{
    ss << prefix << cpp_type_deref(dstParamPtr) << " = " << srcParam << ';' << endl;
}

static void add_update_hal_status_line(const string& prefix, const string& halStatusVar,
    const string& res, stringstream& ss)
{
    ss << prefix << cpp_type_deref(halStatusVar) << " = " << res << ";" << endl;
}

static void add_return_scoped_astatus_line(const string& prefix, const string& varName, stringstream& ss)
{
    string paramName = get_hal_status_param(varName);

    ss << prefix << CPP_RETURN << " ";
    apcc_add_func_call(CREATE_SCOPED_ASTATUS_FUNC, paramName, "", ss);
}

void add_delete_line(const string& prefix, const string& typeName, const string& varName, stringstream& ss)
{
    /* e.g. delete (reinterpret_cast<T*>(halStatus)); */
    ss << prefix << CPP_DELETE << " (" << cpp_reinterpret_ptr(typeName, varName) << ");" << endl;
}

static string get_hal_rpc_class_name(const ApccGenInfo& genInfo, const string& aidlIntfName)
{
    for (auto it: genInfo.halAidlIntfInfo)
    {
        if (is_same_string(it.intfName, aidlIntfName))
            return it.className;
    }
    return "";
}

static string get_hal_rpc_class_create_func(const ApccGenInfo& genInfo, const string& aidlIntfName)
{
    stringstream ss;
    ss << get_hal_rpc_class_name(genInfo, aidlIntfName) << "::" << CREATE_NAME;
    return ss.str();
}

static void add_create_hal_rpc_class_func(const ApccGenInfo& genInfo, const string& aidlIntfName,
    const string& prefix, const string& paramName, const string& instanceId, stringstream& ss)
{
    string newPrefix = concat_string(prefix, SPACES);
    const AidlIntfReturnInfo& aidlIntfReturnInfo = apcc_get_aidl_intf_return_info(genInfo, aidlIntfName);
    const FuncParameter& halRpcInstListParam = aidlIntfReturnInfo.halRpcInstListParam;
    const FuncParameter& halRpcParam = aidlIntfReturnInfo.param;

    if (!valid_str(aidlIntfReturnInfo.intfName))
        return;

    /* if (!HalRpc::existInstance(hal_, instanceId, _aidl_return)) { */
    ss << prefix << CPP_IF << " (!" << HAL_RPC_EXIST_INSTANCE_FUNC << '(' <<
        halRpcInstListParam.name << ", " << instanceId << ", " << paramName <<
        "))" << MOD_BEGIN_STR << endl;

    /* shared_ptr<IHal> hal = HalRpc::create(instanceId); */
    ss << newPrefix << halRpcParam.type << " " << halRpcParam.name << " = ";
    apcc_add_func_call(get_hal_rpc_class_create_func(genInfo, aidlIntfName), instanceId, "", ss);

    /* hal_.insert(pair<shared_ptr<IHal>, uint16_t>(hal, instanceId)); */
    ss << newPrefix << halRpcInstListParam.name << ".insert(" << get_pair_type_def(
        halRpcParam.type, INSTANCE_ID_TYPE, halRpcParam.name, instanceId) << ");" << endl;

    /* *_aidl_return = hal; */
    ss << newPrefix << cpp_type_deref(paramName) << " = " << halRpcParam.name << ";" << endl;

    add_module_end(prefix, ss);
}

static void add_hal_util_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.isValidateFuncSupported)
    {
        /* 1. aidl_return_util.h */
        apcc_add_include_header(HAL_AIDL_RETURN_UTIL_HEADER, ss);
        /* 2. aidl_sync_util.h */
        apcc_add_include_header(HAL_AIDL_SYNC_UTIL_HEADER, ss);
    }
    /* 3. hal_status_util.h */
    if (genInfo.halStatusInfo.gen)
    {
        const string& aidlIntfRoot = aidl_get_intf_root();
        apcc_add_include_header(apcc_get_gen_file_name(aidlIntfRoot, APCC_TYPE_HAL_STATUS_UTIL_HEADER), ss);
    }
    ss << endl;
}

static string get_hal_util_using(const ApccGenInfo& genInfo, const string& usingType)
{
    stringstream ss;
    for (auto it: genInfo.halRpcNamespace)
    {
        ss << it << "::";
    }
    ss << usingType;
    return ss.str();
}

static void add_hal_util_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.isValidateFuncSupported)
    {
        /* 1. aidl_return_util::validateAndCall */
        apcc_add_using(get_hal_util_using(genInfo, HAL_USING_VALIDATE_AND_CALL), ss);
        /* 2. aidl_return_util::validateAndCallWithLock */
        apcc_add_using(get_hal_util_using(genInfo, HAL_USING_VALIDATE_AND_CALL_WITH_LOCK), ss);
        /* 3. aidl_sync_util::acquireGlobalLock */
        apcc_add_using(get_hal_util_using(genInfo, HAL_USING_ACQUIRE_GLOBAL_LOCK), ss);
        ss << endl;
    }
}

static string get_valid_instance_name(const string& instanceName)
{
    stringstream ss;
    ss << instanceName << " ? " << instanceName << " : " << '"' << HAL_DEFAULT_INSTANCE << '"';
    return ss.str();
}

static void get_hal_msg_param(const vector<FuncParameter>& param, vector<string>& paramName)
{
    paramName.clear();
    if (param.size() >= 3)
    {
        /* 1. param: data */
        paramName.push_back(param[1].name);
        /* 2. param: length */
        paramName.push_back(param[2].name);
        /* 3. param: result */
        if (param.size() >= 4)
            paramName.push_back(param[3].name);
    }

}

static string check_param(const string& param)
{
    stringstream ss;
    ss << '!' << param;
    return ss.str();
}

static string check_param(const string& param1, const string& param2)
{
    stringstream ss;
    ss << '!' << param1 << " || " << '!' << param2;
    return ss.str();
}

static string get_hal_result_type_ref(const string& halResultPtrVar, const string& halResultType)
{
    stringstream ss;
    /* e.g. *((T*)res) */
    ss << CPP_POINTER << "((" << cpp_type_ptr(halResultType) << ")" << " " << halResultPtrVar << ")";
    return ss.str();
}

static void get_hal_cfm_param(const string& data, const string& length,
    const string& res, const string& halResultType, vector<string>& param)
{
    param.clear();
    param.push_back(data);
    param.push_back(length);
    param.push_back(get_hal_result_type_ref(res, halResultType));
}

static void get_hal_cfm_param(const string& data, const string& length, const string& res,
    const string& halResultType, const FuncType& funcType, vector<string>& param)
{
    std::size_t paramSize = funcType.param.size();
    get_hal_cfm_param(data, length, halResultType, res, param);
    if (paramSize >= 4)
    {
        /* TODO */
    }
}

static string get_hal_upstream_req_parse_param(const string& data, const string& length,
    const FuncType& aidlCallbackResultFunc)
{
    stringstream ss;
    ss << data << ", " << length;
    for (auto it: aidlCallbackResultFunc.param)
    {
        ss << ", " << it.name;
    }
    return ss.str();
}

static string get_hal_downstream_cfm_parse_param(const string& statusName,
    const string& aidlReturnName, const string& payloadName)
{
    stringstream ss;
    /* TODO: Add HalStatus */
    // ss << statusName << ".getStatus(), " << statusName << ".getMessage()" << ", ";
    ss << aidlReturnName << ", ";
    ss << payloadName;
    return ss.str();
}

static string get_hal_downstream_cfm_parse_param(const string& statusName, const string& payloadName)
{
    stringstream ss;
    ss << statusName << ".getStatus(), " << statusName << ".getMessage()";
    ss << ", " << payloadName;
    return ss.str();
}

static void add_var_def(const string& prefix, const string& varName, const string& defVal, stringstream& ss)
{
    ss << prefix << varName << " = " << defVal << ";" << endl;
}

/* ------------------------------------------------------------------------------------ */

static void hrs_init_rpc_func(bool isValidateFuncSupported, const FuncType& apiFunc, const FuncType& internalFunc,
    const FuncType& halReqFunc, const FuncType& halCommonFunc, const string& halReqName, const string& halCfmName, HalRpcFunc& rpcFunc)
{
    string validateFuncName = "";
    const string& cppTypeName = halCommonFunc.returnType;

    rpcFunc.apiFunc = apiFunc;
    rpcFunc.internalFunc = internalFunc;
    rpcFunc.halReqFunc = halReqFunc;
    rpcFunc.commonFunc = halCommonFunc;
    rpcFunc.commonReturnType = cppTypeName;
    // logd("%s: commonReturnType: %s", __func__, cppTypeName.c_str());
    rpcFunc.halReqName = halReqName;
    rpcFunc.halCfmName = halCfmName;

    if (isValidateFuncSupported)
    {
        validateFuncName = apcc_is_validate_func_with_lock(apiFunc.funcName) ?
                            VALIDATE_AND_CALL_WITH_LOCK_FUNC :
                            (apcc_is_cpp_type(cppTypeName, CPP_VOID) ?
                            VALIDATE_AND_CALL_FUNC :
                            (apcc_is_basic_type(cppTypeName) ?
                             "" :
                             VALIDATE_AND_CALL_FUNC));
    }
    rpcFunc.validateFuncName = validateFuncName;
}

static void hrs_init_rpc_func(const ApccGenInfo& genInfo, vector<HalRpcFunc>& rpcFunc)
{
    bool isCfmDefined = genInfo.isCfmDefined;
    for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcInternalFunc.size(); index++)
    {
        HalRpcFunc rf;
        hrs_init_rpc_func(genInfo.isValidateFuncSupported, genInfo.halRpcFunc[index], genInfo.halRpcInternalFunc[index],
            genInfo.halReqFunc[index], genInfo.halCommonFunc[index], genInfo.halReqDef[index],
            isCfmDefined ? genInfo.halCfmDef[index] : genInfo.halReqDef[index], rf);
        rpcFunc.push_back(rf);
    }
}

static HalRpcSourceInstance* hrs_create_instance()
{
    if (sHalRpcSourceInstance != NULL)
    {
        delete sHalRpcSourceInstance;
    }
    sHalRpcSourceInstance = new HalRpcSourceInstance;
    return sHalRpcSourceInstance;
}

static void hrs_init_gen(const ApccGenInfo& genInfo)
{
    HalRpcSourceInstance* inst = hrs_create_instance();
    hrs_init_rpc_func(genInfo, inst->rpcFunc);
    inst->addDefaultHalStatus = !((genInfo.halResultInfo.size() == 1) &&
                                (genInfo.halCfmDef.size() == 1)) &&
                                genInfo.isAidlSyncApi;
    inst->existValidUsing = exist_valid_using(genInfo);
    inst->verifyPtr = false;
    // logd("%s: addDefaultHalStatus: %x, halResultInfo size: %d, genInfo.halCfmDef: %d", __func__,
    //     inst->addDefaultHalStatus, genInfo.halResultInfo.size(), genInfo.halCfmDef.size());
}

static void hrs_add_static_variable(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalRpcClassInfo& halRpcClassInfo = genInfo.halRpcClassInfo;

    if (genInfo.isAidlIntfRoot)
    {
        ss << halRpcClassInfo.staticInst.type << ' ' << halRpcClassInfo.className << "::" <<
            halRpcClassInfo.staticInst.name << " = " << CPP_NULLPTR << ';' << endl;

        if (genInfo.isSingleSomeip)
        {
            ss << halRpcClassInfo.staticProxy.type << ' ' << halRpcClassInfo.className << "::" <<
                halRpcClassInfo.staticProxy.name << " = " << CPP_NULLPTR << ';' << endl;
        }
        ss << endl;
    }
}

static void hrs_add_default_constructor(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << genInfo.halRpcClassInfo.className << "::" << genInfo.halRpcClassInfo.className << "()" << endl;
    if (genInfo.isAidlIntfRoot)
        ss << SPACES << ": " << genInfo.halRpcClassInfo.className << "(\"" << HAL_DEFAULT_INSTANCE << "\") {" << endl;
    else
        ss << SPACES << ": " << genInfo.halRpcClassInfo.className << "(" << HAL_DEFAULT_INSTANCE_ID << ") {" << endl;

    add_module_end(ss);
}

static void hrs_add_constructor(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalRpcClassInfo& halRpcClassInfo = genInfo.halRpcClassInfo;
    const string& paramType = genInfo.isAidlIntfRoot ? HAL_INST_PARAM_TYPE : INSTANCE_ID_TYPE;
    const string& paramName = genInfo.isAidlIntfRoot ? HAL_INST_PARAM_NAME : INSTANCE_ID_NAME;

    ss << endl;
    ss << halRpcClassInfo.className << "::" << halRpcClassInfo.className << '(' <<
        paramType << " " << paramName <<")" << endl;
    ss << SPACES << ": " << HAL_RPC_CLIENT_CLASS << '(' << paramName;
    if (!apcc_is_aidl_sync_api())
    {
        ss << ", " << CPP_FALSE;
    }
    add_module_begin(") ", ss);
    add_module_end(ss);
}

static void hrs_add_destructor(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << endl;
    ss << genInfo.halRpcClassInfo.className << "::" << '~' << genInfo.halRpcClassInfo.className << "() {" << endl;
    if (genInfo.halRpcServiceInfo.gen)
    {
        if (genInfo.halRpcServiceInfo.func.size() >= 2)
        {
            apcc_add_class_func_call(genInfo.halRpcServiceInfo.className,
                genInfo.halRpcServiceInfo.func[1], SPACES, ss);
        }
    }
    if (genInfo.isAidlIntfRoot || !genInfo.isSingleSomeip)
    {
        apcc_add_func_call_no_param(HAL_RPC_CLOSE_SOMEIP_FUNC, SPACES, ss);
    }
    add_module_end(ss);
}

static void hrs_add_is_valid_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << endl;
    ss << CPP_BOOL << ' ' << genInfo.halRpcClassInfo.className << "::" << IS_VALID_FUNC << MOD_BEGIN_STR << endl;
    ss << SPACES << "return true;" << endl;
    add_module_end(ss);
}

static void hrs_add_is_rpc_enabled_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.isAidlIntfRoot)
    {
        ss << SPACES << "char value[PROPERTY_VALUE_MAX] = { '\\0' };" << endl;
        ss << SPACES << "property_get(\"" << genInfo.halRpcClassInfo.halModePropertyName << "\", value, \"\");" << endl;
        ss << SPACES << CPP_RETURN << " !strcmp(value, \"" << HAL_MODE_PROPERTY_VALUE_RPC << "\");" << endl;
    }
    else
    {
        ss << SPACES << CPP_RETURN << " " << CPP_TRUE;
    }
}

static void hrs_add_get_static_proxy_func(const HalRpcClassInfo& halRpcClassInfo, stringstream& ss)
{
    ss << SPACES << CPP_RETURN << " " << halRpcClassInfo.staticProxy.name << ';' << endl;
}

static string hrs_check_ptr(const string& param)
{
    stringstream ss;
    ss << param << " == " << CPP_NULLPTR;
    return ss.str();
}

static void hrs_add_hal_rpc_instance(const string& prefix, const string& param,
    const ApccGenInfo& genInfo, stringstream& ss)
{
    ss << prefix << CPP_SHARED_PTR << '<' << genInfo.halRpcClassInfo.className << "> " <<
        genInfo.halRpcClassInfo.classVar << " =" << endl;
    ss << prefix << SPACES << "ndk::SharedRefBase::make<" << genInfo.halRpcClassInfo.className <<
        ">(" << param << ");" << endl;

    apcc_add_class_func_call_no_param(genInfo.halRpcClassInfo.classVar, INIT_FUNC, prefix, ss);

    if (genInfo.isAidlIntfRoot || !genInfo.isSingleSomeip)
    {
        apcc_add_class_func_call_no_param(genInfo.halRpcClassInfo.classVar, OPEN_SOMEIP_NAME, prefix, ss);
    }
}

static void hrs_add_aidl_intf_root_create_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    if (funcType.param.empty())
        return;
    const string& instance_name = funcType.param[0].name;

    apcc_add_if_line(SPACES, concat_string("!", CALL_IS_RPC_ENABLED_FUNC), ss);
    apcc_add_return_line(SPACES_2, CPP_NULLPTR, ss);
    ss << endl;
    apcc_add_if_line(SPACES, hrs_check_ptr(genInfo.halRpcClassInfo.staticInst.name), MOD_BEGIN_STR, ss);

    if (genInfo.halRpcServiceInfo.gen)
    {
        if (!genInfo.halRpcServiceInfo.func.empty())
        {
            apcc_add_class_func_call(genInfo.halRpcServiceInfo.className,
                genInfo.halRpcServiceInfo.func[0], SPACES_2, ss);
        }
    }

    hrs_add_hal_rpc_instance(SPACES_2, get_valid_instance_name(instance_name), genInfo, ss);

    ss << SPACES_2 << genInfo.halRpcClassInfo.staticInst.name << " = " <<
        genInfo.halRpcClassInfo.classVar << ";" << endl;

    add_module_end(SPACES, ss);
    apcc_add_return_line(SPACES, genInfo.halRpcClassInfo.staticInst.name, ss);
}

static void hrs_add_aidl_intf_create_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    if (funcType.param.empty())
        return;
    const string& param_name = funcType.param[0].name;

    hrs_add_hal_rpc_instance(SPACES, param_name, genInfo, ss);

    ss << SPACES << CPP_RETURN << " " << genInfo.halRpcClassInfo.classVar << ';' << endl;
}

static void hrs_add_create_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    if (genInfo.isAidlIntfRoot)
    {
        hrs_add_aidl_intf_root_create_func(genInfo, funcType, ss);
    }
    else
    {
        hrs_add_aidl_intf_create_func(genInfo, funcType, ss);
    }
}

static void hrs_add_delete_hal_status_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    HalRpcSourceInstance* inst = sHalRpcSourceInstance;

    apcc_add_switch_line(funcType.param[0].name, SPACES, MOD_BEGIN_STR, ss);
    if (!genInfo.halResultInfo.empty())
    {
        for (vector<HalResultInfo>::size_type index = 0; index < genInfo.halResultInfo.size(); index++)
        {
            HalResultInfo hri = genInfo.halResultInfo[index];
            apcc_add_case_line(hri.halCfm, SPACES_2, MOD_BEGIN_STR, ss);
            add_delete_line(SPACES_3, hri.halResultName, HAL_STATUS_VAR, ss);
            apcc_add_break_line(SPACES_3, ss);
            add_module_end(SPACES_2, ss);
        }
    }
    /* default */
    apcc_add_default_line(SPACES_2, MOD_BEGIN_STR, ss);
    if (inst->addDefaultHalStatus &&
        (genInfo.halResultInfo.size() < genInfo.halCfmDef.size()))
    {
        add_delete_line(SPACES_3, HAL_STATUS_PARAM_TYPE, HAL_STATUS_VAR, ss);
    }
    apcc_add_break_line(SPACES_3, ss);
    add_module_end(SPACES_2, ss);

    add_module_end(SPACES, ss);
}

static void hrs_add_static_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalRpcClassInfo& halRpcClassInfo = genInfo.halRpcClassInfo;

    ss << endl;
    for (auto it: genInfo.halRpcStaticFunc)
    {
        const string& funcName = it.funcName;

        apcc_add_class_func_impl(it, halRpcClassInfo.className, "", MOD_BEGIN_STR, ss);

        if (is_same_string(funcName, IS_RPC_ENABLED_FUNC))
             hrs_add_is_rpc_enabled_func(genInfo, ss);
        else if (is_same_string(funcName, GET_STATIC_PROXY_NAME))
             hrs_add_get_static_proxy_func(halRpcClassInfo, ss);
        else if (is_same_string(funcName, CREATE_NAME))
             hrs_add_create_func(genInfo, it, ss);
        else if (is_same_string(funcName, DELETE_HAL_STATUS_FUNC))
            hrs_add_delete_hal_status_func(genInfo, it, ss);

        add_module_end(ss);
        ss << endl;
    }
}

static void hrs_add_api_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.isAidlIntfRoot || genInfo.halApiFunc.empty())
        return;

    const FuncType& funcType = genInfo.halApiFunc[0];

    ss << endl;
    /* shared_ptr<IHal> create_hal_rpc(const char* instance_name) */
    apcc_add_func_impl(funcType, "", MOD_BEGIN_STR, ss);

    ss << SPACES << CPP_RETURN << " ";
    ss << genInfo.halRpcNamespace[0];
    for (vector<string>::size_type index = 1; index < genInfo.halRpcNamespace.size(); index++)
    {
        ss << "::" << genInfo.halRpcNamespace[index];
    }
    ss << "::" << genInfo.halRpcClassInfo.className << "::" << CREATE_NAME << '(' << HAL_INST_PARAM_NAME << ");" << endl;

    add_module_end(ss);
}

static void hrs_add_validate_func(const ApccGenInfo& genInfo, HalRpcFunc& rpcFunc, stringstream& ss)
{
    ss << SPACES << CPP_RETURN << " " << rpcFunc.validateFuncName << "(this, " <<
        genInfo.halStatusInfo.aidlHalStatusType << "::" << genInfo.halStatusInfo.defaultHalStatusCode << ',' << endl;

    ss << get_validate_func_align_str(rpcFunc.validateFuncName) << "&" <<
        genInfo.halRpcClassInfo.className << "::" << rpcFunc.internalFunc.funcName;

    if (exist_aidl_return_param(rpcFunc.apiFunc))
    {
        const vector<FuncParameter>& param = rpcFunc.apiFunc.param;
        // firstly output the last param of aidl return
        ss << ", " << AIDL_RETURN_NAME;
        for (vector<FuncParameter>::size_type index = 0; index < param.size() - 1; index++)
        {
            ss << ", " << param[index].name;
        }
    }
    else
    {
        for (auto it: rpcFunc.apiFunc.param)
        {
            ss << ", " << it.name;
        }
    }

    ss << ");" << endl;
}

static void hrs_add_func(const ApccGenInfo& genInfo, HalRpcFunc& rpcFunc, stringstream& ss)
{
    apcc_add_class_func_impl(rpcFunc.apiFunc, genInfo.halRpcClassInfo.className, "", MOD_BEGIN_STR, ss);
    if (valid_str(rpcFunc.validateFuncName))
    {
        hrs_add_validate_func(genInfo, rpcFunc, ss);
    }
    else
    {
        apcc_add_return_func_call(rpcFunc.internalFunc.funcName, apcc_get_param_name(rpcFunc.apiFunc), SPACES, ss);
    }
    add_module_end(ss);
    ss << endl;
}

static string hrs_get_internal_param_name(const FuncType& funcType, const string& validateFuncName, bool addResultPtr)
{
    stringstream ss;
    vector<FuncParameter>::size_type size = funcType.param.size();
    if (size >= 1)
    {
        if (!is_validate_and_call_with_lock_func(validateFuncName))
            ss << funcType.param[0].name;

        if (size >= 2)
        {
            for (vector<FuncParameter>::size_type index = 1; index < size; index++)
            {
                if (!is_validate_and_call_with_lock_func(validateFuncName))
                    ss << ", " << funcType.param[index].name;
            }
        }

        if (addResultPtr)
        {
            ss << ", &" << RESULT_NAME;
        }
    }
    else
    {
        if (addResultPtr)
        {
            ss << "&" << RESULT_NAME;
        }
    }
    return ss.str();
}

static void hrs_add_internal_result_var(string& resultType, stringstream& ss)
{
    /* e.g. std::shared_ptr<IHalMod> result = nullptr; */
    ss << SPACES << resultType << " " << RESULT_NAME;
    if (apcc_is_shared_ptr_type(resultType))
    {
        ss << " = nullptr";
    }
    ss << ";" << endl;
}

static string hrs_get_return_pair(const string& resultType)
{
    stringstream ss;
    if (is_same_string(resultType, CPP_STRING) ||
        apcc_is_basic_type(resultType) ||
        apcc_is_enum_type(resultType) ||
        apcc_is_shared_ptr_type(resultType))
    {
        ss << RESULT_NAME;
    }
    else
    {
        ss << STD_NAMESPACE << "::" << STD_MOVE_FUNC << '(' << RESULT_NAME << ')';
    }
    return ss.str();
}

static void hrs_add_internal_return_pair(const ApccGenInfo& genInfo, const string& resultType,
    const string& prefix, stringstream& ss)
{
    const HalStatusInfo& halStatusInfo = genInfo.halStatusInfo;
    /* return pair */
    ss << prefix << CPP_RETURN << MOD_BEGIN_STR << hrs_get_return_pair(resultType) << ", ";
    if (halStatusInfo.existHalStatusType &&
        halStatusInfo.existCreateHalStatusFunc)
    {
        ss << halStatusInfo.createHalStatusFunc[0].funcName << '(' <<
            apcc_create_hal_status(halStatusInfo.aidlHalStatusType, halStatusInfo.defaultHalStatusCode) << ')';
    }
    else
    {
        ss << apcc_create_hal_scoped_astatus(halStatusInfo.aidlHalStatusType, STATUS_ERROR_UNKNOWN_CODE);
    }
    ss << "};" << endl;
}

static void hrs_add_internal_status_ok_module(const ApccGenInfo& genInfo, HalRpcFunc& rpcFunc,
    const string& resultType, stringstream& ss)
{
    string resultParam = RES_NAME;
    bool existReturnType = !apcc_is_void_type(rpcFunc.commonReturnType);

    if (existReturnType)
    {
        const vector<FuncParameter>& param = rpcFunc.internalFunc.param;
        uint32_t outParamTupleIndex = 3;    /* 0: int32_t status, 1: string info,  2: T _aidl_return */

        add_hal_result_line(SPACES_2, resultParam,
            get_hal_result_type(genInfo, rpcFunc.halCfmName), HAL_STATUS_VAR, ss);

        /* add out param */
        for (auto it: param)
        {
            if (apcc_is_out_param_type(it.type, it.name))
            {
                add_output_param_line(SPACES_2, it.name, resultParam, outParamTupleIndex, ss);
                ++ outParamTupleIndex;
            }
        }

        if (aidl_is_interface(rpcFunc.commonReturnType))
        {
            /* uint16_t instanceId = res.result; */
            add_cfm_param_assignment_line(SPACES_2, CPP_UINT16, INSTANCE_ID_NAME, resultParam, RESULT_NAME, ss);
            /* *_aidl_return = T::create(instanceId); */
            add_create_hal_rpc_class_func(genInfo, rpcFunc.commonReturnType, SPACES_2,
                AIDL_RETURN_NAME, INSTANCE_ID_NAME, ss);
        }
        else
        {
            /* *_aidl_return = res.result; */
            add_cfm_param_assignment_line(SPACES_2, cpp_type_deref(AIDL_RETURN_NAME), resultParam, RESULT_NAME, ss);
        }

        if (genInfo.isValidateFuncSupported &&
            !param.empty() &&
            is_same_string(param.rbegin()->name, AIDL_RETURN_NAME))
        {
            /* copy result into last parameter of internal func */
            add_copy_hal_result_line(SPACES_2, RESULT_NAME, AIDL_RETURN_NAME, ss);
        }
    }
    else
    {
        /* aidl intf return void */
        add_hal_result_line(SPACES_2, resultParam, HAL_STATUS_PARAM_TYPE, HAL_STATUS_VAR, ss);
    }

    if (valid_str(resultType))
    {
        /* return pair */
        ss << SPACES_2 << CPP_RETURN << MOD_BEGIN_STR << hrs_get_return_pair(resultType) <<
            ", " << SCOPED_ASTATUS_OK << "};" << endl;
    }
    else
    {
        /* remove hal status */
        add_remove_hal_status_line(MESSAGE_TYPE_NAME, SPACES_2, ss);

        /* return ScopedAStatus */
        if (existReturnType)
        {
            add_return_scoped_astatus_line(SPACES_2, get_struct_field(RES_NAME, STATUS_NAME), ss);
        }
        else
        {
            add_return_scoped_astatus_line(SPACES_2, RES_NAME, ss);
        }
    }

    add_module_end(SPACES, ss);
}

static void hrs_add_internal_func(const ApccGenInfo& genInfo, HalRpcFunc& rpcFunc, stringstream& ss)
{
    const string& returnType = rpcFunc.internalFunc.returnType;
    string paramName;
    string aidlCallbackVar;

    apcc_add_class_func_impl(rpcFunc.internalFunc, genInfo.halRpcClassInfo.className, "", MOD_BEGIN_STR, ss);

    add_message_type_var_line(rpcFunc.halReqName, SPACES, ss);
    add_payload_var_line(SPACES, ss);
    add_session_id_var_line(SPACES, ss);
    add_instance_id_var_line(genInfo, rpcFunc, SPACES, ss);

    if (genInfo.halRpcClassInfo.existCallback)
    {
        if (is_add_callback_required(rpcFunc.internalFunc, genInfo.aidlCallbackName, aidlCallbackVar))
        {
            apcc_add_func_call(ADD_HAL_CALLBACK_FUNC, aidlCallbackVar, SPACES, ss);
            ss << endl;
        }
    }
    if (genInfo.halRpcClassInfo.existCallback2)
    {
        if (is_add_callback_required(rpcFunc.internalFunc, genInfo.aidlCallback2Name, aidlCallbackVar))
        {
            apcc_add_func_call(ADD_HAL_CALLBACK2_FUNC, aidlCallbackVar, SPACES, ss);
            ss << endl;
        }
    }
    if (genInfo.halRpcClassInfo.existCallbackResult)
    {
        if (is_add_callback_required(rpcFunc.internalFunc, genInfo.aidlCallbackResultName, aidlCallbackVar))
        {
            apcc_add_func_call(ADD_HAL_CALLBACK_RESULT_FUNC, aidlCallbackVar, SPACES, ss);
            ss << endl;
        }
    }
    if (genInfo.halRpcClassInfo.existCallback ||
        genInfo.halRpcClassInfo.existCallbackResult)
    {
        if (is_clear_callback_required(genInfo, rpcFunc.internalFunc))
        {
            apcc_add_func_call(CLEAR_HAL_CALLBACK_FUNC, "", SPACES, ss);
            ss << endl;
        }
    }

    if (apcc_is_pair_type(returnType))
    {
        string resultType, statusType;

        if (!apcc_get_cpp_pair_type(returnType, resultType, statusType))
        {
            loge("%s: fail to get pair type for %s", __func__, returnType.c_str());
        }

        // logd("%s: returnType: %s, resultType: %s, statusType: %s", __func__,
        //     returnType.c_str(), resultType.c_str(), statusType.c_str());

        paramName = hrs_get_internal_param_name(rpcFunc.internalFunc, rpcFunc.validateFuncName,
                    apcc_is_shared_ptr_type(resultType) ? false : true);

        /* 1. add result var */
        hrs_add_internal_result_var(resultType, ss);

        /* 2. add HalStatus var */
        add_hal_status_var_line(SPACES, ss);
        ss << endl;

        /* 3. serialize hal msg */
        add_hal_msg_func_call(rpcFunc.halReqFunc, SPACES, ss);
        hrs_add_internal_return_pair(genInfo, resultType, SPACES_2, ss);
        ss << endl;

        /* 4. send msg and verify result */
        add_send_msg_line(rpcFunc.halReqName, get_if_line_prefix(SPACES), get_if_line_suffix(), ss);
        hrs_add_internal_return_pair(genInfo, resultType, SPACES_2, ss);
        ss << endl;

        /* 5. wait hal status */
        add_wait_hal_status_line(MESSAGE_TYPE_NAME, HAL_STATUS_VAR, SPACES, ss);

        /* 6. handle status ok */
        hrs_add_internal_status_ok_module(genInfo, rpcFunc, resultType, ss);

        /* 7. return pair */
        hrs_add_internal_return_pair(genInfo, resultType, SPACES, ss);
    }
    else
    {
        paramName = hrs_get_internal_param_name(rpcFunc.internalFunc, rpcFunc.validateFuncName, false);

        if (!apcc_is_void_type(rpcFunc.commonReturnType))
        {
            /* 1. add result var */
            if (genInfo.isValidateFuncSupported)
            {
                hrs_add_internal_result_var(rpcFunc.commonReturnType, ss);
            }
        }

        if (genInfo.isAidlSyncApi)
        {
            /* 2. add HalStatus var */
            add_hal_status_var_line(SPACES, ss);
            ss << endl;
        }

        /* 3. serialize hal msg */
        add_hal_msg_func_call(rpcFunc.halReqFunc, SPACES, ss);
        apcc_add_return_line(SPACES_2, apcc_create_hal_status_error_unknown(genInfo), ss);
        ss << endl;

        /* 4. send msg and verify result */
        add_send_msg_line(MESSAGE_TYPE_NAME, get_if_line_prefix(SPACES), get_if_line_suffix(), ss);
        apcc_add_return_line(SPACES_2, apcc_create_hal_status_error_unknown(genInfo), ss);
        ss << endl;

        if (genInfo.isAidlSyncApi)
        {
            /* 5. wait hal status */
            add_wait_hal_status_line(MESSAGE_TYPE_NAME, HAL_STATUS_VAR, SPACES, ss);
            /* 6. handle status ok */
            hrs_add_internal_status_ok_module(genInfo, rpcFunc, "", ss);
            apcc_add_return_line(SPACES, apcc_create_hal_status_error_unknown(genInfo), ss);
        }
        else
        {
            /* return status ok */
            apcc_add_return_line(SPACES, SCOPED_ASTATUS_OK, ss);
        }
    }

    add_module_end(ss);
    ss << endl;
}

static void hrs_add_init_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    const HalRpcClassInfo& halRpcClassInfo = genInfo.halRpcClassInfo;

    if (genInfo.isSingleSomeip)
    {
        if (genInfo.isAidlIntfRoot)
        {
            /* HalRpc::setAddHeaderFlag(true); */
            ss << SPACES << HAL_RPC_SET_ADD_HEADER_FLAG_FUNC << '(' << CPP_TRUE << ");" << endl;
            /* sProxy = shared_from_this(); */
            ss << SPACES << halRpcClassInfo.staticProxy.name << " = " << STD_SHARED_FROM_THIS_FUNC << ';' << endl;
        }
        else
        {
            string className = concat_string(genInfo.aidlHalRoot.halName.name, RPC_NAME);
            /* HalRpc::setProxy(HalRpc::getStaticProxy()); */
            ss << SPACES << HAL_RPC_SET_PROXY_FUNC << '(' << apcc_get_static_proxy_func(className) << ");" << endl;;
        }
    }

    apcc_add_func_call(HAL_RPC_INIT_FUNC, "", SPACES, ss);
}

static void hrs_add_init_someip_context_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (valid_str(genInfo.aidlCallbackName))
    {
        ss << SPACES << apcc_get_cpp_vector_type(CPP_UINT16) << " " << SOMEIP_EVENT_ID_VAR << ";" << endl;
    }

    ss << SPACES << SOMEIP_APP_NAME_VAR << " = " << genInfo.halMsgInfo.clientAppName << ";" << endl;
    ss << endl;

    ss << SPACES << SOMEIP_CONTEXT_VAR << ".service_id = " << genInfo.halMsgInfo.serviceId << ";" << endl;
    ss << SPACES << SOMEIP_CONTEXT_VAR << ".instance_id = " << HAL_RPC_MAP_2_SOMEIP_INSTANCE_ID_FUNC <<
        '(' << INSTANCE_ID_VAR_NAME << ')' << ";" << endl;

    if (valid_str(genInfo.aidlCallbackName))
    {
        ss << SPACES << CPP_FOR << " (" << apcc_get_cpp_vector_type(CPP_UINT16) << "::size_type " <<
            INDEX_NAME << " = 0; " << INDEX_NAME << " < " << genInfo.halMsgInfo.indCount << "; " <<
            INDEX_NAME << "++) {" << endl;
        ss << SPACES_2 << SOMEIP_EVENT_ID_VAR << ".push_back(" << genInfo.halMsgInfo.indBase <<
            " + " << INDEX_NAME << ");" << endl;
        add_module_end(SPACES, ss);

        ss << SPACES << SOMEIP_CONTEXT_VAR << ".addEvents(" << genInfo.halMsgInfo.eventGroupId <<
            ", " << SOMEIP_EVENT_ID_VAR << ");" << endl;
    }
}

static void hrs_add_init_hal_status_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* hal_status_ = std::make_shared<SomeipMap>(deleteHalStatus, getMaxResponse()); */
    ss << SPACES << HAL_STATUS_INT_VAR << " = " << STD_NAMESPACE <<
        "::" << STD_MAKE_SHARED_FUNC << '<' << SOMEIP_MAP_NAME << ">("
        DELETE_HAL_STATUS_FUNC << ", " << GET_MAX_RESPONSE_NAME <<
        "());" << endl;
}

static string hrs_get_parse_hal_status_func_name(const ApccGenInfo& genInfo)
{
    string halName = apcc_get_package_name(genInfo.aidlPackageName);
    convert_first_char_to_upper(halName);

    stringstream ss;
    ss << halName << PARSE_HAL_STATUS_FUNC;
    return ss.str();
}

static void hrs_add_handle_hal_cfm_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    HalRpcSourceInstance* inst = sHalRpcSourceInstance;

    if (funcType.param.size() < 3)
        return;

    const string& message_type = funcType.param[0].name;
    const string& data = funcType.param[1].name;
    const string& length = funcType.param[2].name;
    string parseHalStatusFunc = hrs_get_parse_hal_status_func_name(genInfo);

    ss << SPACES << CPP_VOID_POINTER << " " << RES_NAME << " = " << CPP_NULL << ";" << endl;
    ss << endl;

    /* if (!halStatus) */
    apcc_add_if_line(SPACES, check_param(HAL_STATUS_VAR), ss);
    apcc_add_return_line(SPACES_2, ss);
    ss << endl;

    apcc_add_switch_line(message_type, SPACES, MOD_BEGIN_STR, ss);

    if (!genInfo.halResultInfo.empty())
    {
        for (vector<HalResultInfo>::size_type index = 0; index < genInfo.halResultInfo.size(); index++)
        {
            HalResultInfo hri = genInfo.halResultInfo[index];
            vector<string> parseCfmParam;

            get_hal_cfm_param(data, length, hri.halResultName, RES_NAME, hri.halResultParseFunc, parseCfmParam);

            apcc_add_case_line(hri.halCfm, SPACES_2, MOD_BEGIN_STR, ss);

            add_new_hal_result_line(SPACES_3, RES_NAME, hri.halResultName, ss);

            apcc_add_func_call(hri.halResultParseFunc.funcName, parseCfmParam, SPACES_3, ss);
            ss << endl;

            apcc_add_break_line(SPACES_3, ss);
            add_module_end(SPACES_2, ss);
        }
    }

    apcc_add_default_line(SPACES_2, MOD_BEGIN_STR, ss);
    if ((inst->addDefaultHalStatus) &&
        (genInfo.halResultInfo.size() < genInfo.halCfmDef.size()))
    {
        vector<string> parseCfmParam;

        add_new_hal_result_line(SPACES_3, RES_NAME, HAL_STATUS_PARAM_TYPE, ss);
        get_hal_cfm_param(data, length, RES_NAME, HAL_STATUS_PARAM_TYPE, parseCfmParam);
        apcc_add_func_call(parseHalStatusFunc, parseCfmParam, SPACES_3, ss);
        ss << endl;
    }
    apcc_add_break_line(SPACES_3, ss);
    add_module_end(SPACES_2, ss);

    add_module_end(SPACES, ss);

    ss << endl;
    add_update_hal_status_line(SPACES, HAL_STATUS_VAR, RES_NAME, ss);
}

static void hrs_add_handle_hal_ind_func(const vector<string>& halIndDef, const vector<FuncType>& halRpcHandleIndFunc,
    const vector<string>& paramName, const string& prefix, stringstream& ss)
{
    if (!halIndDef.empty())
    {
        for (vector<string>::size_type index = 0; index < halIndDef.size(); index++)
        {
            const FuncType& funcType = halRpcHandleIndFunc[index];
            string newSpace = concat_string(prefix, SPACES);

            apcc_add_case_line(halIndDef[index], prefix, MOD_BEGIN_STR, ss);

            apcc_add_func_call(funcType.funcName, paramName, newSpace, ss);
            ss << endl;

            apcc_add_break_line(newSpace, ss);

            add_module_end(prefix, ss);
        }
    }
}

static void hrs_add_handle_hal_ind_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    const string& message_type = funcType.param[0].name;
    vector<string> param;

    /* param: "data" */
    param.push_back(funcType.param[1].name);
    /* param: "length" */
    param.push_back(funcType.param[2].name);

    apcc_add_switch_line(message_type, SPACES, MOD_BEGIN_STR, ss);

    hrs_add_handle_hal_ind_func(genInfo.halIndDef, genInfo.halRpcHandleIndFunc, param, SPACES_2, ss);

    hrs_add_handle_hal_ind_func(genInfo.halInd2Def, genInfo.halRpcHandleInd2Func, param, SPACES_2, ss);

    apcc_add_default_line(SPACES_2, MOD_BEGIN_STR, ss);
    ss << SPACES_3 << COMMENT_UNKNOWN_MESSAGE_TYPE << endl;
    apcc_add_break_line(SPACES_3, ss);
    add_module_end(SPACES_2, ss);

    add_module_end(SPACES, ss);
}

static void hrs_add_handle_hal_req_func(const ApccGenInfo& genInfo, const FuncType& funcType, stringstream& ss)
{
    if (funcType.param.size() < 3)
    {
        /* invalid param */
        return;
    }

    const string& message_type = funcType.param[0].name;
    vector<string> paramName;
    get_hal_msg_param(funcType.param, paramName);

    apcc_add_switch_line(message_type, SPACES, MOD_BEGIN_STR, ss);

    if (!genInfo.halUpReqDef.empty())
    {
        for (vector<string>::size_type index = 0; index < genInfo.halUpReqDef.size(); index++)
        {
            const FuncType& funcType = genInfo.halRpcHandleUpReqFunc[index];

            apcc_add_case_line(genInfo.halUpReqDef[index], SPACES_2, MOD_BEGIN_STR, ss);

            apcc_add_func_call(funcType.funcName, paramName, SPACES_3, ss);
            ss << endl;

            apcc_add_break_line(SPACES_3, ss);

            add_module_end(SPACES_2, ss);
        }
    }

    apcc_add_default_line(SPACES_2, MOD_BEGIN_STR, ss);
    ss << SPACES_3 << COMMENT_UNKNOWN_MESSAGE_TYPE << endl;
    apcc_add_break_line(SPACES_3, ss);
    add_module_end(SPACES_2, ss);

    add_module_end(SPACES, ss);
}

static void hrs_add_hal_rpc_override_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halRpcOverrideFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcOverrideFunc.size(); index++)
        {
            const FuncType& funcType = genInfo.halRpcOverrideFunc[index];
            const string& funcName = funcType.funcName;
            apcc_add_class_func_impl(funcType, genInfo.halRpcClassInfo.className, "", MOD_BEGIN_STR, ss);

            if (is_same_string(funcName, INIT_FUNC))
                hrs_add_init_func(genInfo, ss);
            else if (is_same_string(funcName, INIT_SOMEIP_CONTEXT_FUNC))
                hrs_add_init_someip_context_func(genInfo, ss);
            else if (is_same_string(funcName, INIT_HAL_STATUS_FUNC))
                hrs_add_init_hal_status_func(genInfo, ss);
            else if (is_same_string(funcName, HANDLE_HAL_CFM_FUNC))
                hrs_add_handle_hal_cfm_func(genInfo, funcType, ss);
            else if (is_same_string(funcName, HANDLE_HAL_IND_FUNC))
                hrs_add_handle_hal_ind_func(genInfo, funcType, ss);
            else if (is_same_string(funcName, HANDLE_HAL_REQ_FUNC))
                hrs_add_handle_hal_req_func(genInfo, funcType, ss);

            add_module_end(ss);
            ss << endl;
        }
    }
}

static void hrs_add_verify_ptr(const string& prefix, const FuncType& funcType, stringstream& ss)
{
    if (funcType.param.size() >= 2)
    {
        apcc_add_if_line(prefix, check_param(funcType.param[0].name, funcType.param[1].name), ss);
        apcc_add_return_line(concat_string(prefix, SPACES), ss);
        ss << endl;
    }
}

static void hrs_add_param_line(const string& prefix, const FuncParameter& param, stringstream& ss)
{
    ss << prefix << param.type << " " << param.name << ';' << endl;
    ss << endl;
}

static string hrs_get_hal_msg_param(const FuncType& halMsgFuncType, const FuncType& callerFuncType,
    const FuncParameter& param, const vector<string>& paramName)
{
    stringstream ss;
    if ((halMsgFuncType.param.size() >= 3) &&
        (callerFuncType.param.size() >= 2))
    {
        ss << callerFuncType.param[0].name << ", " <<
            callerFuncType.param[1].name;

        ss << ", " << param.name;
    }
    return ss.str();
}

static void hrs_add_parse_msg_func(const string& prefix, const FuncType& halMsgFuncType, const FuncType& callerFuncType,
    const FuncParameter& param, const vector<string>& paramName, stringstream& ss)
{
    ss << prefix << CPP_IF << " (!";
    apcc_add_func_call(halMsgFuncType.funcName,
        hrs_get_hal_msg_param(halMsgFuncType, callerFuncType, param, paramName), "", "", ss);
    ss << ')' << endl;
    apcc_add_return_line(concat_string(prefix, SPACES), ss);
    ss << endl;
}

static void hrs_get_callback_param(const FuncType& funcType, const string& paramType,
    FuncParameter& param, vector<string>& paramName)
{
    paramName.clear();

    vector<FuncParameter>::size_type size = funcType.param.size();
    if (size == 1)
    {
        const string& pt = funcType.param[0].type;
        const string& pn = funcType.param[0].name;
        paramName.push_back(pn);

        param.type = pt;
        param.name = pn;
    }
    else if (size > 1)
    {
        for (vector<FuncParameter>::size_type index = 0; index < size; index++)
        {
            paramName.push_back(struct_field(PARAM_VAR_NAME, funcType.param[index].name));
        }

        param.type = paramType;
        param.name = PARAM_VAR_NAME;
    }
}

static void hrs_add_callback(const FuncType& funcType, const vector<string>& paramName,
    const string& prefix, stringstream& ss)
{
    apcc_add_func_call(funcType.funcName, paramName, prefix, ss);
    ss << endl;
}

static void hrs_add_callback(const FuncType& funcType, const string& callbackVarName,
    const vector<string>& paramName, const string& prefix, stringstream& ss)
{
    string funcName = funcType.funcName;
    if (!contain_prefix(funcName, callbackVarName))
    {
        replace_string(funcName, CALLBACK_VAR_NAME, callbackVarName, false);
    }
    apcc_add_func_call(funcName, paramName, prefix, ss);
    ss << endl;
}

static void hrs_add_callback_result(const ApccGenInfo& genInfo, const FuncType& funcType,
    const string& paramType, const string& prefix, stringstream& ss)
{
    FuncParameter param;
    vector<string> paramName;

    hrs_get_callback_param(funcType, paramType, param, paramName);

    if (exist_return_type(funcType.returnType))
    {
        /* Add last parameter for _aidl_return */
        paramName.push_back(cpp_type_ref_ptr(AIDL_RETURN_NAME));

        ss << prefix << SCOPED_ASTATUS << " " << STATUS_NAME << " = ";
    }

    apcc_add_func_call(funcType.funcName, paramName, "", ss);
    ss << endl;
}

static void hrs_add_handle_ind_func(const vector<FuncType>& halRpcHandleIndFunc, const vector<StructType>& halIndParamStructType,
    const vector<FuncType>& aidlCallbackFuncType, const vector<FuncType>& halIndFunc, const string& className,
    const FuncParameter& callbackVar, stringstream& ss)
{
    HalRpcSourceInstance* inst = sHalRpcSourceInstance;

    if (!halRpcHandleIndFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < halRpcHandleIndFunc.size(); index++)
        {
            FuncType funcType;
            const StructType& halIndParamStruct = halIndParamStructType[index];
            const FuncType& aidlCallbackFunc = aidlCallbackFuncType[index];
            bool isCallbackParamEmpty = aidlCallbackFunc.param.empty();
            FuncParameter param;
            vector<string> paramName;

            apcc_init_class_func_type(className, halRpcHandleIndFunc[index], funcType);

            apcc_add_func_impl(funcType, "", MOD_BEGIN_STR, ss);

            if (!isCallbackParamEmpty)
            {
                hrs_get_callback_param(aidlCallbackFunc, halIndParamStruct.name, param, paramName);

                hrs_add_param_line(SPACES, param, ss);

                if (inst->verifyPtr)
                    hrs_add_verify_ptr(SPACES, funcType, ss);

                hrs_add_parse_msg_func(SPACES, halIndFunc[index],
                    halRpcHandleIndFunc[index], param, paramName, ss);
            }

            if (apcc_is_multi_callback())
            {
                apcc_add_for_line(CALLBACK_NAME,  callbackVar.name, SPACES, MOD_BEGIN_STR, ss);

                hrs_add_callback(aidlCallbackFunc, paramName, SPACES_2, ss);

                add_module_end(SPACES, ss);
            }
            else
            {
                hrs_add_callback(aidlCallbackFunc, callbackVar.name, paramName, SPACES, ss);
            }

            add_module_end(ss);
            ss << endl;
        }
    }
}

static void hrs_add_handle_ind_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    hrs_add_handle_ind_func(genInfo.halRpcHandleIndFunc, genInfo.halIndParamStruct, genInfo.aidlCallbackFunc,
        genInfo.halIndFunc, genInfo.halRpcClassInfo.className, genInfo.halRpcClassInfo.callbackVar, ss);

    hrs_add_handle_ind_func(genInfo.halRpcHandleInd2Func, genInfo.halInd2ParamStruct, genInfo.aidlCallback2Func,
        genInfo.halInd2Func, genInfo.halRpcClassInfo.className, genInfo.halRpcClassInfo.callbackVar2, ss);
}

static void hrs_add_handle_upstream_req_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halRpcHandleUpReqFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcHandleUpReqFunc.size(); index++)
        {
            FuncType funcType;
            const FuncType& aidlCallbackResultFunc = genInfo.aidlCallbackResultFunc[index];
            const FuncType& halRpcHandleUpReqFunc = genInfo.halRpcHandleUpReqFunc[index];
            const FuncType& halUpReqFunc = genInfo.halUpReqFunc[index];
            const FuncType& halDownCfmFunc = genInfo.halDownCfmFunc[index];
            string paramName = get_callback_result_param_name(genInfo.halStatusInfo.aidlHalStatusType,
                                STATUS_NAME, AIDL_RETURN_NAME);
            if (halRpcHandleUpReqFunc.param.size() < 2)
            {
                /* invalid param */
                continue;
            }
            const string& data = halRpcHandleUpReqFunc.param[0].name;
            const string& length = halRpcHandleUpReqFunc.param[1].name;

            apcc_init_class_func_type(genInfo.halRpcClassInfo.className, halRpcHandleUpReqFunc, funcType);

            apcc_add_func_impl(funcType, "", MOD_BEGIN_STR, ss);

            if (exist_return_type(aidlCallbackResultFunc.returnType))
            {
                add_var_def_line(SPACES, aidlCallbackResultFunc.returnType, AIDL_RETURN_NAME, ss);
            }

            // apcc_dump_func_param(genInfo.aidlCallbackResultFunc[index].param);
            if (!aidlCallbackResultFunc.param.empty())
            {
                for (auto it: aidlCallbackResultFunc.param)
                {
                    add_var_def_line(SPACES, it.type, it.name, ss);
                }

                apcc_add_if_line(SPACES, check_param(data, length), ss);
                apcc_add_return_line(SPACES_2, ss);
                ss << endl;
            }

            apcc_add_if_line(SPACES, concat_string("!", genInfo.halRpcClassInfo.callbackVarResult.name), ss);
            apcc_add_return_line(SPACES_2, ss);
            ss << endl;

            // + parse upstream req begin
            ss << SPACES << CPP_IF << " (";
            apcc_add_func_call(halUpReqFunc.funcName, get_hal_upstream_req_parse_param(
                data, length, aidlCallbackResultFunc), "", "", ss);
            ss << ") {" << endl;

            hrs_add_callback_result(genInfo, aidlCallbackResultFunc,
                genInfo.halIndParamStruct[index].name, SPACES_2, ss);

            // ++ serialize downstream cfm begin
            if (exist_return_type(aidlCallbackResultFunc.returnType))
            {
                apcc_add_func_call(halDownCfmFunc.funcName, get_hal_downstream_cfm_parse_param(
                    STATUS_NAME, AIDL_RETURN_NAME, RESULT_NAME), SPACES_2, ";", ss);
            }
            else
            {
                apcc_add_func_call(halDownCfmFunc.funcName, get_hal_downstream_cfm_parse_param(
                    STATUS_NAME, PAYLOAD_VAR_NAME), SPACES_2, ";", ss);
            }
            ss << endl;
            // -- serialize downstream cfm end

            // - parse upstream req end
            add_module_end(SPACES, ss);

            add_module_end(ss);
            ss << endl;
        }
    }
}

static void hrs_add_hal_callback_func(const HalRpcCallbackUtilInfo& info, stringstream& ss)
{
    if (apcc_is_multi_callback())
    {
        /* callback_.push_back(callback); */
        ss << SPACES << info.aidlCallbackVar << ".push_back(" << info.funcType.param[0].name << ");" << endl;
    }
    else
    {
        /* callback_ = callback; */
        add_var_def(SPACES, info.aidlCallbackVar,  info.funcType.param[0].name, ss);
    }
}

static void hrs_add_hal_callback_result_func(const HalRpcCallbackUtilInfo& info, stringstream& ss)
{
    /* callback_result_ = callback; */
    add_var_def(SPACES, info.aidlCallbackVar,  info.funcType.param[0].name, ss);
}

static void hrs_add_clear_hal_callback_func(stringstream& ss)
{
    /* TODO */
}

static void hrs_add_callback_util_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (!genInfo.halRpcCallbackUtilInfo.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcCallbackUtilInfo.size(); index++)
        {
            const HalRpcCallbackUtilInfo& info = genInfo.halRpcCallbackUtilInfo[index];
            const FuncType& funcType = info.funcType;
            const string& funcName = funcType.funcName;
            apcc_add_class_func_impl(funcType, genInfo.halRpcClassInfo.className, "", MOD_BEGIN_STR, ss);

            if (is_same_string(funcName, ADD_HAL_CALLBACK_FUNC) ||
                is_same_string(funcName, ADD_HAL_CALLBACK2_FUNC))
                hrs_add_hal_callback_func(info, ss);
            else if (is_same_string(funcName, ADD_HAL_CALLBACK_RESULT_FUNC))
                hrs_add_hal_callback_result_func(info, ss);
            else if (is_same_string(funcName, CLEAR_HAL_CALLBACK_FUNC))
                hrs_add_clear_hal_callback_func(ss);

            add_module_end(ss);
            ss << endl;
        }
    }
}

static void hrs_add_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    HalRpcSourceInstance* inst = sHalRpcSourceInstance;

    /* 1. static variable */
    hrs_add_static_variable(genInfo, ss);

    /* 2. default constructor */
    hrs_add_default_constructor(genInfo, ss);

    /* 3. constructor */
    hrs_add_constructor(genInfo, ss);

    /* 4. destructor */
    hrs_add_destructor(genInfo, ss);

    /* 5. isValid func */
    if (apcc_is_validate_func_supported())
    {
        hrs_add_is_valid_func(genInfo, ss);
    }

    /* 6. static func */
    hrs_add_static_func(genInfo, ss);

    for (vector<FuncType>::size_type index = 0; index < inst->rpcFunc.size(); index++)
    {
        // logd("%s: %d/%d, hrs_add_func", __func__, index, inst->rpcFunc.size());
        hrs_add_func(genInfo, inst->rpcFunc[index], ss);
    }

    for (vector<FuncType>::size_type index = 0; index < inst->rpcFunc.size(); index++)
    {
        // logd("%s: %d/%d, hrs_add_internal_func", __func__, index, inst->rpcFunc.size());
        hrs_add_internal_func(genInfo, inst->rpcFunc[index], ss);
    }

    hrs_add_hal_rpc_override_func(genInfo, ss);

    hrs_add_handle_ind_func(genInfo, ss);

    hrs_add_handle_upstream_req_func(genInfo, ss);

    hrs_add_callback_util_func(genInfo, ss);
}

static void hrs_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* using for hal util */
    add_hal_util_using(genInfo, ss);
 
    apcc_add_intf_internal_using(genInfo, ss);
    apcc_add_callback_internal_using(genInfo, ss);
}

static void hrs_add_hal_include(const ApccGenInfo& genInfo, bool isAidlIntfRoot,
    const string& aidlIntfName, stringstream& ss)
{
    /* hal_api.h */
    if (isAidlIntfRoot)
    {
        apcc_add_include_header(apcc_get_gen_file_name(aidlIntfName, APCC_TYPE_HAL_API_HEADER), ss);
    }
    /* hal_message_def.h */
    apcc_add_include_header(apcc_get_gen_file_name(genInfo.aidlHalRoot.intfName, APCC_TYPE_HAL_MESSAGE_DEF_HEADER), ss);
    /* hal_msg.h */
    apcc_add_hal_msg_header(genInfo, ss);
    /* hal_rpc.h */
    apcc_add_include_header(apcc_get_gen_file_name(aidlIntfName, APCC_TYPE_HAL_RPC_HEADER), ss);
    /* hal_someip_def.h */
    apcc_add_include_header(apcc_get_gen_file_name(aidlIntfName, APCC_TYPE_HAL_SOMEIP_DEF_HEADER), ss);
}

static void hrs_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. std header */
    apcc_add_sys_include_header("stdlib.h", ss);

    /* 2. android header */
    apcc_add_sys_include_header("android-base/logging.h", ss);
    if (genInfo.isAidlIntfRoot)
    {
        apcc_add_sys_include_header("cutils/properties.h", ss);
    }
    ss << endl;

    /* 3. hal util header */
    add_hal_util_header(genInfo, ss);

    /* 4. hal rpc header */
    hrs_add_hal_include(genInfo, genInfo.isAidlIntfRoot, genInfo.aidlIntfName, ss);
    ss << endl;

    /* 5. header for aidl return type */
    if (!genInfo.halAidlIntfInfo.empty())
    {
        for (vector<AidlIntfInfo>::size_type index = 0; index < genInfo.halAidlIntfInfo.size(); index++)
        {
            apcc_add_include_header(genInfo.halAidlIntfInfo[index].headerFile, ss);
        }
        ss << endl;
    }

    if (genInfo.isSingleSomeip && !genInfo.isAidlIntfRoot)
    {
        apcc_add_include_header(genInfo.aidlHalRoot.halRpcHeader, ss);
        ss << endl;
    }

    if (valid_str(genInfo.aidlCallbackResultName))
    {
        apcc_add_include_header(apcc_get_gen_file_name(
            genInfo.aidlCallbackResultName, APCC_TYPE_HAL_MSG_HEADER), ss);
        ss << endl;
    }
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_rpc_source(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    hrs_init_gen(genInfo);

    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. include */
    hrs_add_include(genInfo, ss);

    /* 3. namespace prefix */
    apcc_add_namespace_prefix(genInfo.halRpcNamespace, ss);

    /* 4. using */
    hrs_add_using(genInfo, ss);

    /* 5. class implementation */
    hrs_add_class(genInfo, ss);

    /* 6. namespace prefix */
    apcc_add_namespace_suffix(genInfo.halRpcNamespace, ss);

    /* 7. func */
    hrs_add_api_func(genInfo, ss);
    FUNC_LEAVE();
}
