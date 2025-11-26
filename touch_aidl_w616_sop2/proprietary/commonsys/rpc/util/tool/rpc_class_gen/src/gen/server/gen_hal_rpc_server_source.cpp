/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "aidl.h"
#include "aidl_util.h"

#include "apcc_gen.h"

#include "log.h"
#include "util.h"

static bool hss_exist_return_value(const FuncType& funcType)
{
    return !apcc_is_cpp_type(funcType.returnType, CPP_VOID);
}

static string hss_get_if_line_prefix(const string& prefix, bool negative = true)
{
    stringstream ss;
    ss << prefix << CPP_IF << " (";
    if (negative)
    {
        ss << "!";
    }
    return ss.str();
}

static string hss_get_if_line_suffix()
{
    return ")";
}

static bool hss_is_param_empty(const vector<FuncParameter>& param)
{
    for (auto it: param)
    {
        if (!aidl_is_interface(it.type))
        {
            return false;
        }
    }
    return true;
}

static string hss_get_api_param(const string& aidlIntfVar, const string& instanceIdVar)
{
    stringstream ss;
    ss << aidlIntfVar << ", " << instanceIdVar;
    return ss.str();
}

/* ------------------------------------------------------------------------------------ */

static void hss_add_server_api_func(const ApccGenInfo& genInfo, stringstream& ss)
{
    if (genInfo.halServerInfo.halRpcServerApiFunc.empty())
        return;

    const FuncType& apiFunc = genInfo.halServerInfo.halRpcServerApiFunc[0];
    const vector<FuncParameter>& param = apiFunc.param;
    if (param.size() < 2)
        return;

    ss << endl;
    ss << apiFunc.returnType << " " << apiFunc.funcName << '(' << endl;
    ss << SPACES << param[0].type << " " << param[0].name << ", " <<
        param[1].type << " " << param[1].name << ')' << MOD_BEGIN_STR << endl;

    ss << SPACES << CPP_RETURN << " " << genInfo.halServerInfo.halRpcServerUsingType <<
        "::" << CREATE_NAME << "(" << param[0].name << ", " << param[1].name << ");" << endl;
    add_module_end(ss);
}

/* ------------------------------------------------------------------------------------ */

static void hss_add_param_line(const string& prefix, const vector<FuncParameter> serverFuncParam,
    const vector<FuncParameter> reqFuncParam, stringstream& ss)
{
    vector<FuncParameter>::size_type reqFuncParamSize = reqFuncParam.size();

    if (reqFuncParamSize > 2)
    {
        for (vector<FuncParameter>::size_type index = 2; index < reqFuncParamSize; index++)
        {
            apcc_add_variable(prefix, reqFuncParam[index].type, reqFuncParam[index].name, ss);
        }
    }

    if (!serverFuncParam.empty())
    {
        for (auto it: serverFuncParam)
        {
            string var_type = it.type;
            string var_name = it.name;
            if (aidl_is_interface(var_type))
            {
                /* ignore AIDL interface */
                continue;
            }
            else if (apcc_is_out_param_type(var_type, var_name))
            {
                var_type = apcc_get_out_param_type(var_type);
                apcc_add_variable(prefix, var_type, var_name, ss);
            }
        }
        ss << endl;
    }
}

static void hss_add_parse_msg_func(const string& prefix, const FuncType& halRpcServerReqFunc, stringstream& ss)
{
    ss << prefix << CPP_IF << " (!";
    apcc_add_func_call(halRpcServerReqFunc, "", "", ss);
    ss << ')' << endl;
    apcc_add_return_line(concat_string(prefix, SPACES), ss);
    ss << endl;
}

static string hss_get_req_param_struct_field(const string& structVarName, const string& fieldName)
{
    stringstream ss;
    ss << structVarName << '.' << fieldName;
    return ss.str();
}

static void hss_init_hal_func_param(const ApccGenInfo& genInfo, const FuncType& halRpcServerFunc,
    const string& reqParamStructVar, vector<string>& paramName)
{
    uint32_t inParamCount = 0;

    paramName.clear();
    for (auto it: halRpcServerFunc.param)
    {
        if (aidl_is_interface(it.type) ||
            apcc_is_out_param_type(it.type, it.name))
        {
            /* ignore */
            continue;
        }
        ++inParamCount;
    }

    for (auto it: halRpcServerFunc.param)
    {
        if (aidl_is_interface(it.type))
        {
            const AidlCallbackInfo& aidlCallbackInfo = apcc_get_aidl_call_back_info(genInfo, it.type);
            const string& name = aidlCallbackInfo.param.name;
            if (valid_str(name))
                paramName.push_back(name);
        }
        else if (apcc_is_out_param_type(it.type, it.name))
        {
            paramName.push_back(cpp_type_ref_ptr(it.name));
        }
        else
        {
            if (inParamCount > 1)
            {
                paramName.push_back(hss_get_req_param_struct_field(reqParamStructVar, it.name));
            }
            else if (inParamCount == 1)
            {
                paramName.push_back(it.name);
            }
        }
    }

    if (hss_exist_return_value(halRpcServerFunc))
    {
        paramName.push_back(cpp_type_ref_ptr(AIDL_RETURN_NAME));
    }
}

static void hss_add_hal_intf_func_call(const string& prefix, const FuncParameter& intfParam,
    const string& funcName, const vector<string>& paramName, stringstream& ss)
{
    ss << prefix << intfParam.name << "->";
    apcc_add_func_call(funcName, paramName, "", ";", ss);
    ss << endl;
}

static bool hss_get_remove_intf_param(const ApccGenInfo& genInfo, const string& funcName, string& aidlIntfName,
    FuncParameter& funcParam, FuncType& getIntfFunc, FuncParameter& halRpcServerInstListParam)
{
    if (contain_prefix(funcName, REMOVE_NAME) ||
        contain_prefix(funcName, DELETE_NAME))
    {
        if (!genInfo.aidlIntfReturnInfo.empty())
        {
            for (auto& it: genInfo.aidlIntfReturnInfo)
            {
                const AidlIntfReturnInfo& airi = it.second;
                if (!airi.removeIntfFunc.empty() &&
                    !airi.getIntfFunc.empty())
                {
                    const FuncType& removeIntfFunc = airi.removeIntfFunc[0];
                    if (is_same_string(funcName, removeIntfFunc.funcName))
                    {
                        /* found */
                        aidlIntfName = it.first;
                        funcParam = {apcc_get_shared_ptr_type(aidlIntfName), INTERFACE_VAR};
                        getIntfFunc = airi.getIntfFunc[0];
                        halRpcServerInstListParam = airi.halRpcServerInstListParam;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static void hss_add_remove_interface_func(const ApccGenInfo& genInfo, const string& prefix, const FuncType& halRpcServerFunc,
    const FuncParameter& intfParam, const vector<string>& paramName, stringstream& ss)
{
    const string& funcName = halRpcServerFunc.funcName;
    string aidlIntfName;
    FuncParameter funcParam;
    FuncType getIntfFunc;
    FuncParameter halRpcServerInstListParam;
    vector<string> new_param_name = paramName;

    if (!apcc_is_remove_intf_enabled())
        return;

    if (!hss_get_remove_intf_param(genInfo, funcName, aidlIntfName, funcParam, getIntfFunc, halRpcServerInstListParam))
        return;

    logd("%s: func name: %s, aidl intf name: %s", __func__, funcName.c_str(), aidlIntfName.c_str());
    apcc_dump_func_param(funcParam, "intf param");
    apcc_dump_func_param(halRpcServerInstListParam, "inst list param");
    apcc_dump_func_type(getIntfFunc);

    new_param_name.push_back(cpp_type_ref_ptr(funcParam.name));

    /* shared_ptr<IHal> _intf; */
    apcc_add_variable(prefix, funcParam.type, funcParam.name, ss);

    /* hal_->getIface(key, &_intf); */
    ss << prefix << intfParam.name << "->";
    apcc_add_func_call(getIntfFunc.funcName, new_param_name, "", ";", ss);
    ss << endl;

    /* HalRpc::removeInstance(hal_rpc_server_, _intf); */
    ss << prefix << HAL_RPC_REMOVE_INSTANCE_FUNC << '(' << halRpcServerInstListParam.name << ", " << funcParam.name << ");" << endl;
    ss << endl;
}

static void hss_add_hal_intf_func_call_with_return(const string& prefix, const string& returnVar,
    const FuncParameter& intfParam, const string& funcName, const vector<string>& paramName, stringstream& ss)
{
    ss << prefix << SCOPED_ASTATUS << " " << returnVar << " = ";
    hss_add_hal_intf_func_call("", intfParam, funcName, paramName, ss);
}

static void hss_add_instance_id_var(const string& prefix, const string& paramType,
    const string& paramName, stringstream& ss)
{
    ss << prefix << paramType << " " << paramName << " = " << INVALID_HAL_INSTANCE_ID_NAME << ';' << endl;
}

static void hss_init_hal_cfm_func_call_var(const FuncType& funcType, const string& returnType,
    const string& halStatusParamVar, const string& statusVar, const string& infoVar, vector<string>& paramName)
{
    vector<FuncParameter>::size_type size = funcType.param.size();
    vector<FuncParameter>::size_type pos = 0;

    paramName.clear();

    if (!apcc_is_void_type(returnType))
    {
        /* 1. const HalStatusParam& _hal_status_param; */
        paramName.push_back(halStatusParamVar);
        pos = 1;
    }
    else
    {
        /* 1. int32_t _status; */
        paramName.push_back(statusVar);
        /* 2. string _info; */
        paramName.push_back(infoVar);
        pos = 2;
    }

    for (vector<FuncParameter>::size_type index = pos; index < size; index++)
    {
        const FuncParameter& param = funcType.param[index];
        paramName.push_back(param.name);
    }
}

static void hss_add_hal_cfm_func_call(const ApccGenInfo& genInfo, const FuncType& funcType,
    const string& returnType, const string& halStatusParamVar, const string& statusVar,
    const string& infoVar, const string& prefix, stringstream& ss)
{
    string funcName;
    vector<string> paramName;

    hss_init_hal_cfm_func_call_var(funcType, returnType, halStatusParamVar, statusVar, infoVar, paramName);

    ss << endl;
    apcc_add_func_call(funcType.funcName, paramName, prefix, ";", ss);
    ss << endl;
}

static void hss_add_create_hal_rpc_server(const ApccGenInfo& genInfo, const string& returnType,
    const string& scopedAStatusVar, const string& instanceIdVar, const string& aidlReturnVar,
    const string& prefix, stringstream& ss)
{
    string newPrefix = concat_string(prefix, SPACES_2);
    const AidlIntfReturnInfo& aidlIntfReturnInfo = apcc_get_aidl_intf_return_info(genInfo, returnType);
    const FuncParameter& halRpcServerClassParam = aidlIntfReturnInfo.halRpcServerClassParam;
    const FuncParameter& halRpcServerInstListParam = aidlIntfReturnInfo.halRpcServerInstListParam;

    if (!valid_str(aidlIntfReturnInfo.intfName) || aidlIntfReturnInfo.halRpcServerApiFunc.empty())
        return;

    const FuncType& halRpcServerApiFunc = aidlIntfReturnInfo.halRpcServerApiFunc[0];
    if (halRpcServerApiFunc.param.size() < 2)
        return;

    /* if (_scoped_astatus.isOk() && (_aidl_return != nullptr)) { */
    ss << prefix << CPP_IF << " (" << scopedAStatusVar << '.' << SCOPED_ASTATUS_IS_OK_FUNC <<
        " && (" << aidlReturnVar << " != " << CPP_NULLPTR << "))" << MOD_BEGIN_STR << endl;

    /* if (!HalRpc::existInstance(hal_rpc_server_, _aidl_return, instanceId)) { */
    ss << concat_string(prefix, SPACES) << CPP_IF << " (!" << HAL_RPC_EXIST_INSTANCE_FUNC <<
        '(' << halRpcServerInstListParam.name << ", " << aidlReturnVar << ", " << instanceIdVar <<
        "))" << MOD_BEGIN_STR << endl;

    /* instanceId = HalRpc::getInstanceIdAvailable(hal_rpc_server_); */
    ss << newPrefix << instanceIdVar << " = " << HAL_RPC_GET_INSTANCE_ID_AVAILABLE_FUNC <<
        '(' << halRpcServerInstListParam.name << ");" << endl;

    /* shared_ptr<HalRpcServer> halRpcServer = create_hal_rpc_server(_aidl_return, instanceId); */
    ss << newPrefix << halRpcServerClassParam.type << " " << halRpcServerClassParam.name << " = ";
    apcc_add_func_call(halRpcServerApiFunc.funcName, hss_get_api_param(aidlReturnVar, instanceIdVar), "", ";", ss);
    ss << endl;

    if (genInfo.isSingleSomeip)
    {
        ss << newPrefix << halRpcServerClassParam.name << "->" << SET_PROXY_NAME << '(';
        if (genInfo.isAidlIntfRoot)
        {
            /* halRpcServer->setProxy(shared_from_this()); */
            ss << STD_SHARED_FROM_THIS_FUNC;
        }
        else
        {
            /* halRpcServer->setProxy(proxy_); */
            ss << PROXY_VAR;
        }
        ss << ");" << endl;
    }

    /* hal_rpc_server_.insert(pair<shared_ptr<HalRpcServer>, uint16_t>(halRpcServer, instanceId)); */
    ss << newPrefix << halRpcServerInstListParam.name << ".insert(" << get_pair_type_def(
        halRpcServerClassParam.type, INSTANCE_ID_TYPE, halRpcServerClassParam.name, instanceIdVar)
        << ");" << endl;

    add_module_end(concat_string(prefix, SPACES), ss);
    add_module_end(prefix, ss);
}

static void hss_add_send_intf_return(const ApccGenInfo& genInfo, const string& prefix,
    const string& returnType, const string& halStatusParamVar, const string& statusVar,
    const string& infoVar, const string& halMsgName, const FuncType& halRpcServerCfmFunc, stringstream& ss)
{
    if (aidl_is_interface(returnType))
    {
        /* add instance id var in "uint16_t instance_id" for AIDL interface's instance */
        hss_add_instance_id_var(prefix, INSTANCE_ID_TYPE, INSTANCE_ID_NAME, ss);

        /* create and store hal rpc server */
        hss_add_create_hal_rpc_server(genInfo, returnType, SCOPED_ASTATUS_VAR, INSTANCE_ID_NAME, AIDL_RETURN_NAME, prefix, ss);
    }

    /* serialize hal cfm */
    hss_add_hal_cfm_func_call(genInfo, halRpcServerCfmFunc, returnType, halStatusParamVar, statusVar, infoVar, prefix, ss);
}

static void hss_add_hal_func_var(const string& prefix, const FuncType& halRpcServerFunc, stringstream& ss)
{
    /* T _aidl_return; */
    if (hss_exist_return_value(halRpcServerFunc))
    {
        const string& returnType = halRpcServerFunc.returnType;
        if (aidl_is_interface(returnType))
        {
            apcc_add_variable(prefix, apcc_get_shared_ptr_type(returnType), AIDL_RETURN_NAME, ss);
        }
        else
        {
            apcc_add_variable(prefix, returnType, AIDL_RETURN_NAME, ss);
        }
    }

    /* output parameter */
    for (auto it: halRpcServerFunc.param)
    {
        if (apcc_is_out_param_type(it.type, it.name))
        {
            apcc_add_variable(prefix, apcc_get_out_param_type(it.type), it.name, ss);
        }
    }
    ss << endl;
}

static void hss_add_set_status_var(const string& prefix, const string& scopedAStatusVar,
    const string& statusVar, stringstream& ss)
{
    ss << prefix << CPP_INT32 << " " << statusVar << " = " << scopedAStatusVar << ".getStatus();" << endl;
}

static void hss_add_set_info_var(const string& prefix, const string& scopedAStatusVar,
    const string& infoVar, stringstream& ss)
{
    ss << prefix << CPP_STRING << " " << infoVar << " = " << scopedAStatusVar << ".getMessage();" << endl;
}

static void hss_add_hal_status_param_var(const string& prefix, const string& returnType,
    const string& halStatusParamType, const string& halStatusParamVar, const string& statusVar,
    const string& infoVar, stringstream& ss)
{
    if (!apcc_is_void_type(returnType))
    {
        ss << prefix << halStatusParamType << " " << halStatusParamVar << " = ";
        ss << '{' << statusVar << ", " << infoVar << "};" << endl;
    }
}

static void hss_add_hal_func_call(const ApccGenInfo& genInfo, const string& prefix, const FuncType& halRpcServerFunc,
    const string& halMsgName, const FuncType& halRpcServerCfmFunc, stringstream& ss)
{
    const FuncParameter& intfParam = genInfo.halRpcServerClassInfo.intfParam;
    vector<string> paramName;
    string newPrefix;

    hss_init_hal_func_param(genInfo, halRpcServerFunc, PARAM_VAR_NAME, paramName);

    ss << prefix << CPP_IF << " (" << intfParam.name << " != " << CPP_NULLPTR << ')' << MOD_BEGIN_STR << endl;

    newPrefix = concat_string(prefix, SPACES);
    if (genInfo.handleHalCfm)
    {
        hss_add_hal_func_var(newPrefix, halRpcServerFunc, ss);

        /* optional: remove hal rpc server instance when to remove aidl interface */
        hss_add_remove_interface_func(genInfo, newPrefix, halRpcServerFunc, intfParam, paramName, ss);

        hss_add_hal_intf_func_call_with_return(newPrefix, SCOPED_ASTATUS_VAR, intfParam,
            halRpcServerFunc.funcName, paramName, ss);

        hss_add_set_status_var(newPrefix, SCOPED_ASTATUS_VAR, STATUS_VAR, ss);
        hss_add_set_info_var(newPrefix, SCOPED_ASTATUS_VAR, INFO_VAR, ss);
        hss_add_hal_status_param_var(newPrefix, halRpcServerFunc.returnType,
            HAL_STATUS_PARAM_TYPE, HAL_STATUS_PARAM_VAR, STATUS_VAR, INFO_VAR, ss);

        hss_add_send_intf_return(genInfo, newPrefix, halRpcServerFunc.returnType,
            HAL_STATUS_PARAM_VAR, STATUS_VAR, INFO_VAR, halMsgName, halRpcServerCfmFunc, ss);
    }
    else
    {
        hss_add_hal_intf_func_call(newPrefix, intfParam, halRpcServerFunc.funcName, paramName, ss);
    }
}

static void hss_add_server_handle_hal_req_func_impl(const string& className,
    const FuncType& halRpcServerHandleReqFunc, stringstream& ss)
{
    FuncType funcType;
    apcc_init_class_func_type(className, halRpcServerHandleReqFunc, funcType);
    apcc_add_func_impl(funcType, "", MOD_BEGIN_STR, ss);
}

static void hss_add_server_handle_hal_req_func(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    bool isCfmDefined = genInfo.isCfmDefined;
    if (!genInfo.halRpcServerHandleReqFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcServerHandleReqFunc.size(); index++)
        {
            const string& className = genInfo.halRpcServerClassInfo.className;
            const FuncType& halRpcServerHandleReqFunc = genInfo.halRpcServerHandleReqFunc[index];
            const FuncType& halRpcServerFunc = genInfo.halRpcServerFunc[index];
            const FuncType& halRpcServerReqFunc = genInfo.halRpcServerReqFunc[index];
            const FuncType& halRpcServerCfmFunc = genInfo.halRpcServerCfmFunc[index];
            const string& halCfmName = isCfmDefined ? genInfo.halCfmDef[index] : genInfo.halReqDef[index];
            bool isParamEmpty = hss_is_param_empty(halRpcServerFunc.param);

            hss_add_server_handle_hal_req_func_impl(className, halRpcServerHandleReqFunc, ss);

            if (!isParamEmpty)
            {
                hss_add_param_line(SPACES, halRpcServerFunc.param, halRpcServerReqFunc.param, ss);

                hss_add_parse_msg_func(SPACES, halRpcServerReqFunc, ss);
            }

            hss_add_hal_func_call(genInfo, SPACES, halRpcServerFunc, halCfmName, halRpcServerCfmFunc, ss);

            add_module_end(SPACES, ss);

            add_module_end(ss);
            ss << endl;
        }
    }
}

static void hss_add_override_handle_hal_req_func(const ApccGenInfo& genInfo, const string& className, const FuncType& funcType, stringstream& ss)
{
    const string& message_type = funcType.param[0].name;
    vector<string> param;

    /* param: "data" */
    param.push_back(funcType.param[1].name);
    /* param: "length" */
    param.push_back(funcType.param[2].name);
    /* param: "result" */
    if (genInfo.isAidlSyncApi)
    {
        param.push_back(RESULT_NAME);
    }

    apcc_add_switch_line(message_type, SPACES, MOD_BEGIN_STR, ss);

    if (!genInfo.halReqDef.empty())
    {
        for (vector<string>::size_type index = 0; index < genInfo.halReqDef.size(); index++)
        {
            const FuncType& funcType = genInfo.halRpcServerHandleReqFunc[index];

            apcc_add_case_line(genInfo.halReqDef[index], SPACES_2, MOD_BEGIN_STR, ss);

            apcc_add_func_call(funcType.funcName, param, SPACES_3, ss);
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

static void hss_add_server_callback_param(const string& aidlCallbackName, const string& halCallbackName,
    const FuncParameter& callbackParam, const string& prefix, stringstream& ss)
{
    if (valid_str(aidlCallbackName))
    {
        ss << prefix << callbackParam.name << " =" << endl;
        ss << prefix << SPACES << "ndk::SharedRefBase::make<" << halCallbackName <<
            ">(" << STD_SHARED_FROM_THIS_FUNC << ");" << endl;
    }
}

static void hss_add_server_callback_param(const ApccGenInfo& genInfo, const string& prefix, stringstream& ss)
{
    hss_add_server_callback_param(genInfo.aidlCallbackName, genInfo.halCallbackName,
        genInfo.halRpcServerClassInfo.callbackParam, prefix, ss);

    hss_add_server_callback_param(genInfo.aidlCallback2Name, genInfo.halCallback2Name,
        genInfo.halRpcServerClassInfo.callback2Param, prefix, ss);
}

static void hss_add_override_init_func(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    hss_add_server_callback_param(genInfo, SPACES, ss);

    apcc_add_func_call(HAL_RPC_INIT_FUNC, "", SPACES, ss);
}

static void hss_add_override_init_someip_context_func(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    ss << SPACES << apcc_get_cpp_vector_type(CPP_UINT16) << " " << SOMEIP_EVENT_ID_VAR << ";" << endl;

    ss << SPACES << SOMEIP_APP_NAME_VAR << " = " << genInfo.halMsgInfo.serverAppName << ";" << endl;
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

static void hss_add_server_override_func(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    if (!genInfo.halRpcServerOverrideFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcServerOverrideFunc.size(); index++)
        {
            const FuncType& funcType = genInfo.halRpcServerOverrideFunc[index];
            const string& funcName = funcType.funcName;
            const string& className = genInfo.halRpcServerClassInfo.className;
            apcc_add_class_func_impl(funcType, className, "", MOD_BEGIN_STR, ss);

            if (is_same_string(funcName, INIT_FUNC))
                hss_add_override_init_func(genInfo, className, ss);
            else if (is_same_string(funcName, INIT_SOMEIP_CONTEXT_FUNC))
                hss_add_override_init_someip_context_func(genInfo, className, ss);
            else if (is_same_string(funcName, HANDLE_HAL_REQ_FUNC))
                hss_add_override_handle_hal_req_func(genInfo, className, funcType, ss);

            add_module_end(ss);
            ss << endl;
        }
    }
}

static void hss_add_server_static_func(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    const FuncParameter& staticInst = genInfo.halRpcServerClassInfo.staticInst;
    const string& classVar = genInfo.halRpcServerClassInfo.classVar;
    const vector<FuncParameter>& createFuncParam = genInfo.halRpcServerClassInfo.createFuncParam;

    if (genInfo.halRpcServerStaticFunc.empty() ||
        createFuncParam.size() < 2)
        return;

    apcc_add_class_func_impl(genInfo.halRpcServerStaticFunc[0], className, "", MOD_BEGIN_STR, ss);

    ss << SPACES << staticInst.type << " " << classVar << " =" << endl;
    ss << SPACES_2 << STD_NAMESPACE << "::" << STD_MAKE_SHARED_FUNC << '<' <<
        className << ">(" << createFuncParam[0].name << ", " <<
        createFuncParam[1].name << ");" << endl;

    apcc_add_class_func_call_no_param(classVar, INIT_FUNC, SPACES, ss);

    if (genInfo.isAidlIntfRoot)
    {
        apcc_add_class_func_call_no_param(classVar, OPEN_SOMEIP_NAME, SPACES, ss);
    }

    apcc_add_return_line(SPACES, classVar, ss);

    add_module_end(ss);
    ss << endl;
}

static void hss_add_server_destructor(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    apcc_add_class_constructor_impl(genInfo.halRpcServerClassInfo.deconstructor, className, "", MOD_BEGIN_STR, ss);
    if (genInfo.isAidlIntfRoot || !genInfo.isSingleSomeip)
    {
        apcc_add_func_call_no_param(HAL_RPC_CLOSE_SOMEIP_FUNC, SPACES, ss);
    }
    add_module_end(ss);
    ss << endl;
}

static void hss_add_server_constructor(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    const FuncParameter& intfParam = genInfo.halRpcServerClassInfo.intfParam;
    const vector<FuncParameter>& param = genInfo.halRpcServerClassInfo.constructor.param;

    if (param.size() < 2)
    {
        /* invalid param */
        return;
    }

    apcc_add_class_constructor_impl(genInfo.halRpcServerClassInfo.constructor, className, "", "", ss);

    ss << SPACES << ": " << HAL_RPC_SERVER_CLASS << '(' << param[1].name;
    if (!genInfo.isAidlSyncApi)
    {
        ss << ", " << CPP_FALSE;
    }
    ss << ')' << endl;
    ss << SPACES << ", " << intfParam.name << '(' << param[0].name << ')' << MOD_BEGIN_STR << endl;

    add_module_end(ss);
    ss << endl;
}

static void hss_add_server_default_constructor(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    apcc_add_class_constructor_impl(genInfo.halRpcServerClassInfo.constructorDefault, className, "", "", ss);

    ss << SPACES << ": " << genInfo.halRpcServerClassInfo.className << "(nullptr, 0)" << MOD_BEGIN_STR << endl;

    add_module_end(ss);
    ss << endl;
}

static void hss_add_server_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    const string& className = genInfo.halRpcServerClassInfo.className;

    /* 1. default constructor */
    hss_add_server_default_constructor(genInfo, className, ss);

    /* 2. constructor */
    hss_add_server_constructor(genInfo, className, ss);

    /* 3. destructor */
    hss_add_server_destructor(genInfo, className, ss);

    /* 4. static func */
    hss_add_server_static_func(genInfo, className, ss);

    /* 5. override func */
    hss_add_server_override_func(genInfo, className, ss);

    /* 6. handle hal req func */
    hss_add_server_handle_hal_req_func(genInfo, className, ss);
}

/* ------------------------------------------------------------------------------------ */

static void hss_add_send_msg_line(const ApccGenInfo& genInfo, const string& halMsgName, const string& prefix, stringstream& ss)
{
    const string& classVarInt = genInfo.halRpcServerClassInfo.classVarInt;

    ss << prefix << CPP_IF << " ((" << classVarInt << " == " << CPP_NULLPTR << ") ||" << endl;

    ss << prefix << SPACES << '!' << classVarInt << "->" << SEND_NAME << '(' <<
        halMsgName << ", " << PAYLOAD_NAME << "))" << endl;
}

static void hss_add_hal_ind_func_call(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    apcc_add_func_call(funcType, hss_get_if_line_prefix(prefix), hss_get_if_line_suffix(), ss);
    ss << endl;
}

static void hss_add_hal_intf_call_override_func(const ApccGenInfo& genInfo, const string& className,
    const FuncType& halCallbackFunc, const FuncType& halRpcServerIndFunc, const string& halIndDef, stringstream& ss)
{
    apcc_add_class_func_impl(halCallbackFunc, className, "", MOD_BEGIN_STR, ss);

    /* 1. add payload var */
    apcc_add_variable(SPACES, CPP_BYTE_VECTOR, PAYLOAD_NAME, ss);
    ss << endl;

    /* 2. serialize hal msg */
    hss_add_hal_ind_func_call(halRpcServerIndFunc, SPACES, ss);
    apcc_add_return_line(SPACES_2, apcc_create_hal_status_error_unknown(genInfo), ss);
    ss << endl;

    /* 3. send msg */
    hss_add_send_msg_line(genInfo, halIndDef, SPACES, ss);
    apcc_add_return_line(SPACES_2, apcc_create_hal_status_error_unknown(genInfo), ss);
    ss << endl;

    /* 4. return ScopedAStatus::ok() */
    apcc_add_return_line(SPACES, SCOPED_ASTATUS_OK, ss);
    add_module_end(ss);
    ss << endl;
}

static void hss_add_hal_intf_call_override_func(const ApccGenInfo& genInfo, const string& className,
    const vector<FuncType>& halCallbackFunc, const vector<FuncType>& halRpcServerIndFunc,
    const vector<string>& halIndDef, stringstream& ss)
{
    if (!halCallbackFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < halCallbackFunc.size(); index++)
        {
            hss_add_hal_intf_call_override_func(genInfo, className, halCallbackFunc[index],
                halRpcServerIndFunc[index], halIndDef[index], ss);
        }
    }
}

static void hss_add_hal_intf_call_destructor(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    ss << className << "::" << '~' << className << "()" << MOD_BEGIN_STR << endl;
    add_module_end(ss);
    ss << endl;
}

static void hss_add_hal_intf_call_constructor(const ApccGenInfo& genInfo, const string& className, stringstream& ss)
{
    const HalRpcServerClassInfo& halRpcServerClassInfo = genInfo.halRpcServerClassInfo;

    ss << className << "::" << className << '(' << halRpcServerClassInfo.classType <<
        " " << halRpcServerClassInfo.classVar << ')' << endl;

    ss << SPACES << ": " << halRpcServerClassInfo.classVarInt << '(' <<
        halRpcServerClassInfo.classVar << ')' << MOD_BEGIN_STR << endl;

    add_module_end(ss);
    ss << endl;
}

static void hss_add_hal_intf_call_class(const ApccGenInfo& genInfo, const string& aidlCallbackName,
    const string& className, const vector<FuncType>& halCallbackFunc, const vector<FuncType>& halRpcServerIndFunc,
    const vector<string>& halIndDef, stringstream& ss)
{
    if (!valid_str(aidlCallbackName))
        return;

    /* 1. constructor */
    hss_add_hal_intf_call_constructor(genInfo, className, ss);

    /* 2. destructor */
    hss_add_hal_intf_call_destructor(genInfo, className, ss);

    /* 3. override func */
    hss_add_hal_intf_call_override_func(genInfo, className, halCallbackFunc, halRpcServerIndFunc, halIndDef, ss);
}

static void hss_add_hal_intf_call_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    hss_add_hal_intf_call_class(genInfo, genInfo.aidlCallbackName, genInfo.halCallbackClassInfo.className,
        genInfo.halCallbackFunc, genInfo.halRpcServerIndFunc, genInfo.halIndDef, ss);

    hss_add_hal_intf_call_class(genInfo, genInfo.aidlCallback2Name, genInfo.halCallback2ClassInfo.className,
        genInfo.halCallback2Func, genInfo.halRpcServerInd2Func, genInfo.halInd2Def, ss);
}

/* ------------------------------------------------------------------------------------ */

static void hss_add_class(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. hal callback class */
    hss_add_hal_intf_call_class(genInfo, ss);

    /* 2. hal rpc server class */
    hss_add_server_class(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

static void hss_add_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    apcc_add_intf_internal_using(genInfo, ss);
    apcc_add_callback_internal_using(genInfo, ss);
}

/* ------------------------------------------------------------------------------------ */

static void hss_add_include(const string& aidlIntfName, ApccType apccType, stringstream& ss)
{
    apcc_add_include_header(apcc_get_gen_file_name(aidlIntfName, apccType), ss);
}

static void hss_add_hal_util_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* 1. hal_status_util.h */
    if (genInfo.halStatusInfo.gen)
    {
        hss_add_include(aidl_get_intf_root(), APCC_TYPE_SERVER_HAL_STATUS_UTIL_HEADER, ss);
    }
    ss << endl;
}

static void hss_add_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    const string& aidlIntfName = genInfo.aidlIntfName;

    /* 1. hal rpc server header */
    hss_add_include(aidlIntfName, APCC_TYPE_HAL_RPC_SERVER_HEADER, ss);

    /* 2. hal util header */
    hss_add_hal_util_header(genInfo, ss);

    /* 3. hal message def header */
    hss_add_include(genInfo.aidlHalRoot.intfName, APCC_TYPE_HAL_MESSAGE_DEF_HEADER, ss);

    /* 4. hal msg header */
    apcc_add_hal_msg_header(genInfo, ss);

    /* 5. hal someip def header */
    hss_add_include(aidlIntfName, APCC_TYPE_HAL_SOMEIP_DEF_HEADER, ss);
    ss << endl;
}

/* ------------------------------------------------------------------------------------ */

void gen_hal_rpc_server_source(stringstream& ss)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();

    FUNC_ENTER();
    /* 1. comment */
    apcc_add_comment(ss);

    /* 2. include */
    hss_add_include(genInfo, ss);

    /* 3. namespace prefix */
    apcc_add_namespace_prefix(genInfo.halRpcNamespace, ss);

    /* 4. using */
    hss_add_using(genInfo, ss);

    /* 5. class implementation */
    hss_add_class(genInfo, ss);

    /* 6. namespace prefix */
    apcc_add_namespace_suffix(genInfo.halRpcNamespace, ss);

    /* 7. func */
    hss_add_server_api_func(genInfo, ss);
    FUNC_LEAVE();
}
