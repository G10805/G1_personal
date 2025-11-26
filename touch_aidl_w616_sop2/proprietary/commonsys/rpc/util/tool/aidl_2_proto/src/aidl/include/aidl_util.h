/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "aidl_def.h"

using std::map;
using std::string;
using std::stringstream;
using std::vector;

#define IS_AIDL_PARAM_IN(dir)       (((dir) & AIDL_PARAM_DIR_MASK) == AIDL_PARAM_IN)
#define IS_AIDL_PARAM_OUT(dir)      (((dir) & AIDL_PARAM_DIR_MASK) == AIDL_PARAM_OUT)

#define AIDL_PARAM_OUT_PREFIX       (AIDL_OUT "_")

typedef struct
{
    string type;
    string name;
    AidlParamDir dir;
    bool nullable;
} AidlParameter;

typedef struct
{
    string returnType;
    string funcName;
    vector<AidlParameter> param;
} AidlInterface;

struct AidlFile
{
    string fileName;
    string packageName;
    string messageName;
    AidlType fileType;
    string typeName;
    vector<string> importFile;
    stringstream ss;
};

struct AidlInterfaceFile: AidlFile
{
    vector<AidlInterface> intf;
    vector<string> enumType;
    vector<string> parcelableType;
};

struct AidlParcelableFile: AidlFile
{
    vector<AidlParameter> param;
    vector<string> enumType;
};

struct AidlEnumFile: AidlFile
{
};

typedef struct
{
    string aidlPath;
    uint32_t totalFileNumber;
    vector<AidlFile*> aidlFile;
    uint32_t aidlIntfNum;
    vector<string> enumType;
    string aidlIntfRoot;
    string halStatusType;
} AidlInfo;

typedef struct
{
    map<string, string> enumFieldName;
    map<string, bool> enumClassName;
} EnumStatsInfo;

typedef struct
{
    string callbackName;
    string callback2Name;
    string callbackResultName;
} AidlCallbackName;

bool aidl_get_file_type(const string& line, AidlType& fileType);

bool aidl_contain_callback_suffix(const string& str);
bool aidl_is_callback(const string& str);

bool aidl_is_qualifier(const string& str);

bool aidl_get_package_name(const string& line, string& packageName);

bool aidl_get_import_file(const string& line, string& fileName);

bool aidl_get_interface(const string& line, AidlInterface& intf);

bool aidl_get_common_field(const string& line, AidlParameter& param);

bool aidl_get_message_name_and_type(const string& aidlLine, string& name);

bool aidl_get_message_name_and_type(const string& aidlLine, string& name, string& type);

bool aidl_is_enum_field(const string& line);

bool aidl_get_enum_field(const string& line, string& str);

bool aidl_get_enum_key(const string& aidlString, string& enumKey);

bool aidl_get_enum_key_val(const string& aidlString, const map<string, int32_t>& enumKeyValMap,
    string& enumKey, int32_t& enumVal, int32_t& enumValNew);

bool aidl_get_enum_val(const string& enumValStr, const map<string, int32_t>& enumKeyValMap, int32_t& enumVal);

string aidl_get_file_name_stem(const string& aidlFileName);

string aidl_get_interface_short_name(const string& interfaceName);

const char *aidl_map_file_type_2_string(AidlType fileType);

void aidl_dump_param(const AidlParameter& param, size_t index = 0);

void aidl_dump_param_list(const vector<AidlParameter>& param);

void aidl_dump_interface(const AidlInterface& intf);

void aidl_get_type_name(const AidlInfo& aidlInfo, AidlType type, vector<string>& aidlTypeName);

const string aidl_get_package_name(const string& aidlIntfName);

bool aidl_is_interface(const string& typeName);

const vector<AidlInterface>& aidl_get_interface_list(const string& aidlIntfName);

const vector<string>& aidl_get_import_list(const string& aidlIntfName);

void aidl_get_enum_list(vector<string>& enumType);

bool aidl_is_at_line(const string& line);

bool aidl_is_array(const string& aidlType);
string aidl_get_array_size(const string& aidlType);
string aidl_get_array_item_type(const string& aidlType);

string aidl_get_include_header(const string& importFile);
string aidl_get_include_header(const string& packageName, const string& typeName);

string aidl_get_using_type(const string& importFile);
string aidl_get_using_type(const string& packageName, const string& typeName);
string aidl_get_using_type(const string& packageName, const string& rootTypeName, const string& internalTypeName);

void aidl_get_hal_namespace(const string& packageName, vector<string>& halNamespace);

AidlFile *aidl_new_file(AidlType fileType);
AidlFile *aidl_new_file(const string& fileName, const string& packageName, const vector<string>& importFile,
                        const string& messageName, AidlType fileType);

const AidlFile *aidl_get_file(uint32_t aidlFileIndex);

const vector<string>& aidl_get_enum_type();

bool aidl_is_enum_type(const string& typeName);

bool aidl_is_intf_root(const string& aidlIntfName);

const string& aidl_get_intf_root();

const string& aidl_get_hal_status_type();

bool aidl_get_callback_name(const string& aidlIntfName, AidlCallbackName& aidlCallbackName);

string aidl_get_out_param_name(const string& paramName);