/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <iomanip>

#include "aidl.h"
#include "aidl_def.h"
#include "aidl_util.h"

#include "apcc.h"
#include "apcc_gen.h"

#include "log.h"

#include "proto_def.h"
#include "proto_util.h"

typedef struct
{
    map<string, CppType> aidlTypeMap;
    map<string, CppTypeT> cppTypeMap;
    map<uint32_t, HalFile> fileMap;
    map<uint32_t, GenFunc> genFunc;
    ApccGenInfo *genInfo;
    stringstream ss;
} ApccGenInstance;

static ApccGenInstance sApccGenInstance;

static void init_file_map(map<uint32_t, HalFile>& fileMap)
{
    /* 0. folder: . */
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_ANDROID_BP,                      {FILE_ANDROID_BP, FOLDER_CURRENT_DIR}));

    /* 1. folder: client */
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_API_HEADER,                  {FILE_HAL_API_HEADER, FOLDER_CLIENT_AIDL_API}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_RPC_HEADER,                  {FILE_HAL_RPC_HEADER, FOLDER_CLIENT_AIDL}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_RPC_SOURCE,                  {FILE_HAL_RPC_SOURCE, FOLDER_CLIENT_AIDL}));

    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_STATUS_UTIL_HEADER,          {FILE_HAL_STATUS_UTIL_HEADER, FOLDER_CLIENT_AIDL_UTIL}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_STATUS_UTIL_SOURCE,          {FILE_HAL_STATUS_UTIL_SOURCE, FOLDER_CLIENT_AIDL_UTIL}));

    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_CLIENT_ANDROID_BP,               {FILE_ANDROID_BP, FOLDER_CLIENT}));
    /* 2. folder: include */
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_MESSAGE_DEF_HEADER,          {FILE_HAL_MESSAGE_DEF_HEADER, FOLDER_INCLUDE}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_SOMEIP_DEF_HEADER,           {FILE_HAL_SOMEIP_DEF_HEADER, FOLDER_INCLUDE}));
    /* 3. folder: message */
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_MSG_HEADER,                  {FILE_HAL_MSG_HEADER, FOLDER_MESSAGE_INCLUDE}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_MSG_SOURCE,                  {FILE_HAL_MSG_SOURCE, FOLDER_MESSAGE_SRC}));

    /* 4. folder: server */
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_SERVICE_SOURCE,                  {FILE_SERVICE_SOURCE, FOLDER_SERVER_MAIN}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_SERVICE_HEADER,              {FILE_HAL_SERVICE_HEADER, FOLDER_SERVER_LIB_INCLUDE}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_SERVICE_SOURCE,              {FILE_HAL_SERVICE_SOURCE, FOLDER_SERVER_LIB}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_RPC_SERVER_HEADER,           {FILE_HAL_RPC_SERVER_HEADER, FOLDER_SERVER_LIB_INCLUDE}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_RPC_SERVER_SOURCE,           {FILE_HAL_RPC_SERVER_SOURCE, FOLDER_SERVER_LIB}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_IMPL_HEADER,                 {FILE_HAL_IMPL_HEADER, FOLDER_SERVER_IMPL}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_IMPL_SOURCE,                 {FILE_HAL_IMPL_SOURCE, FOLDER_SERVER_IMPL}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_HAL_SERVER_API_HEADER,           {FILE_HAL_SERVER_API_HEADER, FOLDER_SERVER_API}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_SERVER_HAL_STATUS_UTIL_HEADER,   {FILE_HAL_STATUS_UTIL_HEADER, FOLDER_SERVER_LIB_UTIL}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_SERVER_HAL_STATUS_UTIL_SOURCE,   {FILE_HAL_STATUS_UTIL_SOURCE, FOLDER_SERVER_LIB_UTIL}));

    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_SERVER_ANDROID_BP,               {FILE_ANDROID_BP, FOLDER_SERVER}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_SERVER_LIB_ANDROID_BP,           {FILE_ANDROID_BP, FOLDER_SERVER_LIB}));
    fileMap.insert(pair<uint32_t, HalFile>(APCC_TYPE_SERVER_MAIN_ANDROID_BP,          {FILE_ANDROID_BP, FOLDER_SERVER_MAIN}));
}

static void init_gen_func(map<uint32_t, GenFunc>& genFunc)
{
    /* 0. folder: . */
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_ANDROID_BP,                      gen_android_bp));

    /* 1. folder: client */
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_API_HEADER,                  gen_hal_api_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_RPC_HEADER,                  gen_hal_rpc_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_RPC_SOURCE,                  gen_hal_rpc_source));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_STATUS_UTIL_HEADER,          gen_hal_status_util_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_STATUS_UTIL_SOURCE,          gen_hal_status_util_source));

    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_CLIENT_ANDROID_BP,               gen_android_bp));
    /* 2. folder: include */
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_MESSAGE_DEF_HEADER,          gen_hal_message_def_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_SOMEIP_DEF_HEADER,           gen_hal_someip_def_header));
    /* 3. folder: message */
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_MSG_HEADER,                  gen_hal_msg_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_MSG_SOURCE,                  gen_hal_msg_source));

    /* 4. folder: server */
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_SERVICE_SOURCE,                  gen_service_source));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_SERVICE_HEADER,              gen_hal_service_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_SERVICE_SOURCE,              gen_hal_service_source));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_RPC_SERVER_HEADER,           gen_hal_rpc_server_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_RPC_SERVER_SOURCE,           gen_hal_rpc_server_source));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_IMPL_HEADER,                 gen_hal_impl_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_IMPL_SOURCE,                 gen_hal_impl_source));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_HAL_SERVER_API_HEADER,           gen_hal_api_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_SERVER_HAL_STATUS_UTIL_HEADER,   gen_hal_status_util_header));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_SERVER_HAL_STATUS_UTIL_SOURCE,   gen_hal_status_util_source));

    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_SERVER_ANDROID_BP,               gen_android_bp));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_SERVER_LIB_ANDROID_BP,           gen_android_bp));
    genFunc.insert(pair<uint32_t, GenFunc>(APCC_TYPE_SERVER_MAIN_ANDROID_BP,          gen_android_bp));
}

static void init_apcc_aidl_type_map(map<string, CppType>& aidlTypeMap)
{
    /* 1. C++ basic data type */
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_BOOLEAN,    {CPP_BOOL,   CPP_TYPE_BASIC}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_BYTE,       {CPP_INT8,   CPP_TYPE_BASIC}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_CHAR,       {CPP_CHAR16, CPP_TYPE_BASIC}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_INT,        {CPP_INT32,  CPP_TYPE_BASIC}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_LONG,       {CPP_INT64,  CPP_TYPE_BASIC}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_VOID,       {CPP_VOID,   CPP_TYPE_BASIC}));

    /* 2. C++ STL */
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_ARRAY,      {CPP_VECTOR,       CPP_TYPE_STL}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_BYTE_ARRAY, {CPP_BYTE_VECTOR,  CPP_TYPE_STL}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_INT_ARRAY,  {CPP_INT32_VECTOR, CPP_TYPE_STL}));
    aidlTypeMap.insert(pair<string, CppType>(AIDL_TYPE_STRING,     {CPP_STRING,       CPP_TYPE_STL}));

    /* 3. other C++ common type */
}

static void init_cpp_type_map(map<string, CppTypeT>& cppTypeMap)
{
    /* 1. C++ basic data type */
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_BOOL,   CPP_TYPE_BASIC));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_UINT8,  CPP_TYPE_BASIC));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_INT8,   CPP_TYPE_BASIC));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_CHAR16, CPP_TYPE_BASIC));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_INT32,  CPP_TYPE_BASIC));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_INT64,  CPP_TYPE_BASIC));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_VOID,   CPP_TYPE_BASIC));

    /* 2. C++ STL */
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_VECTOR,       CPP_TYPE_STL));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_BYTE_VECTOR,  CPP_TYPE_STL));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_INT32_VECTOR, CPP_TYPE_STL));
    cppTypeMap.insert(pair<string, CppTypeT>(CPP_STRING,       CPP_TYPE_STL));

    /* 3. other C++ common type */
}

static void init_include_header(ApccGenInfo& genInfo, const string& aidlIntfName, vector<string>& includeHeader)
{
    const vector<string>& importFile = aidl_get_import_list(aidlIntfName);
    for (vector<string>::size_type index = 0; index < importFile.size(); index++)
    {
        string ih = aidl_get_include_header(importFile[index]);
        if (exist_string(includeHeader, ih))
        {
            /* already exist, ignore */
            continue;
        }
        includeHeader.push_back(ih);
    }
}

static void init_create_hal_status_func(const string& aidlHalRoot,
    const string& aidlHalStatusType, vector<FuncType>& createHalStatusFunc)
{
    FuncType funcType;

    funcType.returnType = NDK_SCOPED_ASTATUS;
    funcType.funcName = concat_string("create", aidlHalRoot, "Status");

    funcType.param.push_back({aidlHalStatusType, STATUS_NAME});
    createHalStatusFunc.push_back(funcType);

    funcType.param.push_back({cpp_type_const_ref(CPP_STRING), INFO_NAME});
    createHalStatusFunc.push_back(funcType);
}

static string get_shared_ptr(const string& type)
{
    stringstream ss;
    ss << CPP_SHARED_PTR << '<' << type << '>';
    return ss.str();
}

static string get_std_shared_ptr(const string& type)
{
    stringstream ss;
    ss << STD_NAMESPACE << "::" << CPP_SHARED_PTR << '<' << type << '>';
    return ss.str();
}

static string get_default_hal_status_code(const string& aidlHalStatusType)
{
    vector<string> enumVal;
    string defaultStatusCode = "";

    if (valid_str(aidlHalStatusType) &&
        !is_same_string(aidlHalStatusType, HAL_STATUS_CODE_NAME))
    {
        if (proto_get_enum_val(aidlHalStatusType, enumVal))
        {
            for (auto it: enumVal)
            {
                string name = it;
                // logd("%s: enum name: %s", __func__, name.c_str());
                if (is_same_string(name, STATUS_ERROR_UNKNOWN_NAME) ||
                    is_same_string(name, STATUS_FAILURE_UNKNOWN_NAME) ||
                    contain_suffix(name, STATUS_UNKNOWN_SUFFIX))
                {
                    /* found */
                    return name;
                }
            }
            return *enumVal.rbegin();
        }
    }
    return defaultStatusCode;
}

static string get_aidl_intf_class_name(const string& aidlIntfName)
{
    stringstream ss;
    ss << aidlIntfName << RPC_NAME;
    /* exclude "I" */
    return ss.str().substr(1, string::npos);
}

static string get_aidl_intf_header_file(const string& className)
{
    stringstream ss;
    ss << get_format_string_in_lower(className) << HEADER_FILE_SUFFIX;
    return ss.str();
}

static string get_hal_rpc_callback_type(const string& aidlCallbackName)
{
    if (apcc_is_multi_callback())
    {
        stringstream ss;
        ss << CPP_VECTOR << '<' << get_shared_ptr(aidlCallbackName) << '>';
        return ss.str();
    }
    else
    {
        return get_shared_ptr(aidlCallbackName);
    }
}

static string get_hal_rpc_callback2_type(const string& aidlCallbackName)
{
    return get_shared_ptr(aidlCallbackName);
}

static string get_hal_rpc_class_type(const string& type)
{
    return get_shared_ptr(type);
}

static string get_hal_rpc_inst_static_variable(const string& name)
{
    stringstream ss;
    ss << 's' << name;
    return ss.str();
}

static string get_hal_name_lower(const string& aidlIntfName)
{
    return get_format_string_in_lower(aidl_get_interface_short_name(aidlIntfName));
}

static string get_map_type(const string& keyType, const string& valType)
{
    // e.g. "map<uint16_t, shared_ptr<IHal>>"
    stringstream ss;
    ss << CPP_MAP << '<' << keyType << ", " << valType << '>';
    return ss.str();
}

static bool expose_hal_rpc_server_api(ApccType apccType)
{
    return (apccType == APCC_TYPE_HAL_SERVICE_SOURCE) ||
            (apccType == APCC_TYPE_HAL_IMPL_HEADER) ||
            (apccType == APCC_TYPE_HAL_IMPL_SOURCE) ||
            (apccType == APCC_TYPE_HAL_SERVER_API_HEADER) ||
            (apccType == APCC_TYPE_HAL_RPC_SERVER_SOURCE);
}

static bool is_create_intf_func_name(const string& funcName)
{
    return contain_prefix(funcName, CREATE_NAME) ||
            contain_prefix(funcName, ADD_NAME);
}

static bool is_get_intf_func_name(const string& funcName)
{
    return contain_prefix(funcName, GET_NAME);
}

static bool is_remove_intf_func_name(const string& funcName)
{
    return contain_prefix(funcName, REMOVE_NAME) ||
            contain_prefix(funcName, DELETE_NAME);
}

static void init_aidl_hal_root(ApccGenInfo& genInfo, AidlHalInfo& aidlHalRoot)
{
    AidlHalName& halName = aidlHalRoot.halName;

    // logd("%s: +++", __func__);
    aidlHalRoot.intfName = aidl_get_intf_root();
    // logd("%s: intfName: %s", __func__, aidlHalRoot.intfName.c_str());

    /* halName */
    halName.name = aidl_get_interface_short_name(aidlHalRoot.intfName);
    // logd("%s: hal name: %s", __func__, halName.name.c_str());
    halName.nameUpper = get_format_string_in_upper(halName.name);
    // logd("%s: hal name upper: %s", __func__, halName.nameUpper.c_str());
    halName.nameLower = get_format_string_in_lower(halName.name);
    // logd("%s: halNameLower: %s", __func__, halName.nameLower.c_str());

    aidlHalRoot.headerFile = aidl_get_include_header(genInfo.aidlPackageName, aidlHalRoot.intfName);
    aidlHalRoot.halRpcHeader = apcc_get_gen_file_name(aidlHalRoot.intfName, APCC_TYPE_HAL_RPC_HEADER);
    // logd("%s: ---", __func__);
}

static void init_hal_status_info(ApccGenInfo& genInfo, HalStatusInfo& halStatusInfo)
{
    halStatusInfo.gen = valid_str(aidl_get_hal_status_type());
    halStatusInfo.aidlHalStatusType = aidl_get_hal_status_type();
    halStatusInfo.existHalStatusType = valid_str(halStatusInfo.aidlHalStatusType);
    halStatusInfo.defaultHalStatusCode = get_default_hal_status_code(halStatusInfo.aidlHalStatusType);
    // logd("%s: defaultHalStatusCode: %s", __func__, halStatusInfo.defaultHalStatusCode.c_str());

    // logd("%s: gen: %x, aidlHalStatusType: %s, existHalStatusType: %x", __func__,
    //     halStatusInfo.gen, halStatusInfo.aidlHalStatusType.c_str(), halStatusInfo.existHalStatusType);
    halStatusInfo.halStatusHeader = aidl_get_include_header(genInfo.aidlPackageName, halStatusInfo.aidlHalStatusType);
    // logd("%s: halStatusHeader: %s", __func__, halStatusInfo.halStatusHeader.c_str());
    halStatusInfo.halStatusUsing = aidl_get_using_type(genInfo.aidlPackageName, halStatusInfo.aidlHalStatusType);
    // logd("%s: halStatusUsing: %s", __func__, halStatusInfo.halStatusUsing.c_str());

    halStatusInfo.existCreateHalStatusFunc = apcc_exist_create_hal_status_func();
    init_create_hal_status_func(genInfo.aidlHalRoot.halName.name, halStatusInfo.aidlHalStatusType, halStatusInfo.createHalStatusFunc);
}

static void init_using_type(ApccGenInfo& genInfo, const string& aidlIntfName, vector<string>& usingType)
{
    const vector<string>& importFile = aidl_get_import_list(aidlIntfName);
    for (vector<string>::size_type index = 0; index < importFile.size(); index++)
    {
        string ut = aidl_get_using_type(importFile[index]);
        if (exist_string(usingType, ut))
        {
            /* already exist, ignore */
            continue;
        }
        usingType.push_back(ut);
    }
}

static void init_aidl_intf_using_type(ApccGenInfo& genInfo, vector<string>& usingType)
{
    init_using_type(genInfo, genInfo.aidlIntfName, usingType);
    usingType.push_back(NDK_SCOPED_ASTATUS);
    usingType.push_back((STD_NAMESPACE "::" CPP_SHARED_PTR));
}

static void init_hal_msg_def(const string& halNameUpper, const vector<string>& halMsg, vector<string>& halMsgDef)
{
    for (vector<string>::size_type index = 0; index < halMsg.size(); index++)
    {
        /* e.g. "HAL" + "SampleReq" -> "HAL_SAMPLE_REQ" */
        const string msg = get_format_string_in_upper(halMsg[index]);
        string msgDef = concat_string(halNameUpper, '_', msg);
        halMsgDef.push_back(msgDef);
    }
}

static string get_msg_param_type(ApccGenInfo& genInfo, const string& funcName, const string& msgType)
{
    string suffix = update_string(genInfo.halName, genInfo.aidlHalRoot.halName.name, "");

    stringstream ss;
    ss << funcName << msgType << suffix << PARAM_STRUCT_SUFFIX;

    string str = ss.str();
    convert_first_char_to_upper(str);
    return str;
}

static string get_msg_param_type(ApccGenInfo& genInfo, const string& funcName,
    const string& msgType, const vector<AidlParameter>& param)
{
    return (param.size() >= 2) ? get_msg_param_type(genInfo, funcName, msgType) : "";
}

static string get_ind_param_type(ApccGenInfo& genInfo, const string& funcName)
{
    return get_msg_param_type(genInfo, funcName, PROTO_MESSAGE_IND);
}

static string get_ind_param_type(ApccGenInfo& genInfo, const string& funcName, const vector<AidlParameter>& param)
{
    return get_msg_param_type(genInfo, funcName, PROTO_MESSAGE_IND, param);
}

static string get_req_param_type(ApccGenInfo& genInfo, const string& funcName, const vector<AidlParameter>& param)
{
    return get_msg_param_type(genInfo, funcName, PROTO_MESSAGE_REQ, param);
}

static void init_param_name(const vector<FuncParameter>& param, vector<string>& paramName)
{
    for (auto it: param)
    {
        paramName.push_back(it.name);
    }
}

static bool is_cpp_type(const string& cppType, const string& matchedType)
{
    return !cppType.compare(matchedType);
}

static bool is_cpp_void(const string& cppType)
{
    return is_cpp_type(cppType, CPP_VOID);
}

static bool is_empty_param(const vector<FuncParameter>& param)
{
    if (param.empty())
        return true;

    for (auto it: param)
    {
        if (!aidl_is_interface(it.type))
            return false;
    }
    return true;
}

static string get_var_int(const string& var)
{
    stringstream ss;
    ss << var << '_';
    return ss.str();
}

static string get_param_string(const vector<FuncParameter>& param)
{
    if (!param.empty())
    {
        stringstream ss;
        vector<FuncParameter>::size_type size = param.size();
        for (vector<FuncParameter>::size_type index = 0; index < size - 1; index++)
        {
            ss << param[index].type << " " << param[index].name << ", ";
        }
        ss << param[size-1].type << " " << param[size-1].name;
        return ss.str();
    }
    return "";
}

static string get_comment_hal_msg(const string& halNameLower, const string& comment)
{
    string str = comment;
    replace_string(str, "hal", halNameLower);
    return str;
}

static void add_unique_str(const string& str, vector<string>& vec)
{
    bool unique = true;
    if (!str.empty())
    {
        for (auto it: vec)
        {
            if (is_same_string(str, it))
            {
                unique = false;
                break;
            }
        }
        if (unique)
        {
            vec.push_back(str);
        }
    }
}

static void add_unique_str(const vector<string>& str, vector<string>& vec)
{
    if (!str.empty())
    {
        for (auto it: str)
        {
            add_unique_str(it, vec);
        }
    }
}

static void add_hal_common_msg_using_type(const string& aidlPackageName, const string& aidlIntfName,
    const vector<string>& halCommonMsg, stringstream& ss)
{
    vector<string>::size_type size = halCommonMsg.size();
    if (valid_str(aidlPackageName) &&
        valid_str(aidlIntfName) &&
        (size > 1))
    {
        /* Ignore the 1st message which is for AIDL intf itself */
        for (vector<string>::size_type index = 1; index < halCommonMsg.size(); index++)
        {
            const string& msgName = halCommonMsg[index];
            string ut = aidl_get_using_type(aidlPackageName, aidlIntfName, msgName);
            apcc_add_using(msgName, ut, ss);
        }
        ss << endl;
    }
}

static void add_hal_enum_type_using_type(const string& aidlPackageName, const string& aidlIntfName,
    const vector<string>& halEnumType, stringstream& ss)
{
    if (valid_str(aidlPackageName) &&
        valid_str(aidlIntfName) &&
        !halEnumType.empty())
    {
        for (auto it: halEnumType)
        {
            string ut = aidl_get_using_type(aidlPackageName, aidlIntfName, it);
            apcc_add_using(it, ut, ss);
        }
        ss << endl;
    }
}

static void add_unique_using_type(const vector<string>& str, vector<string>& usingType)
{
    for (auto it: str)
    {
        /* ignore "ndk::ScopedAStatus" & "std::shared_ptr" */
        if (contain_string(it, SCOPED_ASTATUS) ||
            contain_string(it, CPP_SHARED_PTR))
        {
            continue;
        }
        add_unique_str(it, usingType);
    }
}

static void add_unique_using_type(const vector<CppTypeInfo>& cppTypeInfo, vector<string>& usingType)
{
    for (auto it: cppTypeInfo)
    {
        /* ignore "ndk::ScopedAStatus" & "std::shared_ptr" */
        if (contain_string(it.usingType, SCOPED_ASTATUS) ||
            contain_string(it.usingType, CPP_SHARED_PTR))
        {
            continue;
        }
        add_unique_str(it.usingType, usingType);
    }
}

static void add_unique_include_header(const vector<CppTypeInfo>& cppTypeInfo, vector<string>& includeHeader)
{
    for (auto it: cppTypeInfo)
    {
        /* ignore "ndk::ScopedAStatus" & "std::shared_ptr" */
        if (contain_string(it.usingType, SCOPED_ASTATUS) ||
            contain_string(it.usingType, CPP_SHARED_PTR))
        {
            continue;
        }
        add_unique_str(it.includeHeader, includeHeader);
    }
}

static string class_func_name(const string& className, const string& funcName)
{
    stringstream ss;
    ss << className << "::" << funcName;
    return ss.str();
}

static string hal_rpc_service_name(const string& halName)
{
    stringstream ss;
    ss << halName << RPC_NAME << SERVICE_NAME;
    return ss.str();
}

static void set_common_func(const string& funcName, const string& returnType, FuncType& funcType)
{
    funcType.param.clear();

    funcType.returnType = returnType;
    funcType.funcName = funcName;
}

static string create_cpp_type_optional(const string& cppType)
{
    stringstream ss;
    ss << CPP_OPTOINAL << '<' << cppType << '>';
    return ss.str();
}

static string create_cpp_type_shared_ptr(const string& cppType)
{
    stringstream ss;
    ss << CPP_CONST << " " << CPP_SHARED_PTR << '<' << cppType << '>';
    return ss.str();
}

static string create_cpp_type_shared_ptr_ref(const string& cppType)
{
    return concat_string(create_cpp_type_shared_ptr(cppType), CPP_REF);
}

static string create_std_cpp_type_shared_ptr_ref(const string& cppType)
{
    stringstream ss;
    ss << CPP_CONST << " " << STD_NAMESPACE << "::" << CPP_SHARED_PTR << '<' << cppType << '>' << CPP_REF;
    return ss.str();
}

static void update_cpp_type_ref(string& cppType, const string& typeName = "")
{
    if (aidl_is_interface(cppType))
    {
        cppType = create_cpp_type_shared_ptr_ref(cppType);
    }
    else if ((apcc_get_cpp_type(cppType) != CPP_TYPE_BASIC) &&
            !apcc_is_enum_type(cppType) &&
            !apcc_is_out_param_type(cppType, typeName))
    {
        cppType = cpp_type_const_ref(cppType);
    }
}

static void copy_func_param(const vector<FuncParameter>& srcParam, vector<FuncParameter>& dstParam)
{
    dstParam.clear();
    for (auto it: srcParam)
    {
        FuncParameter param = it;
        update_cpp_type_ref(param.type, param.name);
        dstParam.push_back(param);
    }
}

static void copy_func_param_generic(const vector<FuncParameter>& srcParam, vector<FuncParameter>& dstParam)
{
    dstParam.clear();
    for (auto it: srcParam)
    {
        FuncParameter param = it;
        if (apcc_is_out_param_type(param.type, param.name))
        {
            /* ignore AIDL out param */
            logd("%s: ignore aidl intf param (%s %s)", __func__, param.type.c_str(), param.name.c_str());
            continue;
        }
        if (aidl_is_interface(param.type))
        {
            if (aidl_is_callback(param.type))
            {
                /* ignore AIDL callback */
                logd("%s: ignore aidl callback param (%s %s)", __func__, param.type.c_str(), param.name.c_str());
                continue;
            }
            else
            {
                param.type = INSTANCE_ID_TYPE;
                param.name = INSTANCE_ID_NAME;
            }
        }
        else
        {
            update_cpp_type_ref(param.type, param.name);
        }
        dstParam.push_back(param);
    }
}

static void update_upstream_cpp_type_ref(ApccGenInfo& genInfo, const string& funcName,
    const vector<FuncParameter>& srcParam, vector<FuncParameter>& dstParam)
{
    dstParam.clear();

    /* The 1st parameter: "uint8_t* msg_data" */
    dstParam.push_back({cpp_type_const(CPP_UINT8_POINTER), MSG_DATA_NAME});

    /* The 2nd parameter: "size_t msg_length" */
    dstParam.push_back({CPP_SIZE_T, MSG_LENGTH_NAME});

    /* The 3rd parameter: "T& param" */
    if (!srcParam.empty())
    {
        if (srcParam.size() == 1)
        {
            dstParam.push_back(srcParam[0]);
        }
        else
        {
            dstParam.push_back({cpp_type_ref(get_ind_param_type(genInfo, funcName)), PARAM_VAR_NAME});
        }
    }
}

static string create_cpp_type_shared_ptr_ptr(const string& cppType)
{
    return cpp_type_ptr(get_shared_ptr(cppType));
}

static string get_stl_include_header(const string& baseTypeName)
{
    return !baseTypeName.compare(CPP_PAIR) ||
        !baseTypeName.compare(CPP_SHARED_PTR) ?
        "" : baseTypeName;
}

static string get_stl_using_type(const string& baseTypeName)
{
    stringstream ss;
    ss << STD_NAMESPACE << "::" << baseTypeName;
    return ss.str();
}

static string get_stl_type_ref(const string& baseTypeName)
{
    stringstream ss;
    ss << CPP_CONST << ' ' << baseTypeName << '&';
    return ss.str();
}

static bool exist_cpp_type_info(const vector<CppTypeInfo>& cppTypeInfo, const string& includeHeader)
{
    for (auto it: cppTypeInfo)
    {
        if (is_same_string(it.includeHeader, includeHeader))
            return true;
    }
    return false;
}

static void store_stl_type(const string& cppTypeName, const char* baseTypeName, vector<CppTypeInfo>& cppTypeInfo)
{
    CppTypeInfo cti;

    if (contain_string(cppTypeName, baseTypeName))
    {
        cti.name = cppTypeName;
        cti.type = CPP_TYPE_STL;

        cti.includeHeader = get_stl_include_header(baseTypeName);
        cti.usingType = get_stl_using_type(baseTypeName);
        cti.typeRef = get_stl_type_ref(baseTypeName);

        if (exist_cpp_type_info(cppTypeInfo, cti.includeHeader))
            return;

        // logd("%s: cppTypeName: %s, baseTypeName: %s, includeHeader: %s", __func__,
        //     cppTypeName.c_str(), baseTypeName, cti.includeHeader.c_str());
        cppTypeInfo.push_back(cti);
    }
}

static string get_func_param_name(const FuncType& funcType)
{
    stringstream ss;
    vector<FuncParameter>::size_type size = funcType.param.size();
    if (size >= 1)
    {
        ss << funcType.param[0].name;
        if (size >= 2)
        {
            for (vector<FuncParameter>::size_type index = 1; index < size; index++)
            {
                ss << ", " << funcType.param[index].name;
            }
        }
    }
    return ss.str();
}

static string get_func_name(const string& className, const string& funcName)
{
    stringstream ss;
    if (valid_str(className))
    {
        ss << className << "::" << funcName;
    }
    else
    {
        ss << funcName;
    }
    return ss.str();
}

static string get_static_func_prefix(const string& prefix)
{
    stringstream ss;
    ss << prefix << CPP_STATIC << " ";
    return ss.str();
}

static string get_hal_mode_property_name(const string& aidlPackageName, const string& halNameLower)
{
    stringstream ss;
    ss << HAL_MODE_PROPERTY_NAME_PREFIX;
    if (contain_string(aidlPackageName, AIDL_HAL_PACKAGE_PREFIX))
    {
        string module = get_sub_string(aidlPackageName, AIDL_HAL_PACKAGE_PREFIX, '.');
        // logd("%s: module: %s", __func__, module.c_str());
        if (!is_empty_string(module))
        {
            size_t pos = module.find_first_of('.');
            if (pos != string::npos)
            {
                ss << module.substr(0, pos);
            }
            else
            {
                ss << module;
            }
        }
        else
        {
            ss << halNameLower;
        }
    }
    else
    {
        ss << halNameLower;
    }
    ss << HAL_MODE_PROPERTY_NAME_SUFFIX;
    return ss.str();
}

static const string& search_string(const vector<string>& v1, const vector<string>& v2,
    const vector<string>& v3, const string& key)
{
    const static string sEmptyString = "";

    const string& str = get_string(v1, key);
    if (valid_str(str))
        return str;

    const string& str2 = get_string(v2, key);
    if (valid_str(str2))
        return str2;

    const string& str3 = get_string(v3, key);
    if (valid_str(str3))
        return str3;

    return sEmptyString;
}

static const string& get_common_include_header(ApccGenInfo& genInfo, const string& cppTypeName)
{
    return search_string(genInfo.aidlIntfIncludeHeader, genInfo.aidlCallbackIncludeHeader,
                         genInfo.aidlCallback2IncludeHeader, cppTypeName);
}

static const string& get_common_using_type(ApccGenInfo& genInfo, const string& cppTypeName)
{
    return search_string(genInfo.aidlIntfUsingType, genInfo.aidlCallbackUsingType,
                         genInfo.aidlCallback2UsingType, cppTypeName);
}

static string get_common_type_ref(ApccGenInfo& genInfo, const string& cppTypeName)
{
    stringstream ss;
    if (!get_string(genInfo.aidlEnumType, cppTypeName).empty())
    {
        return cppTypeName;
    }
    else
    {
        ss << CPP_CONST << ' ' << cppTypeName << '&';
        return ss.str();
    }
}

static void store_common_type(ApccGenInfo& genInfo, const string& cppTypeName, vector<CppTypeInfo>& cppTypeInfo)
{
    CppTypeInfo cti;

    if (exist_string(genInfo.aidlIntfUsingType, cppTypeName) ||
        exist_string(genInfo.aidlCallbackUsingType, cppTypeName))
    {
        return;
    }

    cti.name = cppTypeName;
    cti.type = CPP_TYPE_COMMON;

    cti.includeHeader = get_common_include_header(genInfo, cppTypeName);
    cti.usingType = get_common_using_type(genInfo, cppTypeName);
    cti.typeRef = get_common_type_ref(genInfo, cppTypeName);

    if (exist_cpp_type_info(cppTypeInfo, cti.includeHeader))
        return;

    // logd("%s: cppTypeName: %s, includeHeader: %s", __func__, cppTypeName.c_str(), cti.includeHeader.c_str());
    cppTypeInfo.push_back(cti);
}

static void store_cpp_type(ApccGenInfo& genInfo, const string& cppTypeName, CppTypeT cppType,
    const string& itemTypeName, CppTypeT itemType, vector<CppTypeInfo>& cppTypeInfo)
{
    CppTypeInfo cti;

    if (cppType == CPP_TYPE_BASIC)
    {
        /* not store basic type */
        return;
    }

    if (cppType == CPP_TYPE_STL)
    {
        store_stl_type(cppTypeName, CPP_ARRAY, cppTypeInfo);
        store_stl_type(cppTypeName, CPP_STRING, cppTypeInfo);
        store_stl_type(cppTypeName, CPP_VECTOR, cppTypeInfo);
        store_stl_type(cppTypeName, CPP_MAP, cppTypeInfo);
        store_stl_type(cppTypeName, CPP_OPTOINAL, cppTypeInfo);
        store_stl_type(cppTypeName, CPP_PAIR, cppTypeInfo);
        store_stl_type(cppTypeName, CPP_SHARED_PTR, cppTypeInfo);
        if (itemType == CPP_TYPE_COMMON)
        {
            store_common_type(genInfo, itemTypeName, cppTypeInfo);
        }
    }
    else if (cppType == CPP_TYPE_COMMON)
    {
        store_common_type(genInfo, cppTypeName, cppTypeInfo);
    }
}

static void get_cpp_type(const string& aidlType, string& cppTypeName, CppTypeT& cppType)
{
    ApccGenInstance *inst = &sApccGenInstance;
    map<string, CppType>::iterator it = inst->aidlTypeMap.find(aidlType);
    if (it != inst->aidlTypeMap.end())
    {
        cppTypeName = it->second.name;
        cppType = it->second.type;
    }
    else
    {
        cppTypeName = aidlType;
        cppType = CPP_TYPE_COMMON;
    }
}

static CppTypeT get_cpp_type(const string& cppTypeName)
{
    ApccGenInstance *inst = &sApccGenInstance;
    map<string, CppTypeT>::iterator it = inst->cppTypeMap.find(cppTypeName);
    return (it != inst->cppTypeMap.end()) ? it->second : CPP_TYPE_COMMON;
}

static string get_cpp_array_type(const string& itemType, const string& arraySize)
{
    stringstream ss;
    ss << CPP_ARRAY << '<' << itemType << ", " << arraySize << '>';
    return ss.str();
}

static string get_cpp_vector_type(const string& itemType)
{
    stringstream ss;
    ss << CPP_VECTOR << '<' << itemType << '>';
    return ss.str();
}

void map_2_cpp_type(const string& aidlType, string& cppTypeName, CppTypeT& cppType,
    string& itemTypeName, CppTypeT& itemType)
{
    get_cpp_type(aidlType, cppTypeName, cppType);
    if ((cppType == CPP_TYPE_COMMON) ||
        (cppType == CPP_TYPE_STL))
    {
        if (aidl_is_array(aidlType))
        {
            string itn = aidl_get_array_item_type(aidlType);
            if (is_cpp_type(itn, AIDL_TYPE_BYTE))
            {
                /* map item to "uint8_t" for AIDL byte array, instead of "int8_t" */
                itemTypeName = CPP_UINT8;
                itemType = CPP_TYPE_BASIC;
                // logd("%s: AIDL_TYPE_BYTE, itemTypeName: %s, itemType: %d", __func__, itemTypeName.c_str(), itemType);
            }
            else
            {
                get_cpp_type(itn, itemTypeName, itemType);
            }

            string arraySize = aidl_get_array_size(aidlType);
            // logd("%s: array: %s, size: %s", __func__, aidlType.c_str(), arraySize.c_str());

            cppTypeName = valid_str(arraySize) ?
                            get_cpp_array_type(itemTypeName, arraySize) :
                            get_cpp_vector_type(itemTypeName);
            // logd("%s: cppTypeName: %s", __func__, cppTypeName.c_str());

            cppType = CPP_TYPE_STL;
        }
    }
    else
    {
        itemTypeName = "";
    }
}

static string map_2_hal_common_func_name(const string& halName, const string& aidlFuncName)
{
    string str = aidlFuncName;
    convert_first_char_to_upper(str);
    /* e.g. HalFoo */
    return halName + str;
}

static string map_2_hal_msg_func_name(const string& halCommonFuncName, const string& halName,
    const string& pattern, const string& suffix)
{
    string str = update_string(halCommonFuncName, halName, halName + pattern);

    stringstream ss;
    /* e.g. HalFoo + Hal + Serialize + Req -> HalSerializeFooReq */
    ss << str << suffix;
    return ss.str();
}

static string map_2_hal_cfm_param(ApccGenInfo& genInfo, const string& halCommonFuncName, const string& halName)
{
    string str = update_string(halCommonFuncName, halName, "");
    string suffix = update_string(halName, genInfo.aidlHalRoot.halName.name, "");

    stringstream ss;
    /* e.g. HalFoo + Hal -> FooCfmParam */
    ss << str << PROTO_MESSAGE_CFM << suffix << PARAM_STRUCT_SUFFIX;
    return ss.str();
}

static void init_func_type(const string& returnType, const string& funcName, const vector<FuncParameter>& param, FuncType& funcType)
{
    funcType.returnType = returnType;
    funcType.funcName = funcName;
    funcType.param = param;
}

static void init_func_param(const string& type, const string& name, FuncParameter& funcParam)
{
    funcParam.type = type;
    funcParam.name = name;
}

void map_2_hal_common_func_type(ApccGenInfo& genInfo, const AidlInterface& aidlIntf,
    vector<CppTypeInfo>& intfReturnType, vector<CppTypeInfo>& intfParamType,
    const string& halName, FuncType& halCommonFuncType)
{
    FuncParameter funcParam;
    string cppTypeName;
    CppTypeT cppType;
    string itemTypeName;
    CppTypeT itemType;

    /* 1. return type */
    map_2_cpp_type(aidlIntf.returnType, cppTypeName, cppType, itemTypeName, itemType);
    store_cpp_type(genInfo, cppTypeName, cppType, itemTypeName, itemType, intfReturnType);
    halCommonFuncType.returnType = cppTypeName;

    /* 2. func name: concat with hal name */
    halCommonFuncType.funcName = map_2_hal_common_func_name(halName, aidlIntf.funcName);
    // logd("%s: funcName: %s, ", __func__, halCommonFuncType.funcName.c_str());

    /* 3. param: convert aidl parameter into func parameter */
    for (vector<AidlParameter>::size_type index = 0; index < aidlIntf.param.size(); index++)
    {
        const AidlParameter& aidlParam = aidlIntf.param[index];
        // string paramName = is_same_string(aidlParam.name, MSG_NAME) ? MSG_ALIAS_NAME : aidlParam.name;
        string paramName = aidlParam.name;

        map_2_cpp_type(aidlParam.type, cppTypeName, cppType, itemTypeName, itemType);
        if (!IS_AIDL_PARAM_IN(aidlParam.dir))
        {
            cppTypeName = cpp_type_ptr(cppTypeName);
            paramName = aidl_get_out_param_name(paramName);
        }
        else if (aidlParam.nullable)
        {
            cppTypeName = create_cpp_type_optional(cppTypeName);
        }
        store_cpp_type(genInfo, cppTypeName, cppType, itemTypeName, itemType, intfParamType);

        // logd("%s: param { index: %d/%d, (%s %s) }", __func__, index,
        //     aidlIntf.param.size(), cppTypeName.c_str(), paramName.c_str());
        halCommonFuncType.param.push_back({cppTypeName, paramName});
    }
}

static void init_hal_common_func(ApccGenInfo& genInfo, const string& aidlIntfName, const string& halName,
    vector<CppTypeInfo>& intfReturnType, vector<CppTypeInfo>& intfParamType, vector<FuncType>& halCommonFunc)
{
    logd("%s: aidlIntfName: %s", __func__, aidlIntfName.c_str());
    const vector<AidlInterface>& aidlIntfList = aidl_get_interface_list(aidlIntfName);
    logd("%s: aidlIntfList size: %d", __func__, aidlIntfList.size());

    halCommonFunc.clear();

    if (!aidlIntfList.empty())
    {
        for (vector<AidlInterface>::size_type index = 0; index < aidlIntfList.size(); index++)
        {
            FuncType funcType;
            map_2_hal_common_func_type(genInfo, aidlIntfList[index],
                intfReturnType, intfParamType, halName, funcType);

            // logd("%s: aidl intf %d/%d, funcName: %s, param size: %d", __func__, index, 
            //     aidlIntfList.size(), funcType.funcName.c_str(), funcType.param.size());
            // apcc_dump_func_type(funcType);
            halCommonFunc.push_back(funcType);
        }
    }
}

void map_2_hal_lib_func_type(const FuncType& halCommonFuncType, FuncType& funcType)
{
    string cppType;

    funcType.param.clear();

    /* 1. return type: always set as ScopedAStatus */
    funcType.returnType = SCOPED_ASTATUS;

    /* 2. func name  */
    funcType.funcName = halCommonFuncType.funcName;

    /* 3. param */
    // logd("%s: funcName: %s, halCommonFuncType.param size: %d", __func__, funcType.funcName.c_str(), halCommonFuncType.param.size());
    copy_func_param(halCommonFuncType.param, funcType.param);

    // logd("%s: halCommonFuncType.returnType: %s", __func__, halCommonFuncType.returnType.c_str());
    /* 4. param: convert generic return type into the last func parameter pointer */
    if (!is_cpp_void(halCommonFuncType.returnType))
    {
        FuncParameter fp;
        if (aidl_is_interface(halCommonFuncType.returnType))
        {
            // logd("%s: last param is interface (type: %s)", __func__, halCommonFuncType.returnType.c_str());
            cppType = cpp_type_ptr(CPP_UINT16);
        }
        else
        {
            cppType = cpp_type_ptr(halCommonFuncType.returnType);
        }
        init_func_param(cppType, RESULT_NAME, fp);
        // logd("%s: last param (type: %s, name: %s)", __func__, fp.type.c_str(), fp.name.c_str());
        funcType.param.push_back(fp);
    }
}

void map_2_hal_rpc_func_type(const FuncType& halCommonFuncType, FuncType& funcType)
{
    string cppType;
    const string& returnType = halCommonFuncType.returnType;

    funcType.param.clear();

    /* 1. return type: always set as ScopedAStatus */
    funcType.returnType = SCOPED_ASTATUS;

    /* 2. func name  */
    funcType.funcName = halCommonFuncType.funcName;

    /* 3. param */
    copy_func_param(halCommonFuncType.param, funcType.param);

    /* 4. param: convert generic return type into the last func parameter pointer */
    if (!is_cpp_void(returnType))
    {
        FuncParameter fp;

        cppType = aidl_is_interface(returnType) ?
                    create_cpp_type_shared_ptr_ptr(returnType) :
                    cpp_type_ptr(returnType);
        init_func_param(cppType, AIDL_RETURN_NAME, fp);
        funcType.param.push_back(fp);
    }
}

void map_2_hal_rpc_internal_func_type(bool isValidateFuncSupported, const FuncType& halRpcFuncType,
    const FuncType& halCommonFuncType, bool returnPair, FuncType& funcType)
{
    string cppType;
    vector<FuncParameter> funcParam;
    const string& returnType = halCommonFuncType.returnType;

    funcType.param.clear();

    /* 1. return type: already set */

    /* 2. func name  */
    funcType.funcName = halCommonFuncType.funcName;

    /* 3. param (part1) */
    // logd("%s: funcName: %s, isValidateFuncSupported: %x", __func__, halRpcFuncType.funcName.c_str(), isValidateFuncSupported);
    if (isValidateFuncSupported &&
        apcc_is_validate_func_with_lock(halRpcFuncType.funcName))
    {
        // logd("%s: add param %s", __func__, VALIDATE_AND_CALL_WITH_LOCK_TYPE);
        funcType.param.push_back({VALIDATE_AND_CALL_WITH_LOCK_TYPE, LOCK_NAME});
    }

    /* 3. param (part2) */
    copy_func_param(halCommonFuncType.param, funcParam);
    for (auto it: funcParam)
    {
        funcType.param.push_back(it);
    }

    /* 3. param (part3): convert generic return type into the last func parameter pointer */
    if (!is_cpp_void(returnType) && !returnPair)
    {
        funcType.param.push_back({cpp_type_ptr(returnType), AIDL_RETURN_NAME});
    }
}

void map_2_hal_req_func_type(const string& halName, const FuncType& halCommonFuncType,
    const string& suffix, FuncType& funcType)
{
    FuncParameter funcParam;

    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = map_2_hal_msg_func_name(halCommonFuncType.funcName, halName, HAL_MSG_SERIALIZE, suffix);

    /* 3. param */
    copy_func_param_generic(halCommonFuncType.param, funcType.param);
    /* last param is output for payload serialized in "vector<uint8_t>& payload" */
    funcType.param.push_back({CPP_BYTE_VECTOR_REF, PAYLOAD_NAME});
}

void map_2_hal_upstream_req_func_type(const string& halName, const FuncType& halCommonFuncType,
    const string& suffix, FuncType& funcType)
{
    FuncParameter funcParam;

    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = map_2_hal_msg_func_name(halCommonFuncType.funcName, halName, HAL_MSG_PARSE, suffix);

    /* 3. param */
    funcType.param.clear();
    /* The 1st parameter: "uint8_t* msg_data" */
    funcType.param.push_back({CPP_UINT8_POINTER, MSG_DATA_NAME});
    /* The 2nd parameter: "size_t msg_length" */
    funcType.param.push_back({CPP_SIZE_T, MSG_LENGTH_NAME});
    for (auto it: halCommonFuncType.param)
    {
        FuncParameter param = it;
        param.type = cpp_type_ref(param.type);
        funcType.param.push_back(param);
    }
}

static void init_hal_req_func(ApccGenInfo& genInfo, const vector<FuncType>& halCommonFunc,
    bool downstream, vector<FuncType>& halReqFunc)
{
    FuncType funcType;
    string suffix = PROTO_MESSAGE_REQ;

    for (vector<string>::size_type index = 0; index < halCommonFunc.size(); index++)
    {
        if (downstream)
            map_2_hal_req_func_type(genInfo.halName, halCommonFunc[index], suffix, funcType);
        else
            map_2_hal_upstream_req_func_type(genInfo.aidlCallbackResultHalName.name, halCommonFunc[index], suffix, funcType);

        halReqFunc.push_back(funcType);
    }
}

void map_2_hal_ind_func_type(ApccGenInfo& genInfo, const string& halName,
    const FuncType& halCommonCallbackType, FuncType& funcType)
{
    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = map_2_hal_msg_func_name(halCommonCallbackType.funcName, halName,
                                                HAL_MSG_PARSE, PROTO_MESSAGE_IND);

    /* 3. param */
    update_upstream_cpp_type_ref(genInfo, halCommonCallbackType.funcName, halCommonCallbackType.param, funcType.param);
}

static void init_hal_ind_func(ApccGenInfo& genInfo, const vector<FuncType>& halCommonCallback,
    const AidlHalName aidlCallbackHalName, vector<FuncType>& halIndFunc)
{
    FuncType funcType;

    // logd("%s: halCommonCallback size: %d", __func__, halCommonCallback.size());
    for (vector<string>::size_type index = 0; index < halCommonCallback.size(); index++)
    {
        map_2_hal_ind_func_type(genInfo, aidlCallbackHalName.name, halCommonCallback[index], funcType);
        // logd("%s: returnType: %s, funcName: %s", __func__, funcType.returnType.c_str(), funcType.funcName.c_str());
        halIndFunc.push_back(funcType);
    }
}

void map_2_hal_cfm_func_type(ApccGenInfo& genInfo, const string& halName, const string& halStatusCode,
    const FuncType& halCommonFuncType, const string& suffix, FuncType& funcType)
{
    // string halStatusType = !is_empty_string(halStatusCode) ? halStatusCode : CPP_INT32;
    string halStatusType = CPP_INT32;

    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = map_2_hal_msg_func_name(halCommonFuncType.funcName, halName, HAL_MSG_PARSE, suffix);

    /* 3. param */
    funcType.param.clear();
    /* The 1st parameter: "uint8_t* msg_data" */
    funcType.param.push_back({cpp_type_const(CPP_UINT8_POINTER), MSG_DATA_NAME});
    /* The 2nd parameter: "size_t msg_length" */
    funcType.param.push_back({CPP_SIZE_T, MSG_LENGTH_NAME});
    /* The 3rd parameter: "HalCfmParam& param" */
    funcType.param.push_back({cpp_type_ref(map_2_hal_cfm_param(genInfo, halCommonFuncType.funcName, halName)), PARAM_VAR_NAME});

    /* The 4th (or more) param for AIDL intf out param */
    for (auto it: halCommonFuncType.param)
    {
        if (apcc_is_out_param_type(it.type, it.name))
        {
            funcType.param.push_back({cpp_type_ref(apcc_get_out_param_type(it.type)), it.name});
        }
    }
}

void map_2_hal_downstream_cfm_func_type(const string& halName, const string& halStatusCode,
    const FuncType& halCommonFuncType, const string& suffix, FuncType& funcType)
{
    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = map_2_hal_msg_func_name(halCommonFuncType.funcName, halName, HAL_MSG_SERIALIZE, suffix);

    /* 3. param */
    funcType.param.clear();
    /* The 1st parameter: "int32_t status" */
    funcType.param.push_back({CPP_INT32, STATUS_NAME});
    /* The 2nd parameter: "const string& info" */
    funcType.param.push_back({cpp_type_const_ref(CPP_STRING), INFO_NAME});
    if (!is_empty_string(halCommonFuncType.returnType))
    {
        /* The 3rd parameter: "const T& _aidl_return" */
        funcType.param.push_back({cpp_type_const_ref(halCommonFuncType.returnType), AIDL_RETURN_NAME});
    }
    /* The 4th parameter: "vector<uint8_t>& payload" */
    funcType.param.push_back({CPP_BYTE_VECTOR_REF, PAYLOAD_NAME});
}

static void init_hal_cfm_func(ApccGenInfo& genInfo, const vector<FuncType>& halCommonFunc,
    bool upstream, vector<FuncType>& halCfmFunc)
{
    FuncType funcType;
    string suffix = PROTO_MESSAGE_CFM;

    for (vector<string>::size_type index = 0; index < halCommonFunc.size(); index++)
    {
        if (upstream)
            map_2_hal_cfm_func_type(genInfo, genInfo.halName, genInfo.halStatusInfo.aidlHalStatusType,
                halCommonFunc[index], suffix, funcType);
        else
            map_2_hal_downstream_cfm_func_type(genInfo.aidlCallbackResultHalName.name,
                genInfo.halStatusInfo.aidlHalStatusType, halCommonFunc[index], suffix, funcType);

        halCfmFunc.push_back(funcType);
    }
}

string get_hal_cfm_param_name(const string& funcName, const string& defaultCfmParamName)
{
    stringstream ss;
    /* e.g. "HalFooCfmParam: HalCfmParam" */
    ss << funcName << HAL_CFM_PARAM_SUFFIX << ": " << defaultCfmParamName;
    return ss.str();
}

void add_hal_cfm_struct(const string& funcName, const string& defaultCfmParamName,
    const string& fieldType, const string& fieldName, vector<StructType> halCfmStruct)
{
    StructType structType;
    FuncParameter param;

    /* 1. struct name */
    structType.name = get_hal_cfm_param_name(funcName, defaultCfmParamName);

    /* 2. struct field */
    param.type = fieldType;
    param.name = fieldName;
    structType.field.push_back(param);

    halCfmStruct.push_back(structType);
}

static void init_hal_cfm_struct(ApccGenInfo& genInfo, vector<StructType> halCfmStruct)
{
    for (vector<CppTypeInfo>::size_type index = 0; index < genInfo.halResultInfo.size(); index++)
    {
        const string& funcName = genInfo.halCommonFunc[index].funcName;
        CppTypeInfo cti = genInfo.aidlIntfReturnType[index];
        if (cti.name.compare(CPP_VOID))
        {
            add_hal_cfm_struct(funcName, genInfo.halCfmParam, cti.name, RESULT_NAME, halCfmStruct);
        }
    }
}

static void get_hal_rpc_namespace(const string& aidlPackageName, vector<string>& halRpcNamespace)
{
    string rpcNameLower = RPC_NAME;
    transform_string_to_lower(rpcNameLower);

    aidl_get_hal_namespace(aidlPackageName, halRpcNamespace);

    if (!halRpcNamespace.empty() &&
        !is_same_string(*(halRpcNamespace.crbegin()), rpcNameLower))
    {
        halRpcNamespace.push_back(rpcNameLower);
    }
}

static string get_hal_header_file_name(ApccGenInfo& genInfo, const string& headerFileName)
{
    stringstream ss;
    vector<string>::size_type size = genInfo.halNamespace.size();

    if (is_empty_string(headerFileName))
        return "";

    if (size >= 1)
    {
        ss << genInfo.halNamespace[0];
        if (size >= 2)
        {
            for (vector<string>::size_type index = 1; index < size; index++)
            {
                ss << '/' << genInfo.halNamespace[index];
            }
        }
    }
    ss << '/' << headerFileName << HEADER_FILE_SUFFIX;
    return ss.str();
}

static string get_hal_name(const string& aidlIntfName)
{
    string str = aidlIntfName;
    if (!is_empty_string(str) && (str.at(0) == 'I'))
    {
        return str.substr(1, string::npos);
    }
    return str;
}

static void init_hal_payload_struct(const string& halPayloadName, StructType& halPayloadStruct)
{
    halPayloadStruct.name = halPayloadName;

    /* 1. uint16_t message_type; */
    halPayloadStruct.field.push_back({CPP_UINT16, MESSAGE_TYPE_NAME});

    /* 2. void *payload; */
    halPayloadStruct.field.push_back({CPP_VOID_POINTER, PAYLOAD_NAME});
}

static string get_is_hal_msg_macro(const string& halNameUpper, const string& msgSuffix)
{
    stringstream ss;
    ss << "IS_" << halNameUpper << '_' << msgSuffix;
    return ss.str();
}

static string get_class_var(const string& className)
{
    string classVar = className;
    convert_first_char_to_lower(classVar);
    return classVar;
}

static string get_hal_rpc_callback_var_name(ApccGenInfo& genInfo)
{
    return valid_str(genInfo.aidlCallback2Name) || valid_str(genInfo.aidlCallbackResultName) ?
            get_var_int(get_format_string_in_lower(get_hal_name(genInfo.aidlCallbackName))) :
            (apcc_is_multi_callback() ? CALLBACKS_VAR_NAME : CALLBACK_VAR_NAME);
}

static void init_hal_rpc_class_info(ApccGenInfo& genInfo, HalRpcClassInfo& halRpcClassInfo)
{
    string callbackVarName = get_hal_rpc_callback_var_name(genInfo);

    halRpcClassInfo.className = contain_suffix(genInfo.halName, RPC_NAME) ?
                                genInfo.halName :
                                concat_string(genInfo.halName, RPC_NAME);
    halRpcClassInfo.classVar = get_class_var(halRpcClassInfo.className);
    halRpcClassInfo.existCallback = valid_str(genInfo.aidlCallbackName);
    halRpcClassInfo.existCallback2 = valid_str(genInfo.aidlCallback2Name);
    halRpcClassInfo.existCallbackResult = valid_str(genInfo.aidlCallbackResultName);
    halRpcClassInfo.isClearCallbackRequired = apcc_is_clear_callback_required();

    halRpcClassInfo.callbackVar = {get_hal_rpc_callback_type(genInfo.aidlCallbackName),
                                   callbackVarName};
    halRpcClassInfo.callbackVar2 = {get_hal_rpc_callback_type(genInfo.aidlCallback2Name),
                                    get_var_int(get_format_string_in_lower(get_hal_name(genInfo.aidlCallback2Name)))};
    halRpcClassInfo.callbackVarResult = {get_hal_rpc_callback2_type(genInfo.aidlCallbackResultName),
                                         get_var_int(get_format_string_in_lower(get_hal_name(genInfo.aidlCallbackResultName)))};

    halRpcClassInfo.staticInst = {get_hal_rpc_class_type(genInfo.halIntfFileInfo.halBnName),
                                  get_hal_rpc_inst_static_variable(halRpcClassInfo.className)};
    halRpcClassInfo.staticProxy = {get_shared_ptr(HAL_RPC_CLASS), STATIC_PROXY_VAR};
    halRpcClassInfo.halModePropertyName = get_hal_mode_property_name(genInfo.aidlPackageName, genInfo.halNameLower);
}

static void init_hal_msg_info(ApccGenInfo& genInfo, HalMsgInfo& halMsgInfo)
{
    const string& aidlCallbackHalNameUpper = genInfo.aidlCallbackHalName.nameUpper;
    const string& aidlCallbackResultHalNameUpper = genInfo.aidlCallbackResultHalName.nameUpper;
    bool isCfmDefined = genInfo.isCfmDefined;

    halMsgInfo.typeName = concat_string(genInfo.halName, HAL_MSG_TYPE_SUFFIX);
    halMsgInfo.payloadName = concat_string(genInfo.halName, HAL_PAYLOAD_SUFFIX);
    init_hal_payload_struct(halMsgInfo.payloadName, halMsgInfo.payloadStruct);
    halMsgInfo.msgTransportName = concat_string(genInfo.halName, HAL_MSG_TRANSPORT_SUFFIX);

    halMsgInfo.reqBase = concat_string(genInfo.halNameUpper, HAL_REQ_BASE_SUFFIX);
    halMsgInfo.reqCount = concat_string(genInfo.halNameUpper, HAL_REQ_COUNT_SUFFIX);
    halMsgInfo.indBase = concat_string(aidlCallbackHalNameUpper, HAL_IND_BASE_SUFFIX);
    halMsgInfo.indCount = concat_string(aidlCallbackHalNameUpper, HAL_IND_COUNT_SUFFIX);
    halMsgInfo.cfmBase = concat_string(genInfo.halNameUpper, isCfmDefined ? HAL_CFM_BASE_SUFFIX : HAL_REQ_BASE_SUFFIX);
    halMsgInfo.cfmCount = concat_string(genInfo.halNameUpper, isCfmDefined ? HAL_CFM_COUNT_SUFFIX : HAL_REQ_COUNT_SUFFIX);
    halMsgInfo.upReqBase = concat_string(genInfo.halNameUpper, HAL_UP_REQ_BASE_SUFFIX);
    halMsgInfo.upReqCount = concat_string(genInfo.halNameUpper, HAL_UP_REQ_COUNT_SUFFIX);
    halMsgInfo.downCfmBase = concat_string(genInfo.halNameUpper, HAL_DOWN_CFM_BASE_SUFFIX);
    halMsgInfo.downCfmCount = concat_string(genInfo.halNameUpper, HAL_DOWN_CFM_COUNT_SUFFIX);

    halMsgInfo.isHalReq = get_is_hal_msg_macro(genInfo.halNameUpper, "REQ");
    halMsgInfo.isHalCfm = get_is_hal_msg_macro(genInfo.halNameUpper, isCfmDefined ? "CFM" : "REQ");
    halMsgInfo.isHalInd = get_is_hal_msg_macro(aidlCallbackHalNameUpper, "IND");
    halMsgInfo.isHalUpReq = get_is_hal_msg_macro(aidlCallbackResultHalNameUpper, "IND");
    halMsgInfo.isHalDownCfm = get_is_hal_msg_macro(aidlCallbackResultHalNameUpper, isCfmDefined ? "CFM" : "REQ");

    halMsgInfo.clientAppName = concat_string(genInfo.halNameUpper, HAL_CLIENT_APP_NAME_SUFFIX);
    halMsgInfo.serverAppName = concat_string(genInfo.halNameUpper, HAL_SERVER_APP_NAME_SUFFIX);
    halMsgInfo.serviceId = concat_string(genInfo.halNameUpper, HAL_SERVICE_ID_SUFFIX);
    halMsgInfo.eventGroupId = concat_string(genInfo.halNameUpper, HAL_EVENTGROUP_ID_SUFFIX);
}

static void set_delete_hal_status_func(FuncType& funcType)
{
    string funcName = DELETE_HAL_STATUS_FUNC;
    vector<FuncParameter> funcParam;

    funcParam.push_back({CPP_UINT16, MESSAGE_TYPE_NAME});
    funcParam.push_back({CPP_UINT16, unused_param_name(SESSION_ID_NAME)});
    funcParam.push_back({CPP_VOID_POINTER, HAL_STATUS_VAR});

    init_func_type(CPP_VOID, funcName, funcParam, funcType);
}

static void set_hal_rpc_create_func(ApccGenInfo& genInfo, const string& returnType, FuncType& funcType)
{
    const string& paramType = genInfo.isAidlIntfRoot ? HAL_API_PARAM_TYPE : INSTANCE_ID_TYPE;
    const string& paramName = genInfo.isAidlIntfRoot ? HAL_API_PARAM_NAME : INSTANCE_ID_NAME;
    string funcName = CREATE_NAME;
    vector<FuncParameter> funcParam;
    FuncParameter param;

    init_func_param(paramType, paramName, param);
    funcParam.push_back(param);

    init_func_type(returnType, funcName, funcParam, funcType);
}

static void init_hal_rpc_func(const string& aidlIntfName, const vector<FuncType>& halCommonFunc, vector<FuncType>& halRpcFunc)
{
    const vector<AidlInterface>& aidlIntfList = aidl_get_interface_list(aidlIntfName);

    halRpcFunc.clear();
    if (!halCommonFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < halCommonFunc.size(); index++)
        {
            FuncType ft;
            map_2_hal_rpc_func_type(halCommonFunc[index], ft);
            /* Restore func name as AIDL interface name */
            ft.funcName = aidlIntfList[index].funcName;
            halRpcFunc.push_back(ft);
        }
    }
}

static string get_hal_rpc_internal_return_type(bool isValidateFuncSupported,
    const FuncType& commonFuncType, bool& returnPair)
{
    stringstream ss;
    const string& returnType = commonFuncType.returnType;

    if (isValidateFuncSupported)
    {
        if (apcc_is_cpp_type(returnType, CPP_VOID) ||
            apcc_is_basic_type(returnType))
        {
            returnPair = false;
            ss << SCOPED_ASTATUS;
        }
        else
        {
            returnPair = true;
            if (aidl_is_interface(returnType))
                ss << CPP_PAIR << '<' << get_shared_ptr(returnType) << ", " << SCOPED_ASTATUS << '>';
            else
                ss << CPP_PAIR << '<' << returnType << ", " << SCOPED_ASTATUS << '>';
        }
    }
    else
    {
        returnPair = false;
        ss << SCOPED_ASTATUS;
    }

    return ss.str();
}

static void init_hal_rpc_internal_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcInternalFunc)
{
    const vector<AidlInterface>& aidlIntfList = aidl_get_interface_list(genInfo.aidlIntfName);
    bool isValidateFuncSupported = genInfo.isValidateFuncSupported;

    halRpcInternalFunc.clear();

    if (isValidateFuncSupported)
    {
        if (!genInfo.halCommonFunc.empty())
        {
            for (vector<FuncType>::size_type index = 0; index < genInfo.halCommonFunc.size(); index++)
            {
                FuncType funcType;
                bool returnPair = false;

                funcType.returnType = get_hal_rpc_internal_return_type(isValidateFuncSupported,
                    genInfo.halCommonFunc[index], returnPair);

                map_2_hal_rpc_internal_func_type(isValidateFuncSupported, genInfo.halRpcFunc[index],
                    genInfo.halCommonFunc[index], returnPair, funcType);

                /* e.g. "foo" -> "fooInternal" */
                funcType.funcName = concat_string(aidlIntfList[index].funcName, INTERNAL_FUNC_SUFFIX);

                // apcc_dump_func_type(funcType);
                halRpcInternalFunc.push_back(funcType);
            }
        }
    }
    else
    {
        if (!genInfo.halRpcFunc.empty())
        {
            for (vector<FuncType>::size_type index = 0; index < genInfo.halRpcFunc.size(); index++)
            {
                FuncType funcType;

                funcType.returnType = genInfo.halRpcFunc[index].returnType;

                funcType.param = genInfo.halRpcFunc[index].param;

                /* e.g. "foo" -> "fooInternal" */
                funcType.funcName = concat_string(aidlIntfList[index].funcName, INTERNAL_FUNC_SUFFIX);

                // apcc_dump_func_type(funcType);
                halRpcInternalFunc.push_back(funcType);
            }
        }
    }
}

static void set_init_func(FuncType& funcType)
{
    funcType.param.clear();

    funcType.returnType = CPP_VOID;
    funcType.funcName = INIT_FUNC;
}

static void set_init_someip_context_func(FuncType& funcType)
{
    funcType.param.clear();

    funcType.returnType = CPP_VOID;
    funcType.funcName = INIT_SOMEIP_CONTEXT_FUNC;
}

static void set_init_hal_status_func(FuncType& funcType)
{
    funcType.param.clear();

    funcType.returnType = CPP_VOID;
    funcType.funcName = INIT_HAL_STATUS_FUNC;
}

static void set_handle_hal_msg_func(const char* funcName, FuncType& funcType)
{
    funcType.returnType = CPP_VOID;
    funcType.funcName = funcName;

    funcType.param.clear();

    /* The 1st param: uint16_t message_type */
    funcType.param.push_back({CPP_UINT16, MESSAGE_TYPE_NAME});

    /* The 2nd param: uint8_t* data */
    funcType.param.push_back({CPP_UINT8_POINTER, DATA_NAME});

    /* The 3rd param: size_t length */
    funcType.param.push_back({CPP_SIZE_T, LENGTH_NAME});
}

static void set_handle_hal_ind_func(FuncType& funcType)
{
    set_handle_hal_msg_func(HANDLE_HAL_IND_FUNC, funcType);
}

static void set_handle_hal_cfm_func(ApccGenInfo& genInfo, FuncType& funcType)
{
    set_handle_hal_msg_func(HANDLE_HAL_CFM_FUNC, funcType);
    /* The 4th param: HalStatus** halStatus */
    if (genInfo.isAidlSyncApi)
    {
        funcType.param.push_back({CPP_VOID_POINTER_POINTER, HAL_STATUS_VAR});
    }
}

static void set_handle_hal_req_func(ApccGenInfo& genInfo, FuncType& funcType)
{
    set_handle_hal_msg_func(HANDLE_HAL_REQ_FUNC, funcType);
    /* The 4th param: vector<uint8_t>& result */
    if (genInfo.isAidlSyncApi)
    {
        funcType.param.push_back({CPP_BYTE_VECTOR_REF, RESULT_NAME});
    }
}

static void init_get_interface_func(const string& aidlIntfName, FuncType& funcType)
{
    funcType.param.clear();

    funcType.returnType = create_cpp_type_shared_ptr(aidlIntfName);
    funcType.funcName = GET_INTERFACE_NAME;
}

static void init_hal_rpc_override_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcOverrideFunc)
{
    bool existHalCommonCallback = !genInfo.halCommonCallback.empty();
    bool existHalCommonCallbackResult = !genInfo.halCommonCallbackResult.empty();
    FuncType funcType;

    halRpcOverrideFunc.clear();

    /* void init() */
    if (genInfo.isSingleSomeip)
    {
        set_init_func(funcType);
        halRpcOverrideFunc.push_back(funcType);
    }

    /* void initSomeipContext() */
    set_init_someip_context_func(funcType);
    halRpcOverrideFunc.push_back(funcType);

    /* void initHalStatus() */
    if (apcc_is_aidl_sync_api())
    {
        set_init_hal_status_func(funcType);
        halRpcOverrideFunc.push_back(funcType);
    }

    if (genInfo.handleHalCfm)
    {
        /* void handleHalCfm(uint16_t message_type, uint8_t* data, size_t length) */
        set_handle_hal_cfm_func(genInfo, funcType);
        halRpcOverrideFunc.push_back(funcType);
    }

    if (existHalCommonCallback)
    {
        /* void handleHalInd(uint16_t message_type, uint8_t* data, size_t length) */
        set_handle_hal_ind_func(funcType);
        halRpcOverrideFunc.push_back(funcType);
    }

    if (existHalCommonCallbackResult)
    {
        /* void handleHalReq(uint16_t message_type, uint8_t* data, size_t length, vector<uint8_t>& result) */
        set_handle_hal_req_func(genInfo, funcType);
        halRpcOverrideFunc.push_back(funcType);
    }
}

static void init_hal_rpc_static_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcStaticFunc)
{
    FuncType funcType;

    halRpcStaticFunc.clear();

    /* shared_ptr<BnHal> create(const char* instance_name) */
    set_hal_rpc_create_func(genInfo, genInfo.halRpcClassInfo.staticInst.type, funcType);
    halRpcStaticFunc.push_back(funcType);

    if (genInfo.isAidlIntfRoot)
    {
        /* bool isRpcEnabled() */
        set_common_func(IS_RPC_ENABLED_FUNC, CPP_BOOL, funcType);
        halRpcStaticFunc.push_back(funcType);

        if (genInfo.isSingleSomeip)
        {
            /* shared_ptr<HalRpc> getStaticProxy() */
            set_common_func(GET_STATIC_PROXY_NAME, get_shared_ptr(HAL_RPC_CLASS), funcType);
            halRpcStaticFunc.push_back(funcType);
        }
    }

    /* void deleteHalStatus(uint16_t message_type, uint16_t session_id, void* halStatus) */
    if (apcc_is_aidl_sync_api())
    {
        set_delete_hal_status_func(funcType);
        halRpcStaticFunc.push_back(funcType);
    }
}

static void init_hal_rpc_handle_upstream_msg_func(ApccGenInfo& genInfo, const string& prefix,
    const vector<string>& halMsg, const vector<FuncType>& halCommonFunc,
    vector<FuncType>& halRpcHandleFunc, bool isReq = false)
{
    halRpcHandleFunc.clear();

    if (!halMsg.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < halMsg.size(); index++)
        {
            FuncType funcType;

            funcType.returnType = CPP_VOID;
            funcType.funcName = concat_string("handle", prefix, halMsg[index]);

            if (is_empty_param(halCommonFunc[index].param))
            {
                // The 1st param: "uint8_t* /* msg_data */"
                funcType.param.push_back({CPP_UINT8_POINTER, unused_param_name(MSG_DATA_NAME)});
                // The 2nd param: "size_t /* msg_length */"
                funcType.param.push_back({CPP_SIZE_T, unused_param_name(MSG_LENGTH_NAME)});
            }
            else
            {
                // The 1st param: "uint8_t* msg_data"
                funcType.param.push_back({CPP_UINT8_POINTER, MSG_DATA_NAME});
                // The 2nd param: "size_t msg_length"
                funcType.param.push_back({CPP_SIZE_T, MSG_LENGTH_NAME});
            }
            // The 3rd param: "vector<uint8_t>& result"
            if (isReq && genInfo.isAidlSyncApi)
            {
                funcType.param.push_back({CPP_BYTE_VECTOR_REF, RESULT_NAME});
            }

            halRpcHandleFunc.push_back(funcType);
        }
    }
}

static void init_hal_rpc_handle_ind_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcHandleIndFunc)
{
    string prefix = valid_str(genInfo.aidlCallback2Name) ? aidl_get_interface_short_name(genInfo.aidlCallbackName) : "";
    init_hal_rpc_handle_upstream_msg_func(genInfo, prefix, genInfo.halInd,
        genInfo.halCommonCallback, halRpcHandleIndFunc);
}

static void init_hal_rpc_handle_ind2_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcHandleIndFunc)
{
    halRpcHandleIndFunc.clear();
    if (valid_str(genInfo.aidlCallback2Name))
    {
        string prefix = aidl_get_interface_short_name(genInfo.aidlCallback2Name);
        init_hal_rpc_handle_upstream_msg_func(genInfo, prefix, genInfo.halInd2,
            genInfo.halCommonCallback2, halRpcHandleIndFunc);
    }
}

static void init_hal_rpc_handle_up_req_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcHandleUpReqFunc)
{
    init_hal_rpc_handle_upstream_msg_func(genInfo, "", genInfo.halUpReq,
        genInfo.halCommonCallbackResult, halRpcHandleUpReqFunc, true);
}

static void set_add_hal_callback_func(const string& funcName,
    const string& aidlCallbackName, FuncType& funcType)
{
    FuncParameter funcParam;

    funcType.param.clear();

    funcType.returnType = CPP_VOID;
    funcType.funcName = funcName;

    init_func_param(create_cpp_type_shared_ptr_ref(aidlCallbackName), CALLBACK_NAME, funcParam);
    funcType.param.push_back(funcParam);
}

static void init_hal_rpc_callback_util_info(ApccGenInfo& genInfo, vector<HalRpcCallbackUtilInfo>& halRpcCallbackUtilInfo)
{
    HalRpcCallbackUtilInfo info;

    halRpcCallbackUtilInfo.clear();

    if (genInfo.halRpcClassInfo.existCallback)
    {
        info.aidlCallbackName = genInfo.aidlCallbackName;
        info.aidlCallbackVar = genInfo.halRpcClassInfo.existCallback2 || genInfo.halRpcClassInfo.existCallbackResult ?
                                get_var_int(get_format_string_in_lower(get_hal_name(info.aidlCallbackName))) :
                                ((genInfo.apccType == APCC_TYPE_HAL_RPC_SOURCE) && apcc_is_multi_callback() ?
                                CALLBACKS_VAR_NAME : CALLBACK_VAR_NAME);
        /* void addCallback(const shared_ptr<IHalCallback>& callback) */
        set_add_hal_callback_func(ADD_HAL_CALLBACK_FUNC, genInfo.aidlCallbackName, info.funcType);
        halRpcCallbackUtilInfo.push_back(info);
    }

    if (genInfo.halRpcClassInfo.existCallback2)
    {
        info.aidlCallbackName = genInfo.aidlCallback2Name;
        info.aidlCallbackVar = get_var_int(get_format_string_in_lower(get_hal_name(info.aidlCallbackName)));
        /* void addCallback2(const shared_ptr<IHalCallback>& callback) */
        set_add_hal_callback_func(ADD_HAL_CALLBACK2_FUNC, genInfo.aidlCallback2Name, info.funcType);
        halRpcCallbackUtilInfo.push_back(info);
    }

    if (genInfo.halRpcClassInfo.existCallbackResult)
    {
        info.aidlCallbackName = genInfo.aidlCallbackResultName;
        info.aidlCallbackVar = get_var_int(get_format_string_in_lower(get_hal_name(info.aidlCallbackName)));
        /* void addCallbackResult(const shared_ptr<IHalCallback>& callback) */
        set_add_hal_callback_func(ADD_HAL_CALLBACK_RESULT_FUNC, genInfo.aidlCallbackResultName, info.funcType);
        halRpcCallbackUtilInfo.push_back(info);
    }

    if (genInfo.halRpcClassInfo.isClearCallbackRequired &&
        (genInfo.halRpcClassInfo.existCallback ||
        genInfo.halRpcClassInfo.existCallbackResult))
    {
        info.aidlCallbackName = "";
        info.aidlCallbackVar = "";
        /* void clearCallback() */
        set_common_func(CLEAR_HAL_CALLBACK_FUNC, CPP_VOID, info.funcType);
        halRpcCallbackUtilInfo.push_back(info);
    }
}

static void init_aidl_callback_func(const string& callbackName, const string& aidlCallbackName,
    vector<FuncType>& halCommonCallback, vector<FuncType>& aidlCallbackFunc)
{
    const vector<AidlInterface>& aidlIntfList = aidl_get_interface_list(aidlCallbackName);

    for (vector<FuncType>::size_type index = 0; index < halCommonCallback.size(); index++)
    {
        FuncType funcType;
        const FuncType& callback = halCommonCallback[index];

        funcType.funcName = struct_ptr_field(callbackName, aidlIntfList[index].funcName);
        funcType.returnType = callback.returnType;
        funcType.param = callback.param;

        aidlCallbackFunc.push_back(funcType);
    }
}

static void init_aidl_intf_info(const string& aidlIntfName, AidlIntfInfo& aidlIntfInfo)
{
    aidlIntfInfo.intfName = aidlIntfName;
    // logd("%s: intfName: %s", __func__, aidlIntfInfo.intfName.c_str());
    aidlIntfInfo.className = get_aidl_intf_class_name(aidlIntfName);
    // logd("%s: className: %s", __func__, aidlIntfInfo.className.c_str());
    aidlIntfInfo.headerFile = get_aidl_intf_header_file(aidlIntfInfo.className);
    // logd("%s: headerFile: %s", __func__, aidlIntfInfo.headerFile.c_str());
    aidlIntfInfo.usingType = "";
    // logd("%s: usingType: %s", __func__, aidlIntfInfo.usingType.c_str());
}

static void init_hal_aidl_intf_info(ApccGenInfo& genInfo, vector<AidlIntfInfo>& halAidlIntfInfo)
{
    if (!genInfo.halCommonFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halCommonFunc.size(); index++)
        {
            const FuncType& funcType = genInfo.halCommonFunc[index];
            if (aidl_is_interface(funcType.returnType))
            {
                // logd("%s: aidl return type: %s", __func__, funcType.returnType.c_str());
                AidlIntfInfo aidlIntfInfo;
                init_aidl_intf_info(funcType.returnType, aidlIntfInfo);
                halAidlIntfInfo.push_back(aidlIntfInfo);
            }
        }
    }
}

static void init_hal_result_info(ApccGenInfo& genInfo, vector<HalResultInfo>& halResultInfo)
{
    bool isCfmDefined = genInfo.isCfmDefined;

    if (!genInfo.halCommonFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < genInfo.halCommonFunc.size(); index++)
        {
            const FuncType& funcType = genInfo.halCommonFunc[index];
            if (!is_cpp_void(funcType.returnType))
            {
                // logd("%s: return type: %s", __func__, funcType.returnType.c_str());
                HalResultInfo hri;

                hri.halCfm = isCfmDefined ? genInfo.halCfmDef[index] : genInfo.halReqDef[index];
                hri.halResultName = map_2_hal_cfm_param(genInfo, funcType.funcName, genInfo.halName);
                hri.halResultType = hri.halResultName;

                for (auto it: funcType.param)
                {
                    if (apcc_is_out_param_type(it.type, it.name))
                    {
                        logd("%s: store out param (%s %s)", __func__, it.type.c_str(), it.name.c_str());
                        hri.halResultParam.push_back(it);
                    }
                }
                hri.halResultParseFunc = genInfo.halCfmFunc[index];
                logd("%s: halCfm: %s, halResultType: %s", __func__,
                    hri.halCfm.c_str(), hri.halResultType.c_str());
                halResultInfo.push_back(hri);
            }
        }
    }
}

static bool gen_hal_rpc_service_flag(const string& aidlIntfName)
{
    return false;
}

static bool gen_hal_status_util_flag(const string& aidlIntfName)
{
    bool isAidlIntfRoot = aidl_is_intf_root(aidlIntfName);
    return isAidlIntfRoot && (valid_str(aidl_get_hal_status_type())) ? true : false;
}

static bool get_handle_hal_cfm_flag(ApccGenInfo& genInfo)
{
    bool handle_hal_cfm = false;
    for (auto it: genInfo.halCommonFunc)
    {
        if (!is_cpp_void(it.returnType))
        {
            handle_hal_cfm = true;
            break;
        }
    }
    return apcc_is_aidl_sync_api() || handle_hal_cfm;
}

static void init_hal_rpc_service_info(ApccGenInfo& genInfo, HalRpcServiceInfo& halRpcServiceInfo)
{
    vector<string> aidlIntfName;
    FuncType funcType;
    vector <FuncParameter> param;

    halRpcServiceInfo.gen = gen_hal_rpc_service_flag(genInfo.aidlIntfName);
    halRpcServiceInfo.className = hal_rpc_service_name(genInfo.aidlHalRoot.halName.name);

    /* func: init() */
    init_func_type(CPP_VOID, INIT_FUNC, param, funcType);
    halRpcServiceInfo.func.push_back(funcType);

    /* func: deinit() */
    init_func_type(CPP_VOID, DEINIT_FUNC, param, funcType);
    halRpcServiceInfo.func.push_back(funcType);

    halRpcServiceInfo.halRpcBaseClassName = HAL_RPC_CLASS;

    aidl_get_type_name(aidl_get_info(), AIDL_TYPE_INTERFACE, aidlIntfName);
    for (auto it: aidlIntfName)
    {
        string halRpcClass = concat_string(aidl_get_interface_short_name(it), RPC_NAME);
        halRpcServiceInfo.halRpcClassName.push_back(halRpcClass);

        string halRpcInclude = apcc_get_gen_file_name(it, APCC_TYPE_HAL_RPC_HEADER);
        halRpcServiceInfo.halRpcInclude.push_back(halRpcInclude);
        // logd("%s: halRpcClass: %s, halRpcInclude: %s", __func__, halRpcClass.c_str(), halRpcInclude.c_str());
    }
}

static void set_hal_service_init_func(const string& serviceName, FuncType& funcType)
{
    funcType.returnType = CPP_BOOL;
    funcType.funcName = concat_string(serviceName, INIT_SUFFIX);
    funcType.param.clear();
}

static string get_using_type(const vector<string>& namespaceString, const string& className)
{
    stringstream ss;
    if (!namespaceString.empty())
    {
        for (auto it: namespaceString)
        {
            ss << "::" << it;
        }
    }
    ss << "::" << className;
    return ss.str();
}

static string get_hal_callback_class_var(const string& halNameLower)
{
    stringstream ss;
    ss << halNameLower << '_' << CALLBACK_NAME << VAR_SUFFIX;
    return ss.str();
}

static string get_hal_rpc_server_api_func_name(const string& halRpcServerName)
{
    stringstream ss;
    /* e.g. create_hal_rpc_server */
    ss << "create_" << halRpcServerName;
    return ss.str();
}

static void init_hal_rpc_server_api_func(const string& halRpcServerType, const string& halRpcServerName,
    const FuncParameter& aidlIntfParam, vector<FuncType>& halRpcServerApiFunc)
{
    FuncType funcType;

    halRpcServerApiFunc.clear();

    funcType.returnType = get_std_shared_ptr(halRpcServerType);
    funcType.funcName = get_hal_rpc_server_api_func_name(halRpcServerName);
    funcType.param.clear();
    funcType.param.push_back(aidlIntfParam);
    funcType.param.push_back({INSTANCE_ID_TYPE, INSTANCE_ID_NAME});

    halRpcServerApiFunc.push_back(funcType);
}

static void init_hal_server_info(ApccGenInfo& genInfo, HalServerInfo& halServerInfo)
{
    string packageName = apcc_get_package_name(genInfo.aidlPackageName);
    const string& halNameLower = genInfo.halNameLower;
    string halRpcServerName = concat_string(halNameLower, RPC_SERVER_SUFFIX);
    const string& halRpcServerClass = genInfo.halRpcServerClassInfo.className;

    halServerInfo.serviceName = concat_string(packageName, SERVICE_SUFFIX);
    halServerInfo.serviceHeader = concat_string(halServerInfo.serviceName, HEADER_FILE_SUFFIX);
    set_hal_service_init_func(halServerInfo.serviceName, halServerInfo.serviceInitFunc);

    halServerInfo.halRpcServerName = halRpcServerName;
    halServerInfo.halRpcServerHeader = concat_string(halRpcServerName, HEADER_FILE_SUFFIX);
    halServerInfo.halRpcServerUsingType = get_using_type(genInfo.halRpcNamespace, halRpcServerClass);

    init_hal_rpc_server_api_func(halServerInfo.halRpcServerUsingType, halServerInfo.halRpcServerName,
        genInfo.halRpcServerClassInfo.aidlIntfParam, halServerInfo.halRpcServerApiFunc);

    halServerInfo.halApiHeader = apcc_get_gen_file_name(genInfo.aidlIntfName, APCC_TYPE_HAL_SERVER_API_HEADER);
}

static void init_hal_rpc_server_class_info(ApccGenInfo& genInfo, HalRpcServerClassInfo& halRpcServerClassInfo)
{
    string className = concat_string(genInfo.halName, RPC_SERVER_NAME);
    string aidlIntfUsingType = get_using_type(genInfo.halNamespace, genInfo.aidlIntfName);
    vector<FuncParameter> createFuncParam;

    createFuncParam.clear();
    /* 1. const shared_ptr<IHal>& hal */
    createFuncParam.push_back({create_cpp_type_shared_ptr_ref(genInfo.aidlIntfName), get_class_var(genInfo.halName)});
    /* 2. uint16_t instanceId */
    createFuncParam.push_back({INSTANCE_ID_TYPE, INSTANCE_ID_NAME});

    halRpcServerClassInfo.className = className;
    halRpcServerClassInfo.classType = get_shared_ptr(className);
    halRpcServerClassInfo.classVar = get_class_var(className);
    halRpcServerClassInfo.classVarInt = get_var_int(get_format_string_in_lower(className));

    halRpcServerClassInfo.intfParam = {get_shared_ptr(genInfo.aidlIntfName),
                                       get_var_int(get_format_string_in_lower(genInfo.halName))};

    if (valid_str(genInfo.aidlCallbackName))
    {
        halRpcServerClassInfo.callbackParam = {get_shared_ptr(genInfo.aidlCallbackName),
                                               get_var_int(get_format_string_in_lower(genInfo.halCallbackName))};
    }

    if (valid_str(genInfo.aidlCallback2Name))
    {
        halRpcServerClassInfo.callback2Param = {get_shared_ptr(genInfo.aidlCallback2Name),
                                                get_var_int(get_format_string_in_lower(genInfo.halCallback2Name))};
    }

    halRpcServerClassInfo.staticInst = {get_shared_ptr(className),
                                        get_hal_rpc_inst_static_variable(className)};

    halRpcServerClassInfo.aidlIntfParam = {create_std_cpp_type_shared_ptr_ref(aidlIntfUsingType),
                                           get_class_var(genInfo.halName)};

    halRpcServerClassInfo.createFuncParam = createFuncParam;

    halRpcServerClassInfo.constructorDefault = {"", className, vector<FuncParameter>()};
    halRpcServerClassInfo.constructor = {"", className, createFuncParam};
    halRpcServerClassInfo.deconstructor = {"", concat_string("~", className), vector<FuncParameter>()};
}

static void init_hal_callback_class_info(const string& aidlCallbackName, const string& halCallbackName,
    const string& halNameLower, HalCallbackClassInfo& halCallbackClassInfo)
{
    halCallbackClassInfo.existCallback = valid_str(aidlCallbackName);
    halCallbackClassInfo.className = halCallbackName;
    halCallbackClassInfo.classVar = get_hal_callback_class_var(halNameLower);
}

static string map_2_hal_req_param(ApccGenInfo& genInfo, const string& halCommonFuncName, const string& halName)
{
    string str = update_string(halCommonFuncName, halName, "");
    string suffix = update_string(halName, genInfo.aidlHalRoot.halName.name, "");

    stringstream ss;
    /* e.g. HalFoo + Hal -> FooReqParam */
    ss << str << PROTO_MESSAGE_REQ << suffix << PARAM_STRUCT_SUFFIX;
    return ss.str();
}

static void map_2_hal_rpc_server_req_func_type(ApccGenInfo& genInfo, const string& halName,
    const FuncType& halCommonFuncType, const string& suffix, FuncType& funcType)
{
    FuncParameter funcParam;
    uint32_t inParamCount = 0;

    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = map_2_hal_msg_func_name(halCommonFuncType.funcName, halName, HAL_MSG_PARSE, suffix);

    /* 3. param */
    funcType.param.clear();
    /* The 1st parameter: "uint8_t* msg_data" */
    funcType.param.push_back({CPP_UINT8_POINTER, MSG_DATA_NAME});
    /* The 2nd parameter: "size_t msg_length" */
    funcType.param.push_back({CPP_SIZE_T, MSG_LENGTH_NAME});

    for (auto it: halCommonFuncType.param)
    {
        FuncParameter param = it;
        if (aidl_is_interface(param.type) ||
            apcc_is_out_param_type(param.type, param.name))
        {
            /* ignore AIDL interface parameter (e.g. IHalCallback) */
            logd("%s: ignore aidl intf param (%s %s)", __func__, param.type.c_str(), param.name.c_str());
            continue;
        }
        ++inParamCount;
    }

    if (inParamCount > 1)
    {
        /* The 3rd parameter: "HalReqParam& param" */
        funcType.param.push_back({map_2_hal_req_param(
            genInfo, halCommonFuncType.funcName, halName), PARAM_VAR_NAME});
    }
    else if (inParamCount == 1)
    {
        for (auto it: halCommonFuncType.param)
        {
            FuncParameter param = it;
            if (aidl_is_interface(param.type) ||
                apcc_is_out_param_type(param.type, param.name))
            {
                /* ignore AIDL interface parameter (e.g. IHalCallback) */
                logd("%s: ignore aidl intf param (%s %s)", __func__, param.type.c_str(), param.name.c_str());
                continue;
            }
            funcType.param.push_back(param);
        }
    }
}

static void init_hal_rpc_server_func(const string& aidlIntfName, const vector<FuncType>& halCommonFunc,
    vector<FuncType>& halRpcServerFunc)
{
    const vector<AidlInterface>& aidlIntfList = aidl_get_interface_list(aidlIntfName);

    halRpcServerFunc.clear();
    if (!halCommonFunc.empty())
    {
        for (vector<FuncType>::size_type index = 0; index < halCommonFunc.size(); index++)
        {
            FuncType ft;

            /* Restore func name as AIDL interface name */
            ft.funcName = aidlIntfList[index].funcName;
            ft.returnType = halCommonFunc[index].returnType;
            ft.param = halCommonFunc[index].param;

            halRpcServerFunc.push_back(ft);
        }
    }
}

static void init_hal_rpc_server_req_func(ApccGenInfo& genInfo,
    const vector<FuncType>& halCommonFunc, vector<FuncType>& halRpcServerReqFunc)
{
    FuncType funcType;

    for (vector<string>::size_type index = 0; index < halCommonFunc.size(); index++)
    {
        map_2_hal_rpc_server_req_func_type(genInfo, genInfo.halName, halCommonFunc[index], PROTO_MESSAGE_REQ, funcType);

        halRpcServerReqFunc.push_back(funcType);
    }
}

void map_2_hal_rpc_server_ind_func_type(const string& halName, const FuncType& halCommonCallbackType, FuncType& funcType)
{
    FuncParameter funcParam;

    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = map_2_hal_msg_func_name(halCommonCallbackType.funcName, halName,
                                                HAL_MSG_SERIALIZE, PROTO_MESSAGE_IND);

    /* 3. param */
    funcType.param.clear();

    copy_func_param_generic(halCommonCallbackType.param, funcType.param);

    /* last param is output for payload serialized in "vector<uint8_t>& payload" */
    funcType.param.push_back({CPP_BYTE_VECTOR_REF, PAYLOAD_NAME});
}

static void init_hal_rpc_server_ind_func(ApccGenInfo& genInfo, const vector<FuncType>& halCommonFunc,
    const AidlHalName& aidlCallbackHalName, vector<FuncType>& halRpcServerIndFunc)
{
    FuncType funcType;

    for (vector<string>::size_type index = 0; index < halCommonFunc.size(); index++)
    {
        map_2_hal_rpc_server_ind_func_type(aidlCallbackHalName.name, halCommonFunc[index], funcType);

        halRpcServerIndFunc.push_back(funcType);
    }
}

static string get_hal_rpc_server_cfm_func_name(const string& halName)
{
    string s = halName;
    convert_first_char_to_upper(s);

    stringstream ss;
    ss << s << HAL_MSG_SERIALIZE << HAL_STATUS_NAME;
    return ss.str();
}

static string get_hal_rpc_server_cfm_func_name(ApccGenInfo& genInfo,
    const FuncType& halCommonFuncType, const string& halName, const string& suffix)
{
    const string& returnType = halCommonFuncType.returnType;
    bool existReturnType = !is_cpp_void(returnType);
    bool existOutParam = false;

    for (auto it: halCommonFuncType.param)
    {
        if (apcc_is_out_param_type(it.type, it.name))
        {
            existOutParam = true;
        }
    }

    if (!existReturnType && !existOutParam)
    {
        return get_hal_rpc_server_cfm_func_name(apcc_get_package_name(genInfo.aidlPackageName));
    }
    else
    {
        return map_2_hal_msg_func_name(halCommonFuncType.funcName, halName, HAL_MSG_SERIALIZE, suffix);
    }
}

static void map_2_hal_rpc_server_cfm_func_type(ApccGenInfo& genInfo, const string& halName, const string& halStatusCode,
    const FuncType& halCommonFuncType, const string& suffix, FuncType& funcType)
{
    const string& returnType = halCommonFuncType.returnType;
    bool existReturnType = !is_cpp_void(returnType);

    /* 1. return type: always set as "bool" */
    funcType.returnType = CPP_BOOL;

    /* 2. func name  */
    funcType.funcName = get_hal_rpc_server_cfm_func_name(genInfo, halCommonFuncType, halName, suffix);

    /* 3. param */
    funcType.param.clear();
    if (existReturnType)
    {
        /* The 1st parameter: "const HalStatusParam& status" */
        funcType.param.push_back({cpp_type_const_ref(HAL_STATUS_PARAM_TYPE), HAL_STATUS_PARAM_VAR});
    }
    else
    {
        /* The 1st parameter: "int32_t status" */
        funcType.param.push_back({CPP_INT32, STATUS_VAR});
        /* The 2nd parameter: "const string& info" */
        funcType.param.push_back({cpp_type_const_ref(CPP_STRING), INFO_VAR});
    }

    if (existReturnType)
    {
        if (aidl_is_interface(returnType))
        {
            /* The 3rd parameter: "uint16_t instance_id" */
            funcType.param.push_back({INSTANCE_ID_TYPE, INSTANCE_ID_NAME});
        }
        else if (!apcc_is_basic_type(returnType))
        {
            /* The 3rd parameter: "const T& _aidl_return" */
            funcType.param.push_back({cpp_type_const_ref(returnType), AIDL_RETURN_NAME});
        }
        else
        {
            /* The 3rd parameter: "T _aidl_return" */
            funcType.param.push_back({returnType, AIDL_RETURN_NAME});
        }
    }

    /* 4. out param */
    for (auto it: halCommonFuncType.param)
    {
        if (apcc_is_out_param_type(it.type, it.name))
        {
            funcType.param.push_back({cpp_type_const_ref(apcc_get_out_param_type(it.type)), it.name});
        }
    }

    /* The last parameter: "vector<uint8_t>& result" */
    funcType.param.push_back({CPP_BYTE_VECTOR_REF, RESULT_NAME});
}

static void init_hal_rpc_server_cfm_func(ApccGenInfo& genInfo,
    const vector<FuncType>& halCommonFunc, vector<FuncType>& halRpcServerCfmFunc)
{
    FuncType funcType;

    for (vector<string>::size_type index = 0; index < halCommonFunc.size(); index++)
    {
        map_2_hal_rpc_server_cfm_func_type(genInfo, genInfo.halName, genInfo.halStatusInfo.aidlHalStatusType,
            halCommonFunc[index], PROTO_MESSAGE_CFM, funcType);

        halRpcServerCfmFunc.push_back(funcType);
    }
}

static void init_hal_rpc_server_handle_req_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcServerHandleReqFunc)
{
    init_hal_rpc_handle_upstream_msg_func(genInfo, "", genInfo.halReq,
        genInfo.halCommonFunc, halRpcServerHandleReqFunc, true);
}

static void init_hal_rpc_server_override_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcServerOverrideFunc)
{
    FuncType funcType;

    halRpcServerOverrideFunc.clear();

    /* void init() */
    if (valid_str(genInfo.aidlCallbackName) || valid_str(genInfo.aidlCallback2Name))
    {
        set_init_func(funcType);
        halRpcServerOverrideFunc.push_back(funcType);
    }

    /* void initSomeipContext() */
    set_init_someip_context_func(funcType);
    halRpcServerOverrideFunc.push_back(funcType);

    /* void handleHalReq(uint16_t message_type, uint8_t* data, size_t length, vector<uint8_t>& result) */
    set_handle_hal_req_func(genInfo, funcType);
    halRpcServerOverrideFunc.push_back(funcType);
}

static void init_hal_rpc_server_public_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcServerPublicFunc)
{
    FuncType funcType;

    halRpcServerPublicFunc.clear();

    /* const shared_ptr<IHal> getInterface() */
    init_get_interface_func(genInfo.aidlIntfName, funcType);
    halRpcServerPublicFunc.push_back(funcType);
}

static void init_hal_rpc_server_static_func(ApccGenInfo& genInfo, vector<FuncType>& halRpcServerStaticFunc)
{
    FuncType funcType;

    halRpcServerStaticFunc.clear();

    /* shared_ptr<HalRpcServer> create(const shared_ptr<IHal>& hal, uint16_t instanceId) */
    funcType.returnType = get_shared_ptr(genInfo.halRpcServerClassInfo.className);
    funcType.funcName = CREATE_NAME;
    funcType.param = genInfo.halRpcServerClassInfo.createFuncParam;

    halRpcServerStaticFunc.push_back(funcType);
}

static void init_aidl_callback_info(ApccGenInfo& genInfo, map<string, AidlCallbackInfo>& aidlCallbackInfo)
{
    aidlCallbackInfo.clear();
    for (auto funcType: genInfo.halCommonFunc)
    {
        for (auto param: funcType.param)
        {
            if (aidl_is_interface(param.type))
            {
                AidlCallbackInfo aci;
                string aidlIntfName = param.type;

                aci.intfName = aidlIntfName;
                aci.param = {get_shared_ptr(aci.intfName),
                             get_var_int(get_format_string_in_lower(get_hal_name(aci.intfName)))};

                aidlCallbackInfo.insert(pair<string, AidlCallbackInfo>(aidlIntfName, aci));
            }
        }
    }
}

static void init_hal_msg_param_struct(const string& paramType, const FuncType& funcType, StructType& paramStruct)
{
    paramStruct.name = paramType;
    // logd("%s: struct name: %s", __func__, paramStruct.name.c_str());
    // apcc_dump_func_param(funcType.param);

    paramStruct.field.clear();
    for (auto it: funcType.param)
    {
        FuncParameter param = it;
        if (aidl_is_interface(param.type) ||
            apcc_is_out_param_type(param.type, param.name))
        {
            /* ignore AIDL interface parameter (e.g. IHalCallback) */
            // logd("%s: ignore aidl intf param (%s %s)", __func__, param.type.c_str(), param.name.c_str());
            continue;
        }
        paramStruct.field.push_back(param);
    }
    // logd("%s: dump field", __func__);
    // apcc_dump_func_param(paramStruct.field);
}

static void init_hal_upstream_msg_param_struct(ApccGenInfo& genInfo, const string& aidlCallbackName,
    const vector<FuncType>& halCommonCallback, vector<StructType>& halIndParamStruct)
{
    const vector<AidlInterface>& aidlIntfList = aidl_get_interface_list(aidlCallbackName);
    // logd("%s: aidlCallbackName: %s", __func__, aidlCallbackName.c_str());
    // logd("%s: aidlIntfList size: %d", __func__, aidlIntfList.size());
    if (!aidlIntfList.empty())
    {
        for (vector<AidlInterface>::size_type index = 0; index < aidlIntfList.size(); index++)
        {
            StructType paramStruct;
            const AidlInterface& aidlIntf = aidlIntfList[index];
            string paramType = get_ind_param_type(genInfo, aidlIntf.funcName, aidlIntf.param);
            // logd("%s: funcName: %s, paramType: %s", __func__, aidlIntf.funcName.c_str(), paramType.c_str());
            init_hal_msg_param_struct(paramType, halCommonCallback[index], paramStruct);
            halIndParamStruct.push_back(paramStruct);
        }
    }
}

static void init_hal_rpc_server_req_param_struct(ApccGenInfo& genInfo, const string& aidlIntfName,
    const vector<FuncType>& halCommonFunc, vector<StructType>& halRpcServerReqParamStruct)
{
    const vector<AidlInterface>& aidlIntfList = aidl_get_interface_list(aidlIntfName);

    if (!aidlIntfList.empty())
    {
        for (vector<AidlInterface>::size_type index = 0; index < aidlIntfList.size(); index++)
        {
            StructType paramStruct;
            const AidlInterface& aidlIntf = aidlIntfList[index];
            string paramType = get_req_param_type(genInfo, aidlIntf.funcName, aidlIntf.param);
            init_hal_msg_param_struct(paramType, halCommonFunc[index], paramStruct);
            halRpcServerReqParamStruct.push_back(paramStruct);
        }
    }
}

static void dump_aidl_callback_info(const map<string, AidlCallbackInfo>& aidlCallbackInfo)
{
    logd("%s: size %d", __func__, aidlCallbackInfo.size());
    logd("%s: ++++++++++++++++++++++++", __func__);
    for (auto& it: aidlCallbackInfo)
    {
        const AidlCallbackInfo& aci = it.second;
        logd("aidl callback name: %s", aci.intfName.c_str());
        apcc_dump_func_param(aci.param, "callback param");
    }
    logd("%s: ------------------------", __func__);
}

static void store_create_get_intf_func(const FuncType& funcType, vector<FuncType>& createIntfFunc,
    vector<FuncType>& getIntfFunc)
{
    string funcName = funcType.funcName;

    if (is_create_intf_func_name(funcName))
    {
        createIntfFunc.push_back(funcType);
    }
    else if (is_get_intf_func_name(funcName))
    {
        getIntfFunc.push_back(funcType);
    }
}

static string get_intf_hal_name(const FuncType& funcType)
{
    /* e.g. "createHal" -> "Hal" */
    const string& funcName = funcType.funcName;
    if (contain_prefix(funcName, CREATE_NAME))
    {
        return get_sub_string(funcName, CREATE_NAME);
    }
    else if (contain_prefix(funcName, ADD_NAME))
    {
        return get_sub_string(funcName, ADD_NAME);
    }
    return "";
}

static void init_remove_intf_func(ApccGenInfo& genInfo, const FuncType& funcType, vector<FuncType>& removeIntfFunc)
{
    string halName = get_intf_hal_name(funcType);

    for (auto it: genInfo.halRpcServerFunc)
    {
        const string& funcName = it.funcName;
        if (is_remove_intf_func_name(funcName) &&
            (is_same_string(halName, get_sub_string(funcName, REMOVE_NAME)) ||
            is_same_string(halName, get_sub_string(funcName, DELETE_NAME))))
        {
            removeIntfFunc.push_back(it);
        }
    }
}

static void init_aidl_intf_return_info(ApccGenInfo& genInfo, map<string, AidlIntfReturnInfo>& aidlIntfReturnInfo)
{
    aidlIntfReturnInfo.clear();
    for (auto funcType: genInfo.halRpcServerFunc)
    {
        if (aidl_is_interface(funcType.returnType))
        {
            string aidlIntfName = funcType.returnType;
            map<string, AidlIntfReturnInfo>::iterator it = aidlIntfReturnInfo.find(aidlIntfName);

            if (it != aidlIntfReturnInfo.end())
            {
                /* Update AidlIntfReturnInfo */
                AidlIntfReturnInfo& info = it->second;
                store_create_get_intf_func(funcType, info.createIntfFunc, info.getIntfFunc);
            }
            else
            {
                /* Create AidlIntfReturnInfo */
                AidlIntfReturnInfo info;
                string aidlIntfPtr = get_shared_ptr(aidlIntfName);
                string halName = aidl_get_interface_short_name(aidlIntfName);
                string halRpcServerName = concat_string(get_format_string_in_lower(halName), RPC_SERVER_SUFFIX);
                const FuncParameter& aidlIntfParam = {create_std_cpp_type_shared_ptr_ref(aidlIntfName),
                                                      get_class_var(halName)};
                string halRpcServerClassName = concat_string(halName, RPC_SERVER_NAME);
                string halRpcServerClassPtr = get_shared_ptr(halRpcServerClassName);

                info.intfName = aidlIntfName;
                info.includeHeader = concat_string(concat_string(get_hal_name_lower(aidlIntfName),
                                     RPC_SERVER_SUFFIX), HEADER_FILE_SUFFIX);
                info.param = {aidlIntfPtr, get_class_var(get_hal_name(aidlIntfName))};
                info.halRpcInstListParam = {get_map_type(aidlIntfPtr, INSTANCE_ID_TYPE),
                                            get_var_int(get_format_string_in_lower(halName))};
                info.halRpcServerClassParam = {halRpcServerClassPtr, get_class_var(halRpcServerClassName)};
                info.halRpcServerInstListParam = {get_map_type(halRpcServerClassPtr, INSTANCE_ID_TYPE),
                                                  get_var_int(get_format_string_in_lower(halRpcServerClassName))};

                init_hal_rpc_server_api_func(halRpcServerClassName, halRpcServerName, aidlIntfParam, info.halRpcServerApiFunc);

                store_create_get_intf_func(funcType, info.createIntfFunc, info.getIntfFunc);

                init_remove_intf_func(genInfo, funcType, info.removeIntfFunc);

                if (!info.removeIntfFunc.empty())
                {
                    info.intfKeyParam = info.removeIntfFunc[0].param;
                }

                aidlIntfReturnInfo.insert(pair<string, AidlIntfReturnInfo>(aidlIntfName, info));
            }
        }
    }

    for (auto& it: aidlIntfReturnInfo)
    {
        const string& aidlIntfName = it.first;
        AidlIntfReturnInfo& info = it.second;
        vector<CppTypeInfo> aidlIntfReturnType;
        vector<CppTypeInfo> aidlIntfParamType;
        vector<FuncType> halCommonFunc;
        vector<FuncType> halRpcServerFunc;

        if (info.removeIntfFunc.empty())
            continue;

        const FuncType& removeIntfFunc = info.removeIntfFunc[0];
        if (removeIntfFunc.param.empty())
            continue;

        logd("%s: aidl intf name: %s", __func__, aidlIntfName.c_str());

        // apcc_dump_func_type(removeIntfFunc);
        const string& keyType = removeIntfFunc.param[0].type;
        const string& keyName = removeIntfFunc.param[0].name;

        init_hal_common_func(genInfo, aidlIntfName, genInfo.halName, aidlIntfReturnType, aidlIntfParamType, halCommonFunc);
        // apcc_dump_func_type(halCommonFunc, "->halCommonFunc");

        init_hal_rpc_server_func(aidlIntfName, halCommonFunc, halRpcServerFunc);
        // apcc_dump_func_type(halRpcServerFunc, "->halRpcServerFunc");

        for (auto ft: halRpcServerFunc)
        {
            const string& func_name = ft.funcName;
            const string& return_type = ft.returnType;

            if (is_same_string(return_type, keyType) &&
                is_get_intf_func_name(func_name))
            {
                string key = get_sub_string(func_name, GET_NAME);
                string key_lower = key;

                transform_string_to_lower(key_lower);
                logd("%s: key: %s, key_lower: %s", __func__, key.c_str(), key_lower.c_str());

                if (contain_string(keyName, key) ||
                    contain_string(keyName, key_lower))
                {
                    logd("%s: add getIntfKeyFunc for %s", __func__, aidlIntfName.c_str());
                    info.getIntfKeyFunc.push_back(ft);
                }
            }
        }
    }
}

static void dump_aidl_intf_return_info(map<string, AidlIntfReturnInfo>& aidlIntfReturnInfo)
{
    logd("%s: size %d", __func__, aidlIntfReturnInfo.size());
    logd("%s: ++++++++++++++++++++++++", __func__);
    for (auto it: aidlIntfReturnInfo)
    {
        const AidlIntfReturnInfo& info = it.second;
        logd("aidl intf name: %s", info.intfName.c_str());
        logd("include header: %s", info.includeHeader.c_str());
        // apcc_dump_func_param(info.param, "param");
        // apcc_dump_func_param(info.halRpcServerClassParam, "halRpcServerClassParam");
        // apcc_dump_func_param(info.halRpcServerInstListParam, "halRpcServerInstListParam");
        // apcc_dump_func_type(info.halRpcServerApiFunc, "halRpcServerApiFunc");
        apcc_dump_func_type(info.createIntfFunc, "createIntfFunc");
        // apcc_dump_func_type(info.getIntfFunc, "getIntfFunc");
        apcc_dump_func_type(info.removeIntfFunc, "removeIntfFunc");
        apcc_dump_func_type(info.getIntfKeyFunc, "getIntfKeyFunc");
        apcc_dump_func_param(info.intfKeyParam);
    }
    logd("%s: ------------------------", __func__);
}

static void init_hal_create_func(ApccGenInfo& genInfo, const string& returnType, FuncType& funcType)
{
    vector<FuncParameter> funcParam;

    funcParam.clear();
    if (genInfo.isAidlIntfRoot)
    {
        funcParam.push_back({HAL_API_PARAM_TYPE, HAL_API_PARAM_NAME});
    }

    init_func_type(returnType, CREATE_NAME, funcParam, funcType);
}

static void init_hal_callback_func(const vector<FuncType>& halCallbackFunc, vector<FuncType>& callbackFunc)
{
    callbackFunc.clear();
    for (auto it: halCallbackFunc)
    {
        FuncType ft = it;
        ft.returnType = CPP_VOID;
        callbackFunc.push_back(ft);
    }
}

static void init_hal_class_info(ApccGenInfo& genInfo, HalClassInfo& halClassInfo)
{
    string callbackVarName = valid_str(genInfo.aidlCallback2Name) || valid_str(genInfo.aidlCallbackResultName) ?
                             get_var_int(get_format_string_in_lower(get_hal_name(genInfo.aidlCallbackName))) :
                             CALLBACK_VAR_NAME;

    halClassInfo.className = genInfo.halName;
    halClassInfo.classVar = get_class_var(halClassInfo.className);
    halClassInfo.existCallback = valid_str(genInfo.aidlCallbackName);
    halClassInfo.existCallback2 = valid_str(genInfo.aidlCallback2Name);
    halClassInfo.existCallbackResult = valid_str(genInfo.aidlCallbackResultName);

    halClassInfo.callbackVar = {get_shared_ptr(genInfo.aidlCallbackName),
                                callbackVarName};
    halClassInfo.callbackVar2 = {get_shared_ptr(genInfo.aidlCallback2Name),
                                 get_var_int(get_format_string_in_lower(get_hal_name(genInfo.aidlCallback2Name)))};
    halClassInfo.callbackVarResult = {get_shared_ptr(genInfo.aidlCallbackResultName),
                                      get_var_int(get_format_string_in_lower(get_hal_name(genInfo.aidlCallbackResultName)))};

    halClassInfo.staticInst = {get_hal_rpc_class_type(genInfo.halIntfFileInfo.halBnName),
                               get_hal_rpc_inst_static_variable(halClassInfo.className)};

    init_hal_create_func(genInfo, halClassInfo.staticInst.type, halClassInfo.createFunc);

    init_hal_callback_func(genInfo.halCallbackFunc, halClassInfo.callbackFunc);
    init_hal_callback_func(genInfo.halCallback2Func, halClassInfo.callback2Func);
}

static string get_hal_api_return_type(const string& aidlIntfName)
{
    return get_std_shared_ptr(aidlIntfName);
}

static string get_hal_api_func_name(ApccType apccType, const string& halNameLower)
{
    stringstream ss;
    /* e.g. create_hal_rpc */
    if (contain_suffix(halNameLower, "_rpc") ||
        expose_hal_rpc_server_api(apccType))
    {
        ss << "create_" << halNameLower;
    }
    else
    {
        ss << "create_" << halNameLower << "_rpc";
    }
    return ss.str();
}

static void init_hal_api_func(ApccGenInfo& genInfo, ApccType apccType, vector<FuncType>& halApiFunc)
{
    FuncType funcType;

    if (!genInfo.isAidlIntfRoot &&
        !expose_hal_rpc_server_api(apccType))
    {
        /* only expose hal api in root aidl intf */
        halApiFunc.clear();
        return;
    }

    funcType.returnType = get_hal_api_return_type(genInfo.aidlIntfName);
    funcType.funcName = get_hal_api_func_name(apccType, genInfo.halNameLower);
    funcType.param.push_back({HAL_API_PARAM_TYPE, HAL_API_PARAM_NAME});

    halApiFunc.push_back(funcType);
}

static void init_hal_msg_transport_func(ApccGenInfo& genInfo, vector<FuncType>& halMsgTransportFunc)
{
    FuncType funcType;

    funcType.returnType = CPP_VOID;
    funcType.funcName = genInfo.halMsgInfo.msgTransportName;
    funcType.param.push_back({CPP_VOID_POINTER, MV_NAME});

    halMsgTransportFunc.push_back(funcType);
}

static void init_aidl_callback_hal_name(const string& aidlCallbackName, AidlHalName& aidlCallbackHalName)
{
    if (valid_str(aidlCallbackName))
    {
        string halName = aidl_get_interface_short_name(aidlCallbackName);
        aidlCallbackHalName.name = halName;
        aidlCallbackHalName.nameUpper = get_format_string_in_upper(halName);
        aidlCallbackHalName.nameLower = get_format_string_in_lower(halName);
    }
    else
    {
        aidlCallbackHalName.name = "";
        aidlCallbackHalName.nameUpper = "";
        aidlCallbackHalName.nameLower = "";
    }
}

static void init_hal_intf_file_info(ApccGenInfo& genInfo, const string& aidlIntfName,
    const string& halName, HalIntfFileInfo& halIntfFileInfo)
{
    halIntfFileInfo.halBnName = concat_string(AIDL_BN_NAME, halName);
    halIntfFileInfo.halBnHeaderFile = get_hal_header_file_name(genInfo, halIntfFileInfo.halBnName);
    halIntfFileInfo.halHeaderFile = get_hal_header_file_name(genInfo, aidlIntfName);
}

static ApccGenInfo& new_gen_info()
{
    ApccGenInstance *inst = &sApccGenInstance;
    if (inst->genInfo)
    {
        delete inst->genInfo;
    }
    inst->genInfo = new ApccGenInfo;
    return *(inst->genInfo);
}

static void set_gen_info(ApccType apccType, const string& aidlIntfName, const AidlCallbackName& aidlCallbackName)
{
    ApccGenInstance *inst = &sApccGenInstance;
    ApccGenInfo& genInfo = new_gen_info();
    FUNC_ENTER();
    inst->ss.clear();

    genInfo.apccType = apccType;
    genInfo.aidlFileName = concat_string(aidlIntfName, AIDL_FILE_SUFFIX);
    genInfo.protoFileName = concat_string(aidlIntfName, PROTO_FILE_SUFFIX);
    genInfo.aidlIntfName = aidlIntfName;
    genInfo.aidlCallbackName = aidlCallbackName.callbackName;
    genInfo.aidlCallback2Name = aidlCallbackName.callback2Name;
    genInfo.aidlCallbackResultName = aidlCallbackName.callbackResultName;
    genInfo.aidlPackageName = aidl_get_package_name(aidlIntfName);
    genInfo.isMultiAidlIntf = apcc_is_multi_aidl_intf();
    genInfo.isCfmDefined = apcc_is_cfm_defined();
    genInfo.isSingleSomeip = apcc_is_single_someip_enabled() && genInfo.isMultiAidlIntf;
    genInfo.isAidlIntfRoot = aidl_is_intf_root(aidlIntfName);
    init_aidl_hal_root(genInfo, genInfo.aidlHalRoot);
    aidl_get_enum_list(genInfo.aidlEnumType);

    init_include_header(genInfo, genInfo.aidlIntfName, genInfo.aidlIntfIncludeHeader);
    init_include_header(genInfo, genInfo.aidlCallbackName, genInfo.aidlCallbackIncludeHeader);
    init_include_header(genInfo, genInfo.aidlCallback2Name, genInfo.aidlCallback2IncludeHeader);
    init_include_header(genInfo, genInfo.aidlCallbackResultName, genInfo.aidlCallbackResultIncludeHeader);

    init_aidl_intf_using_type(genInfo, genInfo.aidlIntfUsingType);
    init_using_type(genInfo, genInfo.aidlCallbackName, genInfo.aidlCallbackUsingType);
    init_using_type(genInfo, genInfo.aidlCallback2Name, genInfo.aidlCallback2UsingType);
    init_using_type(genInfo, genInfo.aidlCallbackResultName, genInfo.aidlCallbackResultUsingType);

    genInfo.halName = aidl_get_interface_short_name(aidlIntfName);
    genInfo.halNameUpper = get_format_string_in_upper(genInfo.halName);
    genInfo.halNameLower = get_format_string_in_lower(genInfo.halName);
    genInfo.halCallbackName = get_hal_name(genInfo.aidlCallbackName);
    genInfo.halCallback2Name = get_hal_name(genInfo.aidlCallback2Name);
    init_aidl_callback_hal_name(genInfo.aidlCallbackName, genInfo.aidlCallbackHalName);
    init_aidl_callback_hal_name(genInfo.aidlCallback2Name, genInfo.aidlCallback2HalName);
    init_aidl_callback_hal_name(genInfo.aidlCallbackResultName, genInfo.aidlCallbackResultHalName);

    aidl_get_hal_namespace(genInfo.aidlPackageName, genInfo.halNamespace);
    get_hal_rpc_namespace(genInfo.aidlPackageName, genInfo.halRpcNamespace);

    init_hal_intf_file_info(genInfo, genInfo.aidlIntfName, genInfo.halName, genInfo.halIntfFileInfo);
    init_hal_intf_file_info(genInfo, genInfo.aidlCallbackName, genInfo.halCallbackName, genInfo.halCallbackFileInfo);
    init_hal_intf_file_info(genInfo, genInfo.aidlCallback2Name, genInfo.halCallback2Name, genInfo.halCallback2FileInfo);

    init_hal_rpc_class_info(genInfo, genInfo.halRpcClassInfo);
    init_hal_msg_info(genInfo, genInfo.halMsgInfo);
    init_hal_status_info(genInfo, genInfo.halStatusInfo);

    genInfo.halPayload = concat_string(genInfo.aidlHalRoot.halName.name, HAL_MSG_PAYLOAD);
    genInfo.halCfmParam = concat_string(genInfo.halName, HAL_CFM_PARAM_SUFFIX);

    proto_get_message_req(aidlIntfName, genInfo.halReq);
    /* proto message req should not be empty */
    if (genInfo.halReq.empty())
    {
        loge("%s: empty req for aidl intf: %s", __func__, aidlIntfName.c_str());
        return;
    }
    // dump_string("hal req", genInfo.halReq);
    proto_get_message_cfm(aidlIntfName, genInfo.halCfm, false);
    // dump_string("hal cfm", genInfo.halCfm);
    proto_get_message_ind(aidlCallbackName.callbackName, genInfo.halInd);
    // dump_string("hal ind", genInfo.halInd);
    proto_get_message_ind(aidlCallbackName.callback2Name, genInfo.halInd2);
    // dump_string("hal ind2", genInfo.halInd2);
    proto_get_message_req(genInfo.aidlCallbackResultName, genInfo.halUpReq);
    // dump_string("hal up req", genInfo.halUpReq);
    proto_get_message_cfm(genInfo.aidlCallbackResultName, genInfo.halDownCfm, false);
    // dump_string("hal down cfm", genInfo.halDownCfm);

    proto_get_common_message(aidlIntfName, genInfo.halCommonMsg);
    // dump_string("hal common message", genInfo.halCommonMsg);
    proto_get_enum_type(aidlIntfName, genInfo.halEnumType);
    // dump_string("hal enum type", genInfo.halEnumType);

    proto_get_common_message(aidlCallbackName.callbackName, genInfo.halCallbackCommonMsg);
    // dump_string("hal callback common message", genInfo.halCallbackCommonMsg);
    proto_get_enum_type(aidlCallbackName.callbackName, genInfo.halCallbackEnumType);
    // dump_string("hal callback enum type", genInfo.halCallbackEnumType);

    proto_get_common_message(genInfo.aidlCallbackResultName, genInfo.halCallbackResultCommonMsg);
    // dump_string("hal callback result common message", genInfo.halCallbackResultCommonMsg);
    proto_get_enum_type(genInfo.aidlCallbackResultName, genInfo.halCallbackResultEnumType);
    // dump_string("hal callback result enum type", genInfo.halCallbackResultEnumType);

    init_hal_msg_def(genInfo.halNameUpper, genInfo.halReq, genInfo.halReqDef);
    // dump_string("hal req def", genInfo.halReqDef);
    init_hal_msg_def(genInfo.halNameUpper, genInfo.halCfm, genInfo.halCfmDef);
    // dump_string("hal cfm def", genInfo.halCfmDef);
    init_hal_msg_def(genInfo.aidlCallbackHalName.nameUpper, genInfo.halInd, genInfo.halIndDef);
    // dump_string("hal ind def", genInfo.halIndDef);
    init_hal_msg_def(genInfo.aidlCallback2HalName.nameUpper, genInfo.halInd2, genInfo.halInd2Def);
    // dump_string("hal ind2 def", genInfo.halInd2Def);
    init_hal_msg_def(genInfo.aidlCallbackResultHalName.nameUpper, genInfo.halUpReq, genInfo.halUpReqDef);
    // dump_string("hal up req def", genInfo.halUpReqDef);
    init_hal_msg_def(genInfo.halNameUpper, genInfo.halDownCfm, genInfo.halDownCfmDef);
    // dump_string("hal down cfm def", genInfo.halDownCfmDef);

    init_hal_common_func(genInfo, genInfo.aidlIntfName, genInfo.halName, genInfo.aidlIntfReturnType,
        genInfo.aidlIntfParamType, genInfo.halCommonFunc);
    // apcc_dump_func_type(genInfo.halCommonFunc, "halCommonFunc");
    init_hal_common_func(genInfo, genInfo.aidlCallbackName, genInfo.aidlCallbackHalName.name,
        genInfo.aidlCallbackReturnType, genInfo.aidlCallbackParamType, genInfo.halCommonCallback);
    init_hal_common_func(genInfo, genInfo.aidlCallback2Name, genInfo.aidlCallback2HalName.name,
        genInfo.aidlCallback2ReturnType, genInfo.aidlCallback2ParamType, genInfo.halCommonCallback2);
    init_hal_common_func(genInfo, genInfo.aidlCallbackResultName, genInfo.aidlCallbackResultHalName.name,
        genInfo.aidlCallbackResultReturnType, genInfo.aidlCallbackResultParamType, genInfo.halCommonCallbackResult);
    // apcc_dump_func_type(genInfo.halCommonCallbackResult, "halCommonCallbackResult");

    genInfo.handleHalCfm = get_handle_hal_cfm_flag(genInfo);
    logd("%s: handle hal cfm: %x", __func__, genInfo.handleHalCfm);

    init_hal_req_func(genInfo, genInfo.halCommonFunc, true, genInfo.halReqFunc);
    // apcc_dump_func_type(genInfo.halReqFunc, "halReqFunc");
    init_hal_ind_func(genInfo, genInfo.halCommonCallback, genInfo.aidlCallbackHalName, genInfo.halIndFunc);
    init_hal_ind_func(genInfo, genInfo.halCommonCallback2, genInfo.aidlCallback2HalName, genInfo.halInd2Func);
    init_hal_cfm_func(genInfo, genInfo.halCommonFunc, true, genInfo.halCfmFunc);
    // apcc_dump_func_type(genInfo.halCfmFunc, "halCfmFunc");
    init_hal_req_func(genInfo, genInfo.halCommonCallbackResult, false, genInfo.halUpReqFunc);
    init_hal_cfm_func(genInfo, genInfo.halCommonCallbackResult, false, genInfo.halDownCfmFunc);
    init_hal_cfm_struct(genInfo, genInfo.halCfmStruct);

    genInfo.isHalImplEnabled = apcc_is_hal_impl_enabled();
    genInfo.isAidlSyncApi = apcc_is_aidl_sync_api();
    genInfo.isValidateFuncSupported = apcc_is_validate_func_supported();

    init_hal_rpc_func(genInfo.aidlIntfName, genInfo.halCommonFunc, genInfo.halRpcFunc);
    // apcc_dump_func_type(genInfo.halRpcFunc, "halRpcFunc");
    init_hal_rpc_internal_func(genInfo, genInfo.halRpcInternalFunc);
    init_hal_rpc_override_func(genInfo, genInfo.halRpcOverrideFunc);
    init_hal_rpc_static_func(genInfo, genInfo.halRpcStaticFunc);
    init_hal_rpc_handle_ind_func(genInfo, genInfo.halRpcHandleIndFunc);
    init_hal_rpc_handle_ind2_func(genInfo, genInfo.halRpcHandleInd2Func);
    init_hal_rpc_handle_up_req_func(genInfo, genInfo.halRpcHandleUpReqFunc);
    init_hal_rpc_callback_util_info(genInfo, genInfo.halRpcCallbackUtilInfo);

    init_hal_api_func(genInfo, apccType, genInfo.halApiFunc);

    init_hal_msg_transport_func(genInfo, genInfo.halMsgTransportFunc);

    string callbackName = apcc_is_multi_callback() ? CALLBACK_NAME : CALLBACK_VAR_NAME;
    init_aidl_callback_func(callbackName, genInfo.aidlCallbackName, genInfo.halCommonCallback,
        genInfo.aidlCallbackFunc);
    init_aidl_callback_func(callbackName, genInfo.aidlCallback2Name, genInfo.halCommonCallback2,
        genInfo.aidlCallback2Func);
    init_aidl_callback_func(genInfo.halRpcClassInfo.callbackVarResult.name, genInfo.aidlCallbackResultName,
        genInfo.halCommonCallbackResult, genInfo.aidlCallbackResultFunc);

    init_hal_aidl_intf_info(genInfo, genInfo.halAidlIntfInfo);

    init_hal_result_info(genInfo, genInfo.halResultInfo);

    init_hal_rpc_service_info(genInfo, genInfo.halRpcServiceInfo);

    genInfo.intfProtoHeader = proto_get_header_file_name(genInfo.aidlIntfName);
    genInfo.callbackProtoHeader = proto_get_header_file_name(genInfo.aidlCallbackName);
    genInfo.callbackResultProtoHeader = proto_get_header_file_name(genInfo.aidlCallbackResultName);

    /* hal ind */
    init_hal_upstream_msg_param_struct(genInfo, genInfo.aidlCallbackName,
        genInfo.halCommonCallback, genInfo.halIndParamStruct);
    init_hal_upstream_msg_param_struct(genInfo, genInfo.aidlCallback2Name,
        genInfo.halCommonCallback2, genInfo.halInd2ParamStruct);
    /* hal req (upstream) */
    init_hal_upstream_msg_param_struct(genInfo, genInfo.aidlCallbackResultName,
        genInfo.halCommonCallbackResult, genInfo.halUpstreamReqParamStruct);

    init_hal_rpc_server_class_info(genInfo, genInfo.halRpcServerClassInfo);
    init_hal_callback_class_info(genInfo.aidlCallbackName, genInfo.halCallbackName,
        genInfo.halNameLower, genInfo.halCallbackClassInfo);
    init_hal_callback_class_info(genInfo.aidlCallback2Name, genInfo.halCallback2Name,
        genInfo.halNameLower, genInfo.halCallback2ClassInfo);
    init_hal_server_info(genInfo, genInfo.halServerInfo);
    init_hal_rpc_func(genInfo.aidlCallbackName, genInfo.halCommonCallback, genInfo.halCallbackFunc);
    init_hal_rpc_func(genInfo.aidlCallback2Name, genInfo.halCommonCallback2, genInfo.halCallback2Func);
    init_hal_rpc_server_func(genInfo.aidlIntfName, genInfo.halCommonFunc, genInfo.halRpcServerFunc);
    // apcc_dump_func_type(genInfo.halRpcServerFunc, "halRpcServerFunc");
    init_hal_rpc_server_req_func(genInfo, genInfo.halCommonFunc, genInfo.halRpcServerReqFunc);
    init_hal_rpc_server_ind_func(genInfo, genInfo.halCommonCallback, genInfo.aidlCallbackHalName, genInfo.halRpcServerIndFunc);
    init_hal_rpc_server_ind_func(genInfo, genInfo.halCommonCallback2, genInfo.aidlCallback2HalName, genInfo.halRpcServerInd2Func);
    init_hal_rpc_server_cfm_func(genInfo, genInfo.halCommonFunc, genInfo.halRpcServerCfmFunc);
    init_hal_rpc_server_handle_req_func(genInfo, genInfo.halRpcServerHandleReqFunc);
    init_hal_rpc_server_override_func(genInfo, genInfo.halRpcServerOverrideFunc);
    init_hal_rpc_server_public_func(genInfo, genInfo.halRpcServerPublicFunc);
    init_hal_rpc_server_static_func(genInfo, genInfo.halRpcServerStaticFunc);

    init_hal_rpc_server_req_param_struct(genInfo, genInfo.aidlIntfName,
        genInfo.halRpcServerFunc, genInfo.halRpcServerReqParamStruct);

    init_aidl_callback_info(genInfo, genInfo.aidlCallbackInfo);
    dump_aidl_callback_info(genInfo.aidlCallbackInfo);

    init_aidl_intf_return_info(genInfo, genInfo.aidlIntfReturnInfo);
    dump_aidl_intf_return_info(genInfo.aidlIntfReturnInfo);

    init_hal_class_info(genInfo, genInfo.halClassInfo);
    FUNC_LEAVE();
}

static void add_func_name(const string& funcName, const string& prefix, stringstream& ss)
{
    ss << prefix << funcName << '(';
}

static void add_func_name(const string& returnType, const string& funcName, const string& prefix, stringstream& ss)
{
    ss << prefix << returnType << " " << funcName << '(';
}

static void add_func_param(const vector<FuncParameter>& funcParam, const string& suffix, stringstream& ss)
{
    vector<FuncParameter>::size_type size = funcParam.size();

    if (size >= 1)
    {
        ss << funcParam[0].type << " " << funcParam[0].name;
        if (size >= 2)
        {
            for (vector<FuncParameter>::size_type index = 1; index < size; index++)
            {
                ss << ", " << funcParam[index].type << " " << funcParam[index].name;
            }
        }
    }
    ss << ')' << suffix;
}

static void add_func(const FuncType& funcType, const string& prefix, const string& suffix, stringstream& ss)
{
    add_func_name(funcType.returnType, funcType.funcName, prefix, ss);
    add_func_param(funcType.param, suffix, ss);
}

static void add_class_func(const FuncType& funcType, const string& className, const string& prefix, const string& suffix, stringstream& ss)
{
    const string funcName = get_func_name(className, funcType.funcName);
    add_func_name(funcType.returnType, funcName, prefix, ss);
    add_func_param(funcType.param, suffix, ss);
}

static void add_class_constructor(const FuncType& funcType, const string& className, const string& prefix, const string& suffix, stringstream& ss)
{
    const string funcName = get_func_name(className, funcType.funcName);
    add_func_name(funcName, prefix, ss);
    add_func_param(funcType.param, suffix, ss);
}

static void add_func_call(const string& funcName, const vector<string>& paramName,
    const string& prefix, const string& suffix, stringstream& ss)
{
    vector<FuncParameter>::size_type size = paramName.size();

    ss << prefix << funcName << '(';
    if (size >= 1)
    {
        ss << paramName[0];
        if (size >= 2)
        {
            for (vector<FuncParameter>::size_type index = 1; index < size; index++)
            {
                ss << ", " << paramName[index];
            }
        }
    }
    ss << ')' << suffix;
}

static string get_hal_name_in_lower(const string& aidlIntfName)
{
    /* e.g. IHalMod -> HalMod */
    const string halName = aidl_get_interface_short_name(aidlIntfName);
    /* e.g. HalMod -> hal_mod */
    return get_format_string_in_lower(halName);
}

static const string get_hal_file(const string& aidlIntfName, const string& baseName)
{
    string file_name;
    string hal_name;

    if (is_same_string(baseName, FILE_ANDROID_BP) ||
        is_same_string(baseName, FILE_SERVICE_SOURCE))
    {
        return baseName;
    }

    if (is_same_string(baseName, FILE_HAL_SERVICE_HEADER) ||
        is_same_string(baseName, FILE_HAL_SERVICE_SOURCE))
    {
        hal_name = apcc_get_package_name(aidl_get_package_name(aidlIntfName));
    }
    else
    {
        hal_name = get_hal_name_in_lower(aidlIntfName);
    }

    if (contain_suffix(hal_name, "_rpc") &&
        (is_same_string(baseName, FILE_HAL_RPC_HEADER) ||
        is_same_string(baseName, FILE_HAL_RPC_SOURCE)))
    {
        hal_name = hal_name.substr(0, hal_name.find("_rpc"));
    }

    file_name = hal_name + baseName.substr(string("hal").length(), string::npos);
    // logd("%s: file name: %s, hal name: %s, aidl intf name: %s, base name: %s", __func__,
    //     file_name.c_str(), hal_name.c_str(), aidlIntfName.c_str(), baseName.c_str());
    return file_name;
}

static stringstream& get_string_stream()
{
    ApccGenInstance *inst = &sApccGenInstance;
    return inst->ss;
}

static GenFunc get_gen_func(ApccType apccType)
{
    ApccGenInstance *inst = &sApccGenInstance;
    map<uint32_t, GenFunc>::iterator it = inst->genFunc.find(apccType);
    return (it != inst->genFunc.end()) ? it->second : nullptr;
}

static void call_gen_func(ApccType apccType)
{
    stringstream& ss = get_string_stream();
    GenFunc genFunc = get_gen_func(apccType);

    ss.clear();
    ss.str("");
    if (genFunc != nullptr)
    {
        genFunc(ss);
    }
}

static bool gen_file(const string& path, const string& fileName)
{
    stringstream& ss = get_string_stream();
    return create_new_file(path, fileName, ss);
}

static bool gen_file(ApccType apccType, const string& path, const string& fileName)
{
    call_gen_func(apccType);
    return gen_file(path, fileName);
}

/* ------------------------------------------------------------------------------------ */

void apcc_init_gen_info()
{
    ApccGenInstance *inst = &sApccGenInstance;
    logd("%s", __func__);
    init_apcc_aidl_type_map(inst->aidlTypeMap);
    init_cpp_type_map(inst->cppTypeMap);
    init_file_map(inst->fileMap);
    init_gen_func(inst->genFunc);
    logd("%s: done", __func__);
}

void apcc_set_gen_info(ApccType apccType, const string& aidlIntfName, const AidlCallbackName& aidlCallbackName)
{
    set_gen_info(apccType, aidlIntfName, aidlCallbackName);
}

const ApccGenInfo& apcc_get_gen_info()
{
    ApccGenInstance *inst = &sApccGenInstance;
    return *(inst->genInfo);
}

void apcc_add_comment(stringstream& ss)
{
    ss << APCC_GENERIC_COMMENT << endl;
}

void apcc_add_pragma(stringstream& ss)
{
    ss << CPP_PRAGMA << endl;
    ss << endl;
}

void apcc_add_sys_include_header(const string& includeFile, stringstream& ss)
{
    apcc_add_include_header(includeFile, '<', '>', ss);
}

void apcc_add_include_header(const string& includeFile, stringstream& ss)
{
    apcc_add_include_header(includeFile, '"', '"', ss);
}

void apcc_add_include_header(const string& includeFile, char prefixDelim, char suffixDelim, stringstream& ss)
{
    if (valid_str(includeFile))
    {
        /* e.g. #include <hal/IHal.h> */
        ss << CPP_INCLUDE << ' ' << prefixDelim << includeFile << suffixDelim << endl;
    }
}

void apcc_add_sys_include_header(const vector<CppTypeInfo>& cppTypeInfo, stringstream& ss)
{
    apcc_add_include_header(cppTypeInfo, '<', '>', ss);
}

void apcc_add_include_header(const vector<CppTypeInfo>& cppTypeInfo, stringstream& ss)
{
    apcc_add_include_header(cppTypeInfo, '"', '"', ss);
}

void apcc_add_include_header(const vector<CppTypeInfo>& cppTypeInfo, char prefixDelim, char suffixDelim, stringstream& ss)
{
    for (vector<CppTypeInfo>::size_type index = 0; index < cppTypeInfo.size(); index++)
    {
        apcc_add_include_header(cppTypeInfo[index].includeHeader, prefixDelim, suffixDelim, ss);
    }
}

void apcc_add_sys_include_header(const vector<string>& includeFile, stringstream& ss)
{
    apcc_add_include_header(includeFile, '<', '>', ss);
}

void apcc_add_include_header(const vector<string>& includeFile, stringstream& ss)
{
    apcc_add_include_header(includeFile, '"', '"', ss);
}

void apcc_add_include_header(const vector<string>& includeFile, char prefixDelim, char suffixDelim, stringstream& ss)
{
    for (vector<string>::size_type index = 0; index < includeFile.size(); index++)
    {
        apcc_add_include_header(includeFile[index], prefixDelim, suffixDelim, ss);
    }
}

void apcc_add_using(const string& leftVal, const string& rightVal, stringstream& ss)
{
    ss << CPP_USING << " " << leftVal << " = " << rightVal << ';' << endl;
}

void apcc_add_using(const string& usingType, stringstream& ss)
{
    if (valid_str(usingType))
    {
        /* e.g. "using std::vector;" */
        ss << CPP_USING << " " << usingType << ';' << endl;
    }
}

void apcc_add_using(const vector<CppTypeInfo>& cppTypeInfo, stringstream& ss)
{
    for (vector<CppTypeInfo>::size_type index = 0; index < cppTypeInfo.size(); index++)
    {
        apcc_add_using(cppTypeInfo[index].usingType, ss);
    }
}

void apcc_add_using(const vector<string>& usingType, stringstream& ss)
{
    for (vector<CppTypeInfo>::size_type index = 0; index < usingType.size(); index++)
    {
        apcc_add_using(usingType[index], ss);
    }
}

void apcc_add_std_using(const string& usingType, stringstream& ss)
{
    if (valid_str(usingType))
    {
        /* e.g. "using std::vector;" */
        ss << CPP_USING << " " << STD_NAMESPACE << "::" << usingType << ';' << endl;
    }
}

void apcc_add_define(const string& key, const string& value, uint32_t alignWidth, stringstream& ss)
{
    /* e.g. "#define SAMPLE 1" */
    if (key.empty())
    {
        ss << CPP_DEFINE << " " << value << endl;
    }
    else
    {
        ss << CPP_DEFINE << " " << get_align_string(key, alignWidth) << value << endl;
    }
}

void apcc_add_define(const string& key, const string& value, stringstream& ss)
{
    apcc_add_define(key, value, 30, ss);
}

void apcc_add_define(const map<string, string>& def, stringstream& ss, bool newLine)
{
    for (auto iter = def.begin(); iter != def.end(); iter++)
    {
        apcc_add_define(iter->first, iter->second, ss);
        if (newLine)
            ss << endl;
    }
}

void apcc_add_variable(const string& prefix, const string& varType, const string& varName, stringstream& ss)
{
    ss << prefix << varType << ' ' << varName << ';' << endl;
}

void apcc_add_variable(const string& prefix, const FuncParameter& var, stringstream& ss)
{
    apcc_add_variable(prefix, var.type, var.name, ss);
}

void apcc_add_static_variable(const FuncParameter& var, const string& prefix, stringstream& ss)
{
    ss << CPP_STATIC << ' ';
    apcc_add_variable(prefix, var, ss);
}

void apcc_add_static_variable(const vector<FuncParameter>& var, const string& prefix, stringstream& ss)
{
    for (vector<FuncParameter>::size_type index = 0; index < var.size(); index++)
    {
        apcc_add_static_variable(var[index], prefix, ss);
    }
}

void apcc_add_array_variable(const ArrayStruct& arrayVar, const string& prefix, stringstream& ss)
{
    vector<string>::size_type size = arrayVar.item.size();

    ss << prefix << arrayVar.itemType << ' ' << arrayVar.variableName << '[' << arrayVar.arraySize << ']';

    if (!arrayVar.item.empty())
    {
        ss << " =" << endl;
        add_module_begin("", ss);

        for (vector<string>::size_type index = 0; index < size - 1; index++)
        {
            ss << SPACES << arrayVar.item[index] << ',' << endl;
        }
        ss << SPACES << arrayVar.item[size - 1] << endl;
        add_module_end(ss);
    }
    else
    {
        ss << ';' << endl;;
    }
}

void apcc_add_static_array_variable(const ArrayStruct& arrayVar, const string& prefix, stringstream& ss)
{
    ss << CPP_STATIC << ' ';
    apcc_add_array_variable(arrayVar, prefix, ss);
}

void apcc_add_static_array_variable(const vector<ArrayStruct>& arrayVar, const string& prefix, stringstream& ss)
{
    for (vector<ArrayStruct>::size_type index = 0; index < arrayVar.size(); index++)
    {
        apcc_add_static_array_variable(arrayVar[index], prefix, ss);
    }
}

void apcc_add_struct(const StructType& structType, stringstream& ss)
{
    ss << endl;
    ss << CPP_STRUCT << ' ' << structType.name << endl;
    ss << "{" << endl;
    for (vector<FuncParameter>::size_type index = 1; index < structType.field.size(); index++)
    {
        ss << SPACES << structType.field[index].type << ' ' << structType.field[index].name << ';' << endl;
    }
    ss << "};" << endl;
}

void apcc_add_struct(const vector<StructType>& structType, stringstream& ss)
{
    for (vector<string>::size_type index = 0; index < structType.size(); index++)
    {
        apcc_add_struct(structType[index], ss);
    }
}

void apcc_add_typedef_struct(const StructType& structType, stringstream& ss)
{
    ss << endl;
    ss << CPP_TYPEDEF_STRUCT << endl;
    ss << "{" << endl;
    for (auto it: structType.field)
    {
        ss << SPACES << it.type << ' ' << it.name << ';' << endl;
    }
    ss << "} " << structType.name << ';' << endl;
}

void apcc_add_typedef_struct(const vector<StructType>& structType, stringstream& ss)
{
    for (vector<string>::size_type index = 0; index < structType.size(); index++)
    {
        apcc_add_typedef_struct(structType[index], ss);
    }
}

void apcc_add_return_func_call(const FuncType& funcType, const vector<string>& paramName, const string& prefix, stringstream& ss)
{
    ss << prefix << CPP_RETURN << " ";
    add_func_call(funcType.funcName, paramName, "", ";", ss);
}

void apcc_add_return_func_call(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    ss << prefix << CPP_RETURN << " ";
    apcc_add_func_call(funcType, "", ss);
}

void apcc_add_return_func_call(const string& funcName, const vector<FuncParameter>& param, const string& prefix, stringstream& ss)
{
    vector<string> paramName;
    init_param_name(param, paramName);
    ss << prefix << CPP_RETURN << " ";
    add_func_call(funcName, paramName, "", ";", ss);
}

void apcc_add_return_func_call(const string& funcName, const string& paramName, const string& prefix, stringstream& ss)
{
    ss << prefix << CPP_RETURN << " " << funcName << '(' << paramName << ");" << endl;
}

void apcc_add_func_call(const FuncType& funcType, const vector<string>& paramName, const string& prefix, stringstream& ss)
{
    add_func_call(funcType.funcName, paramName, prefix, ";", ss);
}

void apcc_add_func_call(const string& funcName, const vector<string>& paramName,
    const string& prefix, const string& suffix, stringstream& ss)
{
    add_func_call(funcName, paramName, prefix, suffix, ss);
}

void apcc_add_func_call(const FuncType& funcType, const string& prefix, const string& suffix, stringstream& ss)
{
    ss << prefix << funcType.funcName << '(' << get_func_param_name(funcType) << ')' << suffix;
}

void apcc_add_func_call(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    apcc_add_func_call(funcType.funcName, get_func_param_name(funcType), prefix, ss);
}

void apcc_add_func_call(const string& funcName, const vector<FuncParameter>& param, const string& prefix, stringstream& ss)
{
    vector<string> paramName;
    init_param_name(param, paramName);
    add_func_call(funcName, paramName, prefix, ";", ss);
}

void apcc_add_func_call(const string& funcName, const vector<string>& paramName, const string& prefix, stringstream& ss)
{
    add_func_call(funcName, paramName, prefix, ";", ss);
}

void apcc_add_func_call(const string& funcName, const string& paramName, const string& prefix,
    const string& suffix, stringstream& ss)
{
    ss << prefix << funcName << '(' << paramName << ')' << suffix;
}

void apcc_add_func_call(const string& funcName, const string& paramName, const string& prefix, stringstream& ss)
{
    ss << prefix << funcName << '(' << paramName << ");" << endl;
}

void apcc_add_func_call_no_param(const string& funcName, const string& prefix, stringstream& ss)
{
    ss << prefix << funcName << "();" << endl;
}

void apcc_add_class_func_call_no_param(const string& classPtr, const string& funcName, const string& prefix, stringstream& ss)
{
    ss << prefix << classPtr << "->";
    apcc_add_func_call_no_param(funcName, "", ss);
}

void apcc_add_class_func_call(const string& className, const FuncType& funcType, const string& prefix, stringstream& ss)
{
    ss << prefix << className << "::";
    apcc_add_func_call(funcType, "", ss);
}

void apcc_add_class_func_impl(const FuncType& funcType, const string& className,
    const string& prefix, const string& suffix, stringstream& ss)
{
    add_class_func(funcType, className, prefix, suffix, ss);
    ss << endl;
}

void apcc_add_class_constructor_impl(const FuncType& funcType, const string& className,
    const string& prefix, const string& suffix, stringstream& ss)
{
    add_class_constructor(funcType, className, prefix, suffix, ss);
    ss << endl;
}

void apcc_add_static_func_impl(const FuncType& funcType, stringstream& ss)
{
    ss << CPP_STATIC << " ";
    add_func(funcType, "", "", ss);
    ss << endl;
}

void apcc_add_func_impl(const FuncType& funcType, const string& prefix, const string& suffix, stringstream& ss)
{
    add_func(funcType, prefix, suffix, ss);
    ss << endl;
}

void apcc_add_class_constructor_def(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    /* no return type */
    add_func_name(funcType.funcName, prefix, ss);
    add_func_param(funcType.param, ";", ss);
}

void apcc_add_class_constructor_empty_impl(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    /* no return type */
    add_func_name(funcType.funcName, prefix, ss);
    add_func_param(funcType.param, "", ss);
    ss << MOD_BEGIN_STR;
    add_module_end(ss);
}

void apcc_add_func_def(const FuncType& funcType, const string& prefix, const string& suffix, stringstream& ss)
{
    add_func(funcType, prefix, suffix, ss);
}

void apcc_add_static_func_def(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    apcc_add_func_def(funcType, get_static_func_prefix(prefix), ";", ss);
}

void apcc_add_override_func_def(const FuncType& funcType, const string& prefix, stringstream& ss)
{
    apcc_add_func_def(funcType, prefix, " override;", ss);
}

void apcc_add_func_list(const vector<FuncType>& func, const string& prefix, const string& suffix, stringstream& ss)
{
    for (vector<FuncType>::size_type index = 0; index < func.size(); index++)
    {
        apcc_add_func_def(func[index], prefix, suffix, ss);
        ss << endl;
    }
}

void apcc_get_param_name(const FuncType& funcType, vector<string>& paramName)
{
    if (!funcType.param.empty())
    {
        for (vector<FuncParameter>::size_type index = 0; index < funcType.param.size(); index++)
        {
            paramName.push_back(funcType.param[index].name);
        }
    }
}

string apcc_get_param_name(const FuncType& funcType)
{
    return get_func_param_name(funcType);
}

void apcc_add_func_param(const FuncParameter& funcParam, const string& qualifier, const string& space, stringstream& ss)
{
    ss << space << qualifier << funcParam.type << " " << funcParam.name << ';' << endl;
}

void apcc_add_namespace_prefix(const vector<string>& halNamespace, stringstream& ss)
{
    if (!halNamespace.empty())
    {
        for (vector<string>::size_type index = 0; index < halNamespace.size(); index++)
        {
            ss << CPP_NAMESPACE << ' ' << halNamespace[index] << MOD_BEGIN_STR << endl;
        }
        ss << endl;
    }
}

void apcc_add_namespace_suffix(const vector<string>& halNamespace, stringstream& ss)
{
    if (!halNamespace.empty())
    {
        for (auto rit = halNamespace.crbegin(); rit != halNamespace.crend(); ++rit)
        {
            ss << "}  // " << CPP_NAMESPACE << ' ' << *rit << endl;
        }
    }
}

bool apcc_exist_return_value(const FuncType& funcType)
{
    return !apcc_is_cpp_type(funcType.returnType, CPP_VOID) &&
            !aidl_is_interface(funcType.returnType);
}

void apcc_add_delim_line(stringstream& ss)
{
    ss << DELIM_LINE << endl;
    ss << endl;
}

void apcc_add_for_line(const string& itemVar, const string& itemList,
    const string& prefix, const string& suffix, stringstream& ss)
{
    ss << prefix << CPP_FOR << " (" << CPP_CONST << " " << CPP_AUTO <<
        " &" << itemVar << " : " << itemList << ')' << suffix << endl;
}

void apcc_add_for_line(const string& itemType, const string& itemVar,
    const string& initVal, const string& maxVal, const string& prefix,
    const string& suffix, stringstream& ss)
{
    ss << prefix << CPP_FOR << " (" << itemType << " " <<
        itemVar << " = " << initVal << "; " << itemVar <<
        " < " << maxVal << "; " << itemVar << "++)" << suffix << endl;
}

void apcc_add_switch_line(const string& enumVar, const string& prefix,
    const string& suffix, stringstream& ss)
{
    ss << prefix << CPP_SWITCH << " (" << enumVar << ")" << suffix << endl;
}

void apcc_add_case_line(const string& enumVar, const string& prefix,
    const string& suffix, stringstream& ss)
{
    ss << prefix << CPP_CASE << " " << enumVar << ":" << suffix << endl;
}

void apcc_add_break_line(const string& prefix, stringstream& ss)
{
    ss << prefix << CPP_BREAK << ";" << endl;
}

void apcc_add_default_line(const string& prefix, const string& suffix, stringstream& ss)
{
    ss << prefix << CPP_DEFAULT << ":" << suffix << endl;
}

void apcc_add_return_line(const string& prefix, stringstream& ss)
{
    ss << prefix << CPP_RETURN << ";" << endl;
}

void apcc_add_return_line(const string& prefix, const string& result, stringstream& ss)
{
    ss << prefix << CPP_RETURN << " " << result << ";" << endl;
}

void apcc_add_if_line(const string& prefix, const string& flag, stringstream& ss)
{
    ss << prefix << CPP_IF << " " << '(' << flag << ')' << endl;
}

void apcc_add_if_line(const string& prefix, const string& flag, const string& suffix, stringstream& ss)
{
    ss << prefix << CPP_IF << " " << '(' << flag << ')' << suffix << endl;
}

void apcc_add_new_line(const string& varName, const string& typeName, const string& prefix, stringstream& ss)
{
    ss << prefix << varName << " = " << CPP_NEW << " " << typeName << ";" << endl;
}

string apcc_create_hal_status_error_unknown(const ApccGenInfo& genInfo)
{
    const HalStatusInfo& halStatusInfo = genInfo.halStatusInfo;
    stringstream ss;
    if (halStatusInfo.existHalStatusType &&
        halStatusInfo.existCreateHalStatusFunc)
    {
        ss << halStatusInfo.createHalStatusFunc[0].funcName << '(' <<
            apcc_create_hal_status(halStatusInfo.aidlHalStatusType, halStatusInfo.defaultHalStatusCode) << ')';
    }
    else
    {
        ss << CREATE_SCOPED_ASTATUS_ERROR_FUNC << "()";
    }
    return ss.str();
}

void apcc_init_class_func_type(const string& className, const FuncType& funcType, FuncType& newFuncType)
{
    newFuncType.returnType = funcType.returnType;
    newFuncType.funcName = class_func_name(className, funcType.funcName);
    newFuncType.param = funcType.param;
}

void apcc_add_intf_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> usingType;

    add_unique_str(genInfo.aidlIntfUsingType, usingType);
    add_unique_using_type(genInfo.aidlIntfReturnType, usingType);
    add_unique_using_type(genInfo.aidlIntfParamType, usingType);
    add_unique_str(STD_NAMESPACE "::" CPP_VECTOR, usingType);

    apcc_add_using(usingType, ss);
    ss << endl;
}

void apcc_add_intf_internal_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* Add AIDL intf internal using type retrieved from Hal common message and enum type */
    add_hal_common_msg_using_type(genInfo.aidlPackageName, genInfo.aidlIntfName,
        genInfo.halCommonMsg, ss);
    add_hal_enum_type_using_type(genInfo.aidlPackageName, genInfo.aidlIntfName,
        genInfo.halEnumType, ss);
}

void apcc_add_callback_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> usingType;

    add_unique_str(genInfo.aidlCallbackUsingType, usingType);
    add_unique_using_type(genInfo.aidlCallbackReturnType, usingType);
    add_unique_using_type(genInfo.aidlCallbackParamType, usingType);

    apcc_add_using(usingType, ss);
    ss << endl;
}

void apcc_add_callback_internal_using(const ApccGenInfo& genInfo, stringstream& ss)
{
    /* Add AIDL callback internal using type retrieved from Hal common message and enum type */
    add_hal_common_msg_using_type(genInfo.aidlPackageName, genInfo.aidlCallbackName,
        genInfo.halCallbackCommonMsg, ss);
    add_hal_enum_type_using_type(genInfo.aidlPackageName, genInfo.aidlCallbackName,
        genInfo.halCallbackEnumType, ss);
}

void apcc_get_all_using(const ApccGenInfo& genInfo, vector<string>& usingType)
{
    add_unique_using_type(genInfo.aidlIntfUsingType, usingType);
    add_unique_using_type(genInfo.aidlCallbackUsingType, usingType);
    add_unique_using_type(genInfo.aidlCallback2UsingType, usingType);
    add_unique_using_type(genInfo.aidlCallbackResultUsingType, usingType);

    add_unique_using_type(genInfo.aidlIntfReturnType, usingType);
    add_unique_using_type(genInfo.aidlIntfParamType, usingType);

    add_unique_using_type(genInfo.aidlCallbackReturnType, usingType);
    add_unique_using_type(genInfo.aidlCallbackParamType, usingType);

    add_unique_using_type(genInfo.aidlCallbackResultReturnType, usingType);
    add_unique_using_type(genInfo.aidlCallbackResultParamType, usingType);

    /* using std::vector; */
    add_unique_str(STD_NAMESPACE "::" CPP_VECTOR, usingType);
    /* using using std::shared_ptr; */
    add_unique_str(STD_NAMESPACE "::" CPP_SHARED_PTR, usingType);
    /* using ndk::ScopedAStatus; */
    add_unique_str(NDK_SCOPED_ASTATUS, usingType);
}

void apcc_add_intf_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> includeHeader;

    add_unique_str(genInfo.aidlIntfIncludeHeader, includeHeader);
    add_unique_include_header(genInfo.aidlIntfReturnType, includeHeader);
    add_unique_include_header(genInfo.aidlIntfParamType, includeHeader);
    add_unique_str(CPP_VECTOR, includeHeader);

    apcc_add_sys_include_header(includeHeader, ss);
    ss << endl;
}

void apcc_add_callback_include(const ApccGenInfo& genInfo, stringstream& ss)
{
    vector<string> includeHeader;

    add_unique_str(genInfo.aidlCallbackIncludeHeader, includeHeader);
    add_unique_str(genInfo.aidlCallback2IncludeHeader, includeHeader);
    add_unique_include_header(genInfo.aidlCallbackReturnType, includeHeader);
    add_unique_include_header(genInfo.aidlCallbackParamType, includeHeader);
    add_unique_include_header(genInfo.aidlCallback2ParamType, includeHeader);

    apcc_add_sys_include_header(includeHeader, ss);
    ss << endl;
}

void apcc_get_all_include(const ApccGenInfo& genInfo, vector<string>& includeHeader)
{
    /* AIDL intf header */
    add_unique_str(aidl_get_include_header(genInfo.aidlPackageName, genInfo.aidlIntfName), includeHeader);

    add_unique_str(genInfo.aidlIntfIncludeHeader, includeHeader);
    add_unique_str(genInfo.aidlCallbackIncludeHeader, includeHeader);
    add_unique_str(genInfo.aidlCallback2IncludeHeader, includeHeader);
    add_unique_str(genInfo.aidlCallbackResultIncludeHeader, includeHeader);

    add_unique_include_header(genInfo.aidlIntfReturnType, includeHeader);
    add_unique_include_header(genInfo.aidlIntfParamType, includeHeader);

    add_unique_include_header(genInfo.aidlCallbackReturnType, includeHeader);
    add_unique_include_header(genInfo.aidlCallbackParamType, includeHeader);

    add_unique_include_header(genInfo.aidlCallbackResultReturnType, includeHeader);
    add_unique_include_header(genInfo.aidlCallbackResultParamType, includeHeader);

    /* #include <vector> */
    add_unique_str(CPP_VECTOR, includeHeader);
}

void apcc_add_hal_msg_header(const ApccGenInfo& genInfo, stringstream& ss)
{
    string hal_msg_header = apcc_get_gen_file_name(genInfo.aidlIntfName, APCC_TYPE_HAL_MSG_HEADER);
    string hal_callback_msg_header;

    /* hal_msg.h (part1. aidl intf) */
    apcc_add_include_header(hal_msg_header, ss);

    /* hal_msg.h (part2. aidl callback) */
    if (valid_str(genInfo.aidlCallbackName))
    {
        hal_callback_msg_header = apcc_get_gen_file_name(genInfo.aidlCallbackName, APCC_TYPE_HAL_MSG_HEADER);
        if (!is_same_string(hal_msg_header, hal_callback_msg_header))
        {
            apcc_add_include_header(hal_callback_msg_header, ss);
        }
    }

    /* hal_msg.h (part3. aidl callback2) */
    if (valid_str(genInfo.aidlCallback2Name))
    {
        hal_callback_msg_header = apcc_get_gen_file_name(genInfo.aidlCallback2Name, APCC_TYPE_HAL_MSG_HEADER);
        if (!is_same_string(hal_msg_header, hal_callback_msg_header))
        {
            apcc_add_include_header(hal_callback_msg_header, ss);
        }
    }
}

const AidlIntfReturnInfo& apcc_get_aidl_intf_return_info(const ApccGenInfo& genInfo, const string& intfName)
{
    static const AidlIntfReturnInfo emptyAidlIntfReturnInfo;
    auto it = genInfo.aidlIntfReturnInfo.find(intfName);
    return (it != genInfo.aidlIntfReturnInfo.end()) ? it->second : emptyAidlIntfReturnInfo;
}

const AidlCallbackInfo& apcc_get_aidl_call_back_info(const ApccGenInfo& genInfo, const string& intfName)
{
    static const AidlCallbackInfo emptyAidlCallbackInfo;
    auto it = genInfo.aidlCallbackInfo.find(intfName);
    return (it != genInfo.aidlCallbackInfo.end()) ? it->second : emptyAidlCallbackInfo;
}

bool apcc_is_enum_type(const string& typeName)
{
    const ApccGenInfo& genInfo = apcc_get_gen_info();
    return aidl_is_enum_type(typeName) ||
            contain_string(genInfo.halEnumType, typeName) ||
            contain_string(genInfo.halCallbackEnumType, typeName);
}

CppTypeT apcc_get_cpp_type(const string& cppTypeName)
{
    return get_cpp_type(cppTypeName);
}

bool apcc_is_basic_type(const string& cppTypeName)
{
    return apcc_get_cpp_type(cppTypeName) == CPP_TYPE_BASIC;
}

bool apcc_is_cpp_type(const string& cppType, const char* typeName)
{
    return is_same_string(cppType, typeName);
}

bool apcc_is_void_type(const string& cppType)
{
    return is_cpp_void(cppType);
}

bool apcc_is_bool_type(const string& cppType)
{
    return apcc_is_cpp_type(cppType, CPP_BOOL);
}

bool apcc_is_pair_type(const string& cppType)
{
    return contain_string(cppType, CPP_PAIR "<");
}

bool apcc_is_shared_ptr_type(const string& cppType)
{
    return contain_string(cppType, CPP_SHARED_PTR "<");
}

bool apcc_is_out_param_type(const string& cppType, const string& typeName)
{
    /* e.g. T* out_param */
    return contain_char(cppType, '*') && (typeName.find(AIDL_PARAM_OUT_PREFIX) == 0);
}

string apcc_get_out_param_type(const string& cppType)
{
    string str = cppType;
    std::size_t pos = str.find_last_of(CPP_POINTER);
    return (pos != string::npos) ? str.substr(0, pos) : str;
}

string apcc_get_shared_ptr_type(const string& type)
{
    return get_shared_ptr(type);
}

string apcc_get_std_shared_ptr_type(const string& type)
{
    return get_std_shared_ptr(type);
}

string apcc_get_cpp_vector_type(const string& itemType)
{
    return get_cpp_vector_type(itemType);
}

string apcc_get_package_name(const string& aidlPackageName)
{
    std::size_t pos = aidlPackageName.find_last_of('.');
    return pos != string::npos ? aidlPackageName.substr(pos+1, string::npos) : aidlPackageName;
}

bool apcc_is_validate_func_with_lock(const string& funcName)
{
    /* TODO */
    return false;
}

const string apcc_get_gen_file_name(const string& aidlIntfName, ApccType apccType)
{
    string hal_file_name = apcc_get_hal_file_name(apccType);
    return !hal_file_name.empty() ? get_hal_file(aidlIntfName, hal_file_name) : "";
}

const string apcc_get_gen_folder(ApccType apccType)
{
    return apcc_get_hal_folder(apccType);
}

const string apcc_get_hal_file_name(ApccType apccType)
{
    ApccGenInstance *inst = &sApccGenInstance;
    map<uint32_t, HalFile>::iterator it = inst->fileMap.find(apccType);
    return (it != inst->fileMap.end()) ? it->second.name : "";
}

const string apcc_get_hal_folder(ApccType apccType)
{
    ApccGenInstance *inst = &sApccGenInstance;
    map<uint32_t, HalFile>::iterator it = inst->fileMap.find(apccType);
    return (it != inst->fileMap.end()) ? it->second.folder : "";
}

const string apcc_get_full_path(ApccType apccType, const string& parentPath)
{
    const string relativePath = apcc_get_gen_folder(apccType);
    return get_path(parentPath, relativePath);
}

bool apcc_get_gen_flag(const string& aidlIntfName, ApccType apccType)
{
    bool isAidlIntfRoot = aidl_is_intf_root(aidlIntfName);
    switch (apccType)
    {
        case APCC_TYPE_ANDROID_BP:
        case APCC_TYPE_HAL_MESSAGE_DEF_HEADER:
        case APCC_TYPE_HAL_MSG_HEADER:
        case APCC_TYPE_HAL_MSG_SOURCE:
        {
            return apcc_is_test_mode_enabled();
        }
        case APCC_TYPE_HAL_API_HEADER:
        {
            return isAidlIntfRoot ? true : false;
        }
        case APCC_TYPE_HAL_STATUS_UTIL_HEADER:
            /* fall-through */
        case APCC_TYPE_HAL_STATUS_UTIL_SOURCE:
        {
            return gen_hal_status_util_flag(aidlIntfName);
        }

        case APCC_TYPE_SERVICE_SOURCE:
            /* fall-through */
        case APCC_TYPE_HAL_SERVICE_HEADER:
        case APCC_TYPE_HAL_SERVICE_SOURCE:
        case APCC_TYPE_HAL_SERVER_API_HEADER:
        case APCC_TYPE_SERVER_ANDROID_BP:
        case APCC_TYPE_SERVER_LIB_ANDROID_BP:
        case APCC_TYPE_SERVER_MAIN_ANDROID_BP:
        {
            return apcc_is_gen_server_enabled() && isAidlIntfRoot;
        }

        case APCC_TYPE_HAL_IMPL_HEADER:
        {
            return apcc_is_gen_server_enabled() && apcc_is_gen_hal_impl_stub_enabled();
        }

        case APCC_TYPE_HAL_IMPL_SOURCE:
        {
            return apcc_is_gen_server_enabled() && (isAidlIntfRoot || apcc_is_gen_hal_impl_stub_enabled());
        }

        case APCC_TYPE_HAL_RPC_SERVER_HEADER:
            /* fall-through */
        case APCC_TYPE_HAL_RPC_SERVER_SOURCE:
        {
            return apcc_is_gen_server_enabled();
        }

        case APCC_TYPE_SERVER_HAL_STATUS_UTIL_HEADER:
            /* fall-through */
        case APCC_TYPE_SERVER_HAL_STATUS_UTIL_SOURCE:
        {
            return apcc_is_gen_server_enabled() && gen_hal_status_util_flag(aidlIntfName);
        }
        default:
        {
            return true;
        }
    }
}

bool apcc_create_file(const string& aidlIntfName, const AidlCallbackName& aidlCallbackName, const ApccFile& apccFile)
{
    if (!apccFile.gen)
    {
        // logw("%s: not generate source code for file: %s", __func__, apccFile.name.c_str());
        return true;
    }
    if (!apcc_is_remove_file_enabled() &&
        exist_file(apccFile.path, apccFile.name))
    {
        /* source file exist, return */
        logd("%s: source file %s exist", __func__, apccFile.name.c_str());
        return true;
    }
    // logi("%s: %s", __func__, apccFile.name.c_str());
    apcc_set_gen_info(apccFile.type, aidlIntfName, aidlCallbackName);
    return gen_file(apccFile.type, apccFile.path, apccFile.name);
}

bool apcc_get_cpp_pair_type(const string& pairType, string& firstItemType, string& secondItemType)
{
    size_t begin_pos, end_pos;
    begin_pos = pairType.find_first_of('<');
    end_pos = pairType.find_last_of('>');
    if ((begin_pos != string::npos) &&
        (end_pos != string::npos) &&
        (end_pos > begin_pos))
    {
        string str = pairType.substr(begin_pos + 1, end_pos - begin_pos - 1);
        size_t pos = str.find_last_of(',');
        if (pos != string::npos)
        {
            firstItemType = str.substr(0, pos);
            secondItemType = str.substr(pos + 1);
            trim(secondItemType);
            return true;
        }
    }
    return false;
}

string apcc_get_std_enable_shared_from_this(const string& className)
{
    stringstream ss;
    ss << STD_NAMESPACE << "::" << STD_ENABLE_SHARED_FROM_THIS << '<' << className << '>';
    return ss.str();
}

string apcc_get_static_proxy_func(const string& className)
{
    stringstream ss;
    ss << className << "::" << GET_STATIC_PROXY_NAME << "()";
    return ss.str();
}

string apcc_create_hal_status(const string& halStatusCode, const string& status)
{
    stringstream ss;
    ss << halStatusCode << "::" << status;
    return ss.str();
}

string apcc_create_hal_scoped_astatus(const string& halStatusCode, const string& statusCode)
{
    stringstream ss;
    ss << SCOPED_ASTATUS << "::" << FROM_SERVICE_SPECIFIC_ERROR_FUNC <<
        '(' << statusCode << ")";
    return ss.str();
}

void apcc_add_comment_hal_msg(const string& halNameLower, const string& comment, stringstream& ss)
{
    ss << get_comment_hal_msg(halNameLower, comment) << endl;
}

void apcc_dump_cpp_info(const vector<CppTypeInfo>& cppTypeInfo)
{
    logd("%s: size %d", __func__, cppTypeInfo.size());
    for (vector<CppTypeInfo>::size_type index = 0; index < cppTypeInfo.size(); index++)
    {
        const CppTypeInfo& cti = cppTypeInfo[index];
        logd("%s: name: %s, type: 0x%x, include: %s, using: %s", __func__,
            cti.name.c_str(), cti.type, cti.includeHeader.c_str(), cti.usingType.c_str());
    }
}

void apcc_dump_func_type(const FuncType& funcType)
{
    logd("%s: return type: %s, func name: %s, param: %s", __func__,
        funcType.returnType.c_str(), funcType.funcName.c_str(),
        get_param_string(funcType.param).c_str());
}

void apcc_dump_func_type(const vector<FuncType>& funcType, const char* info)
{
    logd("%s: %s, total func: %d", __func__, info, funcType.size());
    for (auto it: funcType)
        apcc_dump_func_type(it);
}

void apcc_dump_func_param(const FuncParameter& funcParam, const string& info)
{
    logd("%s: %s %s", info.c_str(), funcParam.type.c_str(), funcParam.name.c_str());
}

void apcc_dump_func_param(const vector<FuncParameter>& funcParam)
{
    logd("%s: param size %d (%s)", __func__, funcParam.size(), get_param_string(funcParam).c_str());
}

bool apcc_exist_create_hal_status_func()
{
    return EXIST_CREATE_HAL_STATUS_FUNC;
}

const string& apcc_get_ndk_lib()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.ndkLib;
}

bool apcc_is_multi_aidl_intf()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.aidlIntfNumber > 1;
}

bool apcc_is_aidl_sync_api()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.aidlSyncApi;
}

bool apcc_is_multi_callback()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.multiCallback;
}

bool apcc_is_validate_func_supported()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.validateFunc;
}

bool apcc_is_remove_file_enabled()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.removeFile;
}

bool apcc_is_cfm_defined()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.defineCfm;
}

bool apcc_is_gen_server_enabled()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.server;
}

bool apcc_is_gen_hal_impl_stub_enabled()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.halImplStub;
}

bool apcc_is_single_someip_enabled()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.singleSomeip;
}

bool apcc_is_remove_intf_enabled()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.removeIntf;
}

bool apcc_is_test_mode_enabled()
{
    const ApccInfo& apccInfo = apcc_get_info();
    return apccInfo.param.test;
}

bool apcc_is_hal_impl_enabled()
{
    return ENABLE_HAL_IMPL;
}

bool apcc_is_clear_callback_required()
{
    return IS_CLEAR_CALLBACK_REQUIRED;
}
