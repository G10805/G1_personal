/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <string>
#include <vector>

#include "aidl_util.h"
#include "proto_def.h"

using std::string;
using std::vector;

typedef uint32_t    ConvertResult;

#define CONVERT_RESULT_SUCCESS          ((ConvertResult) 0x0)
#define CONVERT_RESULT_FAIL             ((ConvertResult) 0x1)
#define CONVERT_RESULT_NOT_HANDLE       ((ConvertResult) 0x2)
#define CONVERT_RESULT_ALREADY_PARSED   ((ConvertResult) 0x3)

#define IS_CONVERT_SUCCESS(res)         ((res) == CONVERT_RESULT_SUCCESS)
#define IS_CONVERT_FAIL(res)            ((res) == CONVERT_RESULT_FAIL)
#define IS_CONVERT_NOT_HANDLE(res)      ((res) == CONVERT_RESULT_NOT_HANDLE)

#ifdef _WIN32
#define PROTO_RELATIVE_PATH             "\\proto"
#else
#define PROTO_RELATIVE_PATH             "/proto"
#endif

#define PROTO_HEADER_SUFFIX             ".pb.h"

#define MAX_NESTED_DEPTH                (10)

typedef struct
{
    string type;
    string name;
    int32_t value;
} ProtoField;

typedef struct
{
    ProtoNodeType type;
    string name;
} ProtoNodeHeader;

typedef struct
{
    ProtoNodeType type;
    string name;
    vector<ProtoField> field;
} ProtoNode;

typedef struct
{
    string fileName;
    string packageName;
    ProtoType fileType;
    vector<string> importFile;
    vector<vector<ProtoNode>> protoNode;
} ProtoFile;

typedef struct
{
    string protoPath;
    uint32_t totalFileNumber;
    vector<ProtoFile> protoFile;
    bool genHalStatus;
} ProtoInfo;


void proto_util_init(const string& protoPath);

void proto_util_deinit();

string proto_get_file_name(const string& aidlFile);

string proto_get_package_name(const string& aidlPackage);

bool proto_get_package_name(const string& protoFileName, string& packageName);

bool proto_get_namespace(const string& protoFileName, string& protoNamespace);

ProtoType proto_get_file_type(AidlType aidlFileType);

bool proto_create_file(const string& aidlPath, uint32_t aidlFileIndex, const AidlFile *aidlFile,
    const string& protoPath, const ProtoFile& protoFile, bool removeFile = false);

const char *proto_map_file_type_2_string(ProtoType fileType);

void proto_read_file(const string& protoPath, ProtoFile& protoFile);

bool proto_get_message_req(const string& aidlIntfName, vector<string>& msg);

bool proto_get_message_ind(const string& aidlIntfName, vector<string>& msg);

bool proto_get_message_cfm(const string& aidlIntfName, vector<string>& msg, bool excludeLastCommonCfm = true);

bool proto_get_common_message(const string& aidlIntfName, vector<string>& msg);

bool proto_get_enum_type(const string& aidlIntfName, vector<string>& enumType);

bool proto_get_enum_val(const string& enumType, vector<string>& enumVal);

string proto_get_intf_callback_name(const string& aidlIntfName);

string proto_get_header_file_name(const string& aidlIntfName);

string proto_get_hal_name_lower(const string& aidlPackageName);

bool proto_create_hal_status_file(const string& protoPath, bool removeFile = false);

bool proto_create_android_bp(const string& protoPath, bool removeFile = false);

bool proto_create_linux_makefile(const string& protoPath, bool removeFile = false);

bool proto_create_makefile(const string& protoPath, bool removeFile, const string& name, const string& prototype);