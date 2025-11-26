/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "apcc_common.h"
#include "aidl_util.h"
#include "util.h"

#define HAL_LIB_FUNC_INDEX_OFFSET   2

void apcc_init_gen_info();

void apcc_set_gen_info(ApccType apccType, const string& aidlIntfName, const AidlCallbackName& aidlCallbackName);

const ApccGenInfo& apcc_get_gen_info();

void apcc_add_comment(stringstream& ss);

void apcc_add_pragma(stringstream& ss);

void apcc_add_sys_include_header(const string& includeFile, stringstream& ss);
void apcc_add_include_header(const string& includeFile, stringstream& ss);
void apcc_add_include_header(const string& includeFile, char prefixDelim, char suffixDelim, stringstream& ss);

void apcc_add_sys_include_header(const vector<CppTypeInfo>& cppTypeInfo, stringstream& ss);
void apcc_add_include_header(const vector<CppTypeInfo>& cppTypeInfo, stringstream& ss);
void apcc_add_include_header(const vector<CppTypeInfo>& cppTypeInfo, char prefixDelim, char suffixDelim, stringstream& ss);

void apcc_add_sys_include_header(const vector<string>& includeFile, stringstream& ss);
void apcc_add_include_header(const vector<string>& includeFile, stringstream& ss);
void apcc_add_include_header(const vector<string>& includeFile, char prefixDelim, char suffixDelim, stringstream& ss);

void apcc_add_using(const string& leftVal, const string& rightVal, stringstream& ss);
void apcc_add_using(const string& usingType, stringstream& ss);
void apcc_add_using(const vector<CppTypeInfo>& cppTypeInfo, stringstream& ss);
void apcc_add_using(const vector<string>& usingType, stringstream& ss);
void apcc_add_std_using(const string& usingType, stringstream& ss);

void apcc_add_define(const string& key, const string& value, uint32_t alightWidth, stringstream& ss);
void apcc_add_define(const string& key, const string& value, stringstream& ss);
void apcc_add_define(const map<string, string>& def, stringstream& ss, bool newLine = true);

void apcc_add_variable(const string& prefix, const string& varType, const string& varName, stringstream& ss);
void apcc_add_variable(const string& prefix, const FuncParameter& var, stringstream& ss);

void apcc_add_static_variable(const FuncParameter& var, const string& prefix, stringstream& ss);
void apcc_add_static_variable(const vector<FuncParameter>& var, const string& prefix, stringstream& ss);

void apcc_add_array_variable(const ArrayStruct& arrayVar, const string& prefix, stringstream& ss);

void apcc_add_static_array_variable(const ArrayStruct& arrayVar, const string& prefix, stringstream& ss);
void apcc_add_static_array_variable(const vector<ArrayStruct>& arrayVar, const string& prefix, stringstream& ss);

void apcc_add_struct(const StructType& structType, stringstream& ss);
void apcc_add_struct(const vector<StructType>& structType, stringstream& ss);

void apcc_add_typedef_struct(const StructType& structType, stringstream& ss);
void apcc_add_typedef_struct(const vector<StructType>& structType, stringstream& ss);

void apcc_add_class_func_impl(const FuncType& funcType, const string& className,
    const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_static_func_impl(const FuncType& funcType, stringstream& ss);
void apcc_add_func_impl(const FuncType& funcType, const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_return_func_call(const FuncType& funcType, const vector<string>& paramName, const string& prefix, stringstream& ss);
void apcc_add_return_func_call(const FuncType& funcType, const string& prefix, stringstream& ss);
void apcc_add_return_func_call(const string& funcName, const vector<FuncParameter>& param, const string& prefix, stringstream& ss);
void apcc_add_return_func_call(const string& funcName, const string& paramName, const string& prefix, stringstream& ss);
void apcc_add_func_call(const string& funcName, const vector<string>& paramName,
    const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_func_call(const FuncType& funcType, const vector<string>& paramName, const string& prefix, stringstream& ss);
void apcc_add_func_call(const FuncType& funcType, const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_func_call(const FuncType& funcType, const string& prefix, stringstream& ss);
void apcc_add_func_call(const string& funcName, const vector<FuncParameter>& param, const string& prefix, stringstream& ss);
void apcc_add_func_call(const string& funcName, const vector<string>& paramName, const string& prefix, stringstream& ss);
void apcc_add_func_call(const string& funcName, const string& paramName, const string& prefix, stringstream& ss);
void apcc_add_func_call(const string& funcName, const string& paramName, const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_func_call_no_param(const string& funcName, const string& prefix, stringstream& ss);
void apcc_add_class_func_call_no_param(const string& classPtr, const string& funcName, const string& prefix, stringstream& ss);
void apcc_add_class_func_call(const string& className, const FuncType& funcType, const string& prefix, stringstream& ss);
void apcc_add_class_constructor_impl(const FuncType& funcType, const string& className, const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_class_constructor_def(const FuncType& funcType, const string& prefix, stringstream& ss);
void apcc_add_class_constructor_empty_impl(const FuncType& funcType, const string& prefix, stringstream& ss);
void apcc_add_func_def(const FuncType& funcType, const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_static_func_def(const FuncType& funcType, const string& prefix, stringstream& ss);
void apcc_add_override_func_def(const FuncType& funcType, const string& prefix, stringstream& ss);
void apcc_add_func_list(const vector<FuncType>& func, const string& prefix, const string& suffix, stringstream& ss);

void apcc_get_param_name(const FuncType& funcType, vector<string>& paramName);
string apcc_get_param_name(const FuncType& funcType);

void apcc_add_func_param(const FuncParameter& funcParam, const string& qualifier,
    const string& space, stringstream& ss);

void apcc_add_namespace_prefix(const vector<string>& halNamespace, stringstream& ss);
void apcc_add_namespace_suffix(const vector<string>& halNamespace, stringstream& ss);

bool apcc_exist_return_value(const FuncType& funcType);

void apcc_add_delim_line(stringstream& ss);

void apcc_add_for_line(const string& itemVar, const string& itemList,
    const string& prefix, const string& suffix, stringstream& ss);
void apcc_add_for_line(const string& itemType, const string& itemVar,
    const string& initVal, const string& maxVal, const string& prefix,
    const string& suffix, stringstream& ss);

void apcc_add_switch_line(const string& enumVar, const string& prefix,
    const string& suffix, stringstream& ss);

void apcc_add_case_line(const string& enumVar, const string& prefix,
    const string& suffix, stringstream& ss);

void apcc_add_break_line(const string& prefix, stringstream& ss);

void apcc_add_default_line(const string& prefix, const string& suffix, stringstream& ss);

void apcc_add_return_line(const string& prefix, stringstream& ss);

void apcc_add_return_line(const string& prefix, const string& result, stringstream& ss);

void apcc_add_if_line(const string& prefix, const string& flag, stringstream& ss);
void apcc_add_if_line(const string& prefix, const string& flag, const string& suffix, stringstream& ss);

void apcc_add_new_line(const string& varName, const string& typeName, const string& prefix, stringstream& ss);

string apcc_create_hal_status_error_unknown(const ApccGenInfo& genInfo);

void apcc_init_class_func_type(const string& className, const FuncType& funcType, FuncType& newFuncType);

void apcc_add_intf_using(const ApccGenInfo& genInfo, stringstream& ss);
void apcc_add_intf_internal_using(const ApccGenInfo& genInfo, stringstream& ss);
void apcc_add_callback_using(const ApccGenInfo& genInfo, stringstream& ss);
void apcc_add_callback_internal_using(const ApccGenInfo& genInfo, stringstream& ss);

void apcc_get_all_using(const ApccGenInfo& genInfo, vector<string>& usingType);

void apcc_add_intf_include(const ApccGenInfo& genInfo, stringstream& ss);
void apcc_add_callback_include(const ApccGenInfo& genInfo, stringstream& ss);

void apcc_get_all_include(const ApccGenInfo& genInfo, vector<string>& includeHeader);

void apcc_add_hal_msg_header(const ApccGenInfo& genInfo, stringstream& ss);

const AidlIntfReturnInfo& apcc_get_aidl_intf_return_info(const ApccGenInfo& genInfo, const string& intfName);

const AidlCallbackInfo& apcc_get_aidl_call_back_info(const ApccGenInfo& genInfo, const string& intfType);

bool apcc_is_enum_type(const string& typeName);

CppTypeT apcc_get_cpp_type(const string& cppTypeName);

bool apcc_is_basic_type(const string& cppTypeName);

bool apcc_is_cpp_type(const string& cppType, const char* typeName);

bool apcc_is_void_type(const string& cppType);

bool apcc_is_bool_type(const string& cppType);

bool apcc_is_pair_type(const string& cppType);

bool apcc_is_shared_ptr_type(const string& cppType);

bool apcc_is_out_param_type(const string& cppType, const string& typeName);

string apcc_get_out_param_type(const string& cppType);

string apcc_get_shared_ptr_type(const string& cppType);

string apcc_get_std_shared_ptr_type(const string& type);

string apcc_get_cpp_vector_type(const string& itemType);

string apcc_get_package_name(const string& aidlPackageName);

bool apcc_is_validate_func_with_lock(const string& funcName);

bool apcc_create_file(const string& aidlIntfName, const AidlCallbackName& aidlCallbackName, const ApccFile& apccFile);

const string apcc_get_gen_file_name(const string& aidlIntfName, ApccType apccType);

const string apcc_get_gen_folder(ApccType apccType);

const string apcc_get_hal_file_name(ApccType apccType);

const string apcc_get_hal_folder(ApccType apccType);

const string apcc_get_full_path(ApccType apccType, const string& parentPath);

bool apcc_get_gen_flag(const string& aidlIntfName, ApccType apccType);

bool apcc_get_cpp_pair_type(const string& pairType, string& firstItemType, string& secondItemType);

string apcc_get_std_enable_shared_from_this(const string& className);

string apcc_get_static_proxy_func(const string& className);

string apcc_create_hal_status(const string& halStatusCode, const string& status);
string apcc_create_hal_scoped_astatus(const string& halStatusCode, const string& statusCode);

void apcc_add_comment_hal_msg(const string& halNameLower, const string& comment, stringstream& ss);

void apcc_dump_cpp_info(const vector<CppTypeInfo>& cppTypeInfo);

void apcc_dump_func_type(const FuncType& funcType);
void apcc_dump_func_type(const vector<FuncType>& funcType, const char* info);

void apcc_dump_func_param(const FuncParameter& funcParam, const string& info);

void apcc_dump_func_param(const vector<FuncParameter>& funcParam);

bool apcc_exist_create_hal_status_func();

const string& apcc_get_ndk_lib();

bool apcc_is_multi_aidl_intf();

bool apcc_is_aidl_sync_api();

bool apcc_is_multi_callback();

bool apcc_is_validate_func_supported();

bool apcc_is_remove_file_enabled();

bool apcc_is_cfm_defined();

bool apcc_is_gen_server_enabled();

bool apcc_is_gen_hal_impl_stub_enabled();

bool apcc_is_single_someip_enabled();

bool apcc_is_remove_intf_enabled();

bool apcc_is_test_mode_enabled();

bool apcc_is_hal_impl_enabled();

bool apcc_is_clear_callback_required();

void gen_android_bp(stringstream& ss);

/* 1. folder: client */
void gen_hal_api_header(stringstream& ss);
void gen_hal_rpc_header(stringstream& ss);
void gen_hal_rpc_source(stringstream& ss);

void gen_hal_status_util_header(stringstream& ss);
void gen_hal_status_util_source(stringstream& ss);

/* 2. folder: include */
void gen_hal_message_def_header(stringstream& ss);
void gen_hal_someip_def_header(stringstream& ss);

/* 3. folder: message */
void gen_hal_msg_header(stringstream& ss);
void gen_hal_msg_source(stringstream& ss);

/* 4. folder: server */
void gen_service_source(stringstream& ss);
void gen_hal_service_header(stringstream& ss);
void gen_hal_service_source(stringstream& ss);
void gen_hal_rpc_server_header(stringstream& ss);
void gen_hal_rpc_server_source(stringstream& ss);
void gen_hal_impl_header(stringstream& ss);
void gen_hal_impl_source(stringstream& ss);
