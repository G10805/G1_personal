/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#ifndef EXCLUDE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <fstream>
#include <sstream>

#include "log.h"
#include "proto.h"
#include "proto_util.h"
#include "util.h"
#include "aidl.h"

using std::endl;
using std::fstream;
using std::ifstream;
using std::istringstream;
using std::map;
using std::ofstream;
using std::pair;
using std::stringstream;

typedef struct
{
    map<int, bool> enumValueRepeated;
    bool reserveEnumRequired;
    stringstream enumSS;
    map<string, int32_t> enumKeyVal;
} EnumInstance;

typedef struct
{
    string protoPath;
    map<string, string> aidlTypeMap;
    uint32_t aidlFileIndex;
    string messageName[MAX_NESTED_DEPTH];
    ProtoType messageType[MAX_NESTED_DEPTH];
    int nestDepth;
    stringstream pss;
    int fieldIndex[MAX_NESTED_DEPTH];
    EnumInstance enumInst;
} ProtoFileInstance;

static ProtoFileInstance sProtoFileInstance;

/* ------------------------------------------------------ */

static void clear_proto_string_stream()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    inst->pss.str("");
    inst->pss.clear();
}

static void clear_proto_file_instance()
{
    ProtoFileInstance *inst = &sProtoFileInstance;

    for(int i = 0; i < MAX_NESTED_DEPTH; i++)
    {
        inst->messageName[i].clear();
    }

    memset(inst->messageType, 0, sizeof(inst->messageType));
    inst->nestDepth = -1;
    clear_proto_string_stream();
    memset(inst->fieldIndex, 0, sizeof(inst->fieldIndex));
}

static void increase_nest_depth()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    inst->nestDepth ++;
    logd("%s nestDepth %d", __func__, inst->nestDepth);
}

static void decrease_nest_depth()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    if (inst->nestDepth >= 0)
    {
        inst->nestDepth --;
    }
    logd("%s nestDepth %d", __func__, inst->nestDepth);
}

static int get_nest_depth()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    return inst->nestDepth;
}

static bool is_nest_depth_valid(int nestDepth)
{
    return (nestDepth >=0 && nestDepth <= MAX_NESTED_DEPTH);
}

static int get_field_index()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();
    if (is_nest_depth_valid(nestDepth))
    {
        return inst->fieldIndex[nestDepth];
    }
    return 0;
}

static int get_parent_field_index()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth() - 1;
    if (is_nest_depth_valid(nestDepth))
    {
        return inst->fieldIndex[nestDepth];
    }
    return 0;
}

static void set_field_index(int newIndex)
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();
    if (is_nest_depth_valid(nestDepth))
    {
        inst->fieldIndex[nestDepth] = newIndex;
    }
}

static void increase_field_index()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();
    if (is_nest_depth_valid(nestDepth))
    {
        ++ inst->fieldIndex[nestDepth];
        logd("%s nestDepth %d field index %d", __func__, nestDepth, inst->fieldIndex[nestDepth]);
    }
}

static void clear_field_index()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();
    if (is_nest_depth_valid(nestDepth))
    {
        inst->fieldIndex[nestDepth] = 0;
    }
}

static ProtoType get_proto_file_type()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();
    return nestDepth >= 0 ?
            inst->messageType[nestDepth]:
            PROTO_TYPE_UNKNOW;
}

static stringstream& get_proto_string_stream()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    return inst->pss;
}

static void add_proto_string(const char *str)
{
    stringstream& pss = get_proto_string_stream();
    pss << str << endl;
}

static bool is_common_message(const string& msgName)
{
    return !(contain_suffix(msgName, PROTO_MESSAGE_REQ) ||
            contain_suffix(msgName, PROTO_MESSAGE_IND) ||
            contain_suffix(msgName, PROTO_MESSAGE_CFM));
}

static bool is_aidl_array(const string& aidlType)
{
    /* e.g. "int[]" */
    return contain_char(aidlType, '[') && contain_char(aidlType, ']');
}

static string get_aidl_array_item_type(const string& aidlType)
{
    /* e.g. "int[]" -> "int" */
    return aidlType.substr(0, aidlType.find('['));
}

static string get_proto_type(const string& aidlType)
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    map<string, string>::iterator it = inst->aidlTypeMap.find(aidlType);
    return (it != inst->aidlTypeMap.end()) ? it->second : PROTO_TYPE_GENERIC;
}

static string get_proto_type(const string& keyword, const string& itemType)
{
    stringstream ss;
    ss << keyword << ' ' << itemType;
    return ss.str();
}

static const string& get_aidl_file_name()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    const static string sEmptyAidlFileName = "";
    const AidlFile *aidlFile = aidl_get_file(inst->aidlFileIndex);
    return aidlFile != NULL ? aidlFile->fileName : sEmptyAidlFileName;
}

/*
 * true, e.g. wifi.ict.IfaceConcurrencyType
 * false, e.g. AssociationRejectionData
 */
bool is_sub_package_enabled(const string& enumClassName)
{
    EnumStatsInfo* enumStats = aidl_get_enum_stats();
    map<string, bool>::iterator iter;
    iter = enumStats->enumClassName.find(enumClassName);
    if (iter != enumStats->enumClassName.end())
    {
        logd("%s package prefix is required for type %s", __func__, enumClassName.c_str());
        return true;
    }
    return false;
}

static string get_proto_type_from_aidl_type(const string& aidlType)
{
    string protoType = aidlType;
    if (!is_sub_package_enabled(aidlType))
    {
        return protoType;
    }

    string packageName = aidl_get_package_from_message_name(aidlType);
    packageName = proto_get_package_name(packageName);
    protoType = packageName + '.' + protoType;
    return protoType;
}


static string map_2_proto_type(const string& aidlType)
{
    stringstream ss;
    string protoType = get_proto_type(aidlType);
    if (protoType == PROTO_TYPE_GENERIC)
    {
        if (is_aidl_array(aidlType))
        {
            string aidlItemType = get_aidl_array_item_type(aidlType);
            string protoItemType = get_proto_type(aidlItemType);
            logd("%s: proto item type: %s, aidl item type: %s", __func__, protoItemType.c_str(), aidlItemType.c_str());

            if (protoItemType == PROTO_TYPE_GENERIC)
            {
                /* e.g.
                 * "ItemType[]" -> "repeated ItemType"
                 * "ItemType2[]" -> "repeated package.ItemType2"
                 */
                aidlItemType = get_proto_type_from_aidl_type(aidlItemType);
                ss << PROTO_TYPE_ARRAY << SPACE << aidlItemType;
                return ss.str();
            }
            else
            {
                /* byte[] shall be converted to bytes, instead of repeated uint32 */
                if (aidlItemType == AIDL_TYPE_BYTE)
                {
                    ss << PROTO_TYPE_BYTES;
                    return ss.str();
                }
                else
                {
                    /* e.g. "int[]" -> "repeated int32" */
                    ss << PROTO_TYPE_ARRAY << SPACE << protoItemType;
                    return ss.str();
                }
            }
        }
        else
        {
            /* e.g. WifiBand -> wifi.wb.WifiBand */
            return get_proto_type_from_aidl_type(aidlType);
        }
    }
    else
    {
        /* e.g. "int" -> "int32" */
        return protoType;
    }
}

static const vector<string>& get_aidl_import_file()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    static const vector<string> sEmptyImportFile;
    const AidlFile *aidlFile = aidl_get_file(inst->aidlFileIndex);
    return aidlFile != NULL ? aidlFile->importFile : sEmptyImportFile;
}

static bool set_message_type(const string& fileType)
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();

    if (fileType == AIDL_PARCELABLE)
    {
        inst->messageType[nestDepth] = PROTO_TYPE_COMMON;
        return true;
    }
    else if (fileType == AIDL_UNION)
    {
        inst->messageType[nestDepth] = PROTO_TYPE_ONEOF;
        return true;
    }
    else if (fileType == AIDL_ENUM)
    {
        inst->messageType[nestDepth] = PROTO_TYPE_ENUM;
        return true;
    }
    else
    {
        /* Ignore type AIDL_INTERFACE(interface & callback) because
         * 1. AIDL_INTERFACE can be in root node only
         * 2. Proto type for root node is already retrieved in AIDL file scan
         */
    }

    return false;
}

static ProtoType get_message_type()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();

    if (is_nest_depth_valid(nestDepth))
    {
        return inst->messageType[nestDepth];
    }
    return PROTO_TYPE_UNKNOW;
}

static ProtoType get_parent_message_type()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth() - 1;

    if (is_nest_depth_valid(nestDepth))
    {
        return inst->messageType[nestDepth];
    }
    return PROTO_TYPE_UNKNOW;
}

static void set_message_name(const string& messageName)
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();
    if (is_nest_depth_valid(nestDepth))
    {
        inst->messageName[nestDepth] = messageName;
    }
}

static string get_message_name()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    int nestDepth = get_nest_depth();
    if (is_nest_depth_valid(nestDepth))
    {
        return inst->messageName[nestDepth];
    }
    return "";
}

static string get_oneof_name()
{
    stringstream ss;
    ss << get_message_name() << PROTO_ONEOF_SUFFIX;
    return ss.str();
}

static void create_proto_syntax()
{
    stringstream& pss = get_proto_string_stream();
    pss << PROTO_DEFAULT_SYNTAX << endl;
    pss << endl;
}

static void create_proto_package(const string& protoPackageName)
{
    stringstream& pss = get_proto_string_stream();
    /* e.g. "package hal;" */
    pss << PROTO_PACKAGE << SPACE << protoPackageName << LINE_SUFFIX << endl;
    pss << endl;
}

static void create_proto_import(const string& protoImportFile)
{
    stringstream& pss = get_proto_string_stream();
    pss << PROTO_IMPORT << SPACE << "\"" << protoImportFile << PROTO_FILE_SUFFIX << "\"" << LINE_SUFFIX << endl;
}

static void create_proto_import(const string& aidlImport, const string& aidlPackage)
{
    string aidlFileNameStem = aidl_get_file_name_stem(get_aidl_file_name());
    /* e.g. aidlImport: "android.hardware.hal.IHal", aidlPackage: "android.hardware.hal" => "IHal" */
    string protoImportFile = get_sub_string(aidlImport, aidlPackage, '.');

    /* avoid import recursion */
    if (!protoImportFile.compare(aidlFileNameStem))
    {
        logw("import recursion for %s, return", protoImportFile.c_str());
        return;
    }

    /* e.g. "import "IHal.proto";" */
    create_proto_import(protoImportFile);
}

static void create_proto_import_files(AidlType fileType, const string& aidlPackageName)
{
    const ProtoInfo& protoInfo = proto_get_info();
    const vector<string>& aidlImportFile = get_aidl_import_file();
    size_t totalFileNum = aidlImportFile.size();

    logd("%s: aidl import file number %d", __func__, totalFileNum);
    if (totalFileNum > 0)
    {
        for (size_t index = 0; index < totalFileNum; index++)
        {
            create_proto_import(aidlImportFile[index], aidlPackageName);
        }
    }

    if (protoInfo.genHalStatus &&
        (fileType == AIDL_TYPE_INTERFACE))
    {
        create_proto_import(HAL_STATUS_NAME);
    }

    if (totalFileNum > 0)
    {
        /* add 1 empty line */
        add_proto_string("");
    }
}

static void create_proto_option()
{
    add_proto_string(PROTO_DEFAULT_OPTION);
}

static void init_proto_header(const AidlFile *aidlFile)
{
    /* 1. syntax */
    logd("%s: create proto syntax", __func__);
    create_proto_syntax();

    /* 2. package */
    string protoPackageName = aidlFile->packageName;
    protoPackageName = proto_get_package_name(protoPackageName);
    logd("%s: create proto package, package name: %s", __func__, protoPackageName.c_str());
    create_proto_package(protoPackageName);

    /* 3. import */
    logd("%s: create proto import", __func__);
    create_proto_import_files(aidlFile->fileType, aidlFile->packageName);

    /* 4. option */
    logd("%s: create proto option", __func__);
    create_proto_option();
}

static const string get_proto_message_prefix(ProtoType protoFileType, const string& funcName)
{
    if ((protoFileType == PROTO_TYPE_CFM) &&
        funcName.empty())
    {
        /* "IHal.aidl" -> "IHal" -> "Hal" */
        return aidl_get_interface_short_name(aidl_get_file_name_stem(get_aidl_file_name()));
    }
    return "";
}

static const string get_proto_message_suffix(ProtoType protoFileType)
{
    switch (protoFileType)
    {
        case PROTO_TYPE_REQ:
            return PROTO_MESSAGE_REQ;
        case PROTO_TYPE_IND:
            return PROTO_MESSAGE_IND;
        case PROTO_TYPE_CFM:
            return PROTO_MESSAGE_CFM;
        case PROTO_TYPE_COMMON:
        case PROTO_TYPE_ONEOF:
            /* fall-through */
        case PROTO_TYPE_ENUM:
        default:
            return "";
    }
}

static string get_indent_spaces(const string& spaces)
{
    string s = spaces;
    int nestDepth = get_nest_depth();
    for (int i = 0; i < nestDepth; i ++)
    {
        s += SPACES;
    }
    return s;
}

static string create_message_name(ProtoType protoFileType, const string& funcName)
{
    string str = funcName;

    /* e.g. "foo" -> "Foo" */
    convert_first_char_to_upper(str);

    /* e.g. "Hal" + "Foo" -> "FooReq" | "HalFooCfm" | "FooInd" */
    return get_proto_message_prefix(protoFileType, funcName) + str + get_proto_message_suffix(protoFileType);
}

static bool is_param_type_interface(const string& paramType)
{
    const AidlInfo& aidlInfo = aidl_get_info();
    size_t totalFileNum = aidlInfo.aidlFile.size();
    for (size_t index = 0; index < totalFileNum; index++)
    {
        if ((aidlInfo.aidlFile[index]->messageName == paramType) &&
            aidlInfo.aidlFile[index]->fileType == AIDL_TYPE_INTERFACE)
        {
            return true;
        }
    }
    return false;
}

static void create_message_field(const AidlParameter& param, uint32_t fieldIndex, ProtoType type)
{
    stringstream& pss = get_proto_string_stream();
    string spaces = get_indent_spaces(SPACES);

    /* non PROTO_TYPE_COMMON requires additional SPACES */
    if (type != PROTO_TYPE_COMMON)
    {
        spaces += SPACES;
    }

    string protoType = map_2_proto_type(param.type);
    logd("%s: proto type: %s, aidl type: %s", __func__, protoType.c_str(), param.type.c_str());

    if (((type == PROTO_TYPE_REQ || type == PROTO_TYPE_CFM) &&
        is_param_type_interface(param.type)) ||
        aidl_is_callback(param.type) ||
        param.nullable)
    {
        /* e.g.
         * PROTO_TYPE_REQ: "optional IWifiStaIface boundIface = 1;"
         * PROTO_TYPE_CFM: "optional IWifiEventCallback callback = 1;"
         */
        pss << spaces << PROTO_OPTIONAL << SPACE << protoType << SPACE << param.name <<
            " = " << fieldIndex << LINE_SUFFIX << endl;
    }
    else
    {
        /* e.g. "int32 param = 1;" */
        pss << spaces << protoType << SPACE << param.name << " = " << fieldIndex << LINE_SUFFIX << endl;
    }
}

static void create_common_field(const AidlParameter& param, ProtoType protoFileType)
{
    int index = get_field_index();
    create_message_field(param, index + 1, protoFileType);
    increase_field_index();
}

static bool is_message_body_blank_line_needed()
{
    /* don't add blank line for 1st message */
    if (get_field_index() != 0)
    {
        return true;
    }

    return false;
}

static void create_message(const string& messageName, const vector<AidlParameter>& paramList, ProtoType type)
{
    stringstream& pss = get_proto_string_stream();
    vector<AidlParameter>::size_type size = paramList.size();
    string spaces = get_indent_spaces(SPACES);
    logd("%s: message name: %s, param size %d", __func__, messageName.c_str(), size);

    /* 1. message header: e.g. "message Hal" */
    if(is_message_body_blank_line_needed())
    {
        pss << endl;
    }
    pss << spaces << PROTO_MESSAGE << SPACE << messageName << endl;
    pss << spaces << "{" << endl;

    /* 2. message field */
    if (size > 0)
    {
        uint32_t fieldIndex = 1;
        for (uint32_t index = 0; index < size; index++)
        {
            const AidlParameter& param = paramList[index];
            if ((type == PROTO_TYPE_REQ) && !IS_AIDL_PARAM_IN(param.dir))
            {
                /* not add aidl param out in req message */
                logd("%s: ignore adding aidl param out (%s %s) in req", __func__,
                    param.type.c_str(), param.name.c_str());
                continue;
            }
            create_message_field(param, fieldIndex, type);
            ++ fieldIndex;
        }
    }

    pss << spaces << "}" << endl;
    logd("%s: done for message name: %s", __func__, messageName.c_str());
}

static void create_interface_message(ProtoType protoFileType, const string& funcName, const vector<AidlParameter>& paramList)
{
    string messageName = create_message_name(protoFileType, funcName);
    logd("%s: message name: %s, proto file type: %d, func name: %s", __func__,
        messageName.c_str(), protoFileType, funcName.c_str());
    create_message(messageName, paramList, protoFileType);
}

static const string get_default_cfm_name()
{
    const ProtoInfo& protoInfo = proto_get_info();
    /* e.g. "HalStatus" */
    return protoInfo.genHalStatus ?
            HAL_STATUS_NAME :
            create_message_name(PROTO_TYPE_CFM, "");
}

static void init_aidl_ndk_scoped_astatus_param(vector<AidlParameter>& paramList)
{
    paramList.resize(2);

    /* 1. int32 status */
    paramList[0].type = "int32";
    paramList[0].name = "status";

    /* 2. string info */
    paramList[1].type = "string";
    paramList[1].name = "info";
}

static void create_default_proto_cfm_message()
{
    const ProtoInfo& protoInfo = proto_get_info();
    vector<AidlParameter> paramList;
    const string defaultCfmName = get_default_cfm_name();

    if (protoInfo.genHalStatus)
    {
        /* Not create default cfm message if HalStatus.proto is used */
        return;
    }

    logd("%s: default cfm name: %s", __func__, defaultCfmName.c_str());

    init_aidl_ndk_scoped_astatus_param(paramList);
    aidl_dump_param_list(paramList);

    /*
     * e.g.
     *   message HalCfm
     *   {
     *       int32 status = 1;
     *       string info = 2;
     *   }
     **/
    create_message(defaultCfmName, paramList, PROTO_TYPE_CFM);
}

static void init_cfm_param(const string& defaultCfmName, const AidlInterface& intf,
    vector<AidlParameter>& paramList)
{
    const string& returnType = intf.returnType;

    paramList.clear();

    /* 'defaultCfmName = ""' is for NON standard callback with non
     * void return type.
     * this kind of callback is treated as request, and therefore
     * confirmation in peer side is required.
     * meanwhile, confimration goes from android to non-android.
     * hence default confirmation is not required. because default
     * confirmation only makes sense in android aidl.
     */
    if (is_empty_string(defaultCfmName))
    {
        paramList.push_back({returnType, "result", AIDL_PARAM_IN, false});
    }
    else if (is_same_string(returnType, AIDL_TYPE_VOID))
    {
        /* 1. HalStatus status */
        paramList.push_back({defaultCfmName, "status", AIDL_PARAM_IN, false});
    }
    else
    {
        /* 1. HalStatus status */
        paramList.push_back({defaultCfmName, "status", AIDL_PARAM_IN, false});

        /* 2. returnType (e.g. bool) result */
        paramList.push_back({returnType, "result", AIDL_PARAM_IN, false});
    }

    /* add aidl param out in cfm message */
    for (auto it: intf.param)
    {
        if (IS_AIDL_PARAM_OUT(it.dir))
            paramList.push_back({it.type, aidl_get_out_param_name(it.name), AIDL_PARAM_IN, false});
    }
}

static void create_proto_cfm_message(const string& defaultCfmName, const AidlInterface& intf)
{
    vector<AidlParameter> paramList;
    string messageName = create_message_name(PROTO_TYPE_CFM, intf.funcName);

    logd("%s: default cfm name: %s, func name: %s, return type: %s", __func__,
        defaultCfmName.c_str(), intf.funcName.c_str(), intf.returnType.c_str());

    init_cfm_param(defaultCfmName, intf, paramList);
    aidl_dump_param_list(paramList);

    convert_first_char_to_upper(messageName);

    /*
     * e.g.
     *   message HalFuncCfm
     *   {
     *       HalStatus status = 1;
     *       bool result = 2;
     *   }
     **/
    create_message(messageName, paramList, PROTO_TYPE_CFM);
}

static void create_message_req(const AidlInterface& intf, bool isDefaultCfmRequired)
{
    /* 1. create req for "funcName + param" */
    create_interface_message(PROTO_TYPE_REQ, intf.funcName, intf.param);

    increase_field_index();

    /* 2. create cfm for "returnType" */
    string defaultCfmName = isDefaultCfmRequired?
                            get_default_cfm_name():
                            "";
    create_proto_cfm_message(defaultCfmName, intf);

    increase_field_index();
}

static void create_message_req(const AidlInterface& intf)
{
    create_message_req(intf, true);
}

static void create_message_ind(const AidlInterface& intf)
{
    ProtoType type = intf.returnType == AIDL_TYPE_VOID ?
                     PROTO_TYPE_IND:
                     PROTO_TYPE_REQ;

    /* 1. create ind for "funcName + param" */
    create_interface_message(type, intf.funcName, intf.param);

    increase_field_index();
}

static string get_enum_prefix()
{
    string messageName = get_string_upper_letters(get_message_name());
    return messageName;
}

static string get_enum_alias()
{
    string spaces = get_indent_spaces(SPACES);
    return spaces + PROTO_ENUM_ALLOW_ALIAS;
}

static string get_enum_reserved()
{
    string spaces = get_indent_spaces(SPACES);
    return spaces + get_enum_prefix() + PROTO_ENUM_RESERVED_SUFFIX;
}

static EnumInstance *get_enum_instance()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    return &(inst->enumInst);
}

static void clear_enum_instance()
{
    EnumInstance *enumInst = get_enum_instance();
    enumInst->enumValueRepeated.clear();
    enumInst->reserveEnumRequired = false;
    enumInst->enumSS.str("");
    enumInst->enumSS.clear();
    enumInst->enumKeyVal.clear();
}

static void update_enum_instance(const string& enumKey, int oldEnumValue, int newEnumValue, const string& str)
{
    EnumInstance *enumInst = get_enum_instance();

    enumInst->enumKeyVal.insert(std::make_pair(enumKey, newEnumValue));

    /* check if reserved enum is required */
    if (oldEnumValue == 0 && newEnumValue != 0)
    {
        enumInst->reserveEnumRequired = true;
    }

    /*Reserved enum(0) is required, then enum is repeated if new enum(0) is find */
    if (enumInst->reserveEnumRequired && newEnumValue == 0)
    {
        enumInst->enumValueRepeated[newEnumValue] = true;
    }
    else
    {
        /* check if allow alias is required*/
        auto iter = enumInst->enumValueRepeated.find(newEnumValue);
        if (iter != enumInst->enumValueRepeated.end())
        {
            enumInst->enumValueRepeated[newEnumValue] = true;
        }
        else
        {
            enumInst->enumValueRepeated.insert(std::make_pair(newEnumValue, false));
        }
    }

    enumInst->enumSS << str << endl;
}

static bool get_enum_is_repeated()
{
    EnumInstance *enumInst = get_enum_instance();
    for(auto it = enumInst->enumValueRepeated.begin(); it != enumInst->enumValueRepeated.end(); ++it)
    {
        if(it->second)
        {
            return true;
        }
    }
    return false;
}

static const map<string, int32_t>& get_enum_key_val_map()
{
    EnumInstance *enumInst = get_enum_instance();
    return enumInst->enumKeyVal;
}

static void add_enum_strings()
{
    EnumInstance *enumInst = get_enum_instance();
    bool isEnumRepeated = get_enum_is_repeated();
    bool isReservedEnumRepeated = enumInst->reserveEnumRequired;
    string line;

    while(std::getline(enumInst->enumSS, line))
    {
        if (line.find(PROTO_ENUM_ALLOW_ALIAS_PLACE_HOLDER) != string::npos)
        {
            if (isEnumRepeated)
            {
                add_proto_string((get_enum_alias()).c_str());
            }
        }
        else if (line.find(PROTO_ENUM_RESERVED_PLACE_HOLDER) != string::npos)
        {
            if (isReservedEnumRepeated)
            {
                add_proto_string((get_enum_reserved()).c_str());
            }
        }
        else
        {
            add_proto_string(line.c_str());
        }
    }
}

static void create_enum_field(const string& aidlString)
{
    stringstream ss;
    string enumKey;
    int enumVal = get_field_index();
    int enumValNew = enumVal;
    string enumSpaces = get_indent_spaces(SPACES);

    /* calculate enum string */
    aidl_get_enum_key_val(aidlString, get_enum_key_val_map(), enumKey, enumVal, enumValNew);

    ss << enumSpaces << enumKey << " = " << enumValNew << LINE_SUFFIX;

    update_enum_instance(enumKey, enumVal, enumValNew, ss.str());

    set_field_index(enumValNew);
    increase_field_index();
}

static ConvertResult create_proto_message(const string& aidlLine, ProtoType protoFileType)
{
    ConvertResult res = CONVERT_RESULT_NOT_HANDLE;
    AidlInterface intf;
    AidlParameter param;
    string str;

    logd("%s: %s", __func__, proto_map_file_type_2_string(protoFileType));
    switch (protoFileType)
    {
        case PROTO_TYPE_COMMON:
            /* fall-through */
        case PROTO_TYPE_ONEOF:
        {
            if (aidl_get_common_field(aidlLine, param))
            {
                aidl_dump_param(param);
                create_common_field(param, protoFileType);
                return CONVERT_RESULT_SUCCESS;
            }
            break;
        }
        case PROTO_TYPE_REQ:
        {
            if (aidl_get_interface(aidlLine, intf))
            {
                logd("%s: create message req for aidl interface", __func__);
                aidl_dump_interface(intf);
                create_message_req(intf);
                increase_field_index();
                return CONVERT_RESULT_SUCCESS;
            }
            break;
        }
        case PROTO_TYPE_IND:
        {
            if (aidl_get_interface(aidlLine, intf))
            {
                logd("%s: create message ind for aidl interface", __func__);
                aidl_dump_interface(intf);
                if (intf.returnType == AIDL_TYPE_VOID)
                {
                    create_message_ind(intf);
                }
                else
                {
                    create_message_req(intf, false);
                }
                increase_field_index();
                return CONVERT_RESULT_SUCCESS;
            }
            break;
        }
        case PROTO_TYPE_ENUM:
        {
            if (aidl_is_enum_field(aidlLine))
            {
                logd("%s: create enum field", __func__);
                create_enum_field(aidlLine);
                return CONVERT_RESULT_SUCCESS;
            }
            break;
        }
        case PROTO_TYPE_CFM:
        {
            /* post handling */
            break;
        }
        default:
        {
            loge("%s: unknown proto file type %d, %d(%s)", __func__, protoFileType, errno, strerror(errno));
            break;
        }
    }
    return res;
}

static bool is_message_header_blank_line_needed()
{
    /* always add blank line for the root node */
    if (get_nest_depth() == 0 && get_field_index() == 0)
    {
        return true;
    }

    ProtoType protoType = get_parent_message_type();
    if (get_parent_field_index() != 0 &&
        (protoType != PROTO_TYPE_COMMON) &&
        (protoType != PROTO_TYPE_ONEOF))
    {
        return true;
    }

    return false;
}

static void create_proto_common(const string& type)
{
    stringstream& pss = get_proto_string_stream();
    string messageName = get_message_name();
    string spaces = get_indent_spaces("");
    /* e.g. "message WifiChannelInfo" */
    logd("%s: message name: %s", __func__, messageName.c_str());

    /* only add blank line once */
    if (is_message_header_blank_line_needed())
    {
        pss << endl;
    }
    pss << spaces << type << SPACE << messageName << endl;
    pss << spaces << "{" << endl;
}

static void create_proto_parcelable()
{
    create_proto_common(PROTO_MESSAGE);
}

static void create_proto_oneof()
{
    create_proto_common(PROTO_MESSAGE);

    /* add embedded "oneof" type */
    stringstream& pss = get_proto_string_stream();
    string oneofName = get_oneof_name();
    string spaces = get_indent_spaces("") + SPACES;

    pss << spaces << PROTO_ONEOF << SPACE << oneofName << endl;
    pss << spaces << "{" << endl;
}

static void create_proto_enum()
{
    EnumInstance *enumInst = get_enum_instance();
    string enumName = get_message_name();
    string spaces = get_indent_spaces("");
    /* e.g. "enum HalEnum" */
    logd("%s: enum name: %s", __func__, enumName.c_str());

    /* only add blank line once */
    if (is_message_header_blank_line_needed())
    {
        enumInst->enumSS << endl;
    }
    enumInst->enumSS << spaces << PROTO_ENUM << SPACE << enumName << endl;
    enumInst->enumSS << spaces << "{" << endl;
    enumInst->enumSS << PROTO_ENUM_ALLOW_ALIAS_PLACE_HOLDER << endl;
    enumInst->enumSS << PROTO_ENUM_RESERVED_PLACE_HOLDER << endl;
}

static void enum_string_post_processing()
{
    add_enum_strings();
    clear_enum_instance();
}

static void add_module_terminator(const string& spaces = "")
{
    string s = get_indent_spaces(spaces);
    add_proto_string((s + "}").c_str());
}

static void handle_aidl_module_terminator_util()
{
    add_module_terminator();
    clear_field_index();
    decrease_nest_depth();
}

static void add_instance_id()
{
    stringstream& pss = get_proto_string_stream();
    string spaces = get_indent_spaces(SPACES);
    string instanceID = "int32 instanceId = 1;";
    pss << endl;
    pss << spaces << instanceID << endl;
}

static void handle_aidl_module_terminator(ProtoType protoFileType)
{
    switch (protoFileType)
    {
        case PROTO_TYPE_REQ:
        {
            /* add default cfm message */
            create_default_proto_cfm_message();
            add_instance_id();
            handle_aidl_module_terminator_util();
            break;
        }
        case PROTO_TYPE_COMMON:
        {
            handle_aidl_module_terminator_util();
            increase_field_index();
            break;
        }
        case PROTO_TYPE_ONEOF:
        {
            /* add embedeed "oneof" terminator */
            add_module_terminator(SPACES);
            handle_aidl_module_terminator_util();
            increase_field_index();
            break;
        }
        case PROTO_TYPE_ENUM:
        {
            /* append enum strings into pss */
            enum_string_post_processing();
            handle_aidl_module_terminator_util();

            /*
             * don't increase index if enum is nested in parceable message
             * because here enum is definition, not message.
             * index shall be added for other types because index is input for
             * checking whether or not add blank line prior to message creation
             * refer to functions:
             *    is_message_body_blank_line_needed()
             *    is_message_header_blank_line_needed()
             */
            ProtoType protoType = get_message_type();
            if ((protoType != PROTO_TYPE_COMMON) &&
                (protoType != PROTO_TYPE_ONEOF))
            {
                increase_field_index();
            }
            break;
        }
        case PROTO_TYPE_IND:
        {
            handle_aidl_module_terminator_util();
            break;
        }
        case PROTO_TYPE_CFM:
            /* fall-through */
        default:
        {
            /* not handle */
            break;
        }
    }
}

void update_message_name_and_type(const string& aidlLine)
{
    vector<string> result;
    string name, type;
    if (aidl_get_message_name_and_type(aidlLine, name, type))
    {
        set_message_type(type);
        set_message_name(name);
    }
}

static void create_proto_interface()
{
    stringstream& pss = get_proto_string_stream();
    string aidlFileName = get_aidl_file_name();
    string messageName = get_file_name_stem(aidlFileName, AIDL_FILE_SUFFIX);;
    string spaces = get_indent_spaces("");

    logd("%s: message name: %s", __func__, messageName.c_str());

    if (is_message_header_blank_line_needed())
    {
        pss << endl;
    }
    pss << spaces << PROTO_MESSAGE << SPACE << messageName << endl;
    pss << spaces << "{" << endl;
}

static ConvertResult convert_aidl_2_proto_line(const string& aidlLine, const string& aidlPackage)
{
    AidlInterface intf;
    string str;

    ProtoType protoFileType = get_proto_file_type();
    logd("%s aidlLine %s", __func__, aidlLine.c_str());

    if (aidl_get_package_name(aidlLine, str))
    {
        /* ignore in current scan, since package name is retrieved. */
        logd("%s: already parsed package name: %s", __func__, str.c_str());
        return CONVERT_RESULT_ALREADY_PARSED;
    }
    else if (aidl_get_import_file(aidlLine, str))
    {
        /* ignore in current scan, since import file is retrieved in previous scan. */
        logd("%s: already parsed import file: %s", __func__, str.c_str());
        return CONVERT_RESULT_ALREADY_PARSED;
    }
    else if (aidl_is_at_line(aidlLine))
    {
        return CONVERT_RESULT_SUCCESS;
    }
    else if (contain_string(aidlLine, AIDL_INTERFACE) ||
        contain_string(aidlLine, AIDL_PARCELABLE) ||
        contain_string(aidlLine, AIDL_UNION) ||
        contain_string(aidlLine, AIDL_ENUM))
    {
        increase_nest_depth();

        /* update proto file type for root node */
        if (get_nest_depth() == 0)
        {
            protoFileType = get_message_type();
        }

        update_message_name_and_type(aidlLine);

        protoFileType = get_message_type();
        logd("%s protoFileType %d", __func__, protoFileType);

        if (protoFileType == PROTO_TYPE_COMMON)
        {
            create_proto_parcelable();
        }
        else if (protoFileType == PROTO_TYPE_ONEOF)
        {
            create_proto_oneof();
        }
        else if (protoFileType == PROTO_TYPE_ENUM)
        {
            create_proto_enum();
        }
        else if (protoFileType == PROTO_TYPE_REQ)
        {
            create_proto_interface();
        }
        else if (protoFileType == PROTO_TYPE_IND)
        {
            create_proto_interface();
        }

        return CONVERT_RESULT_SUCCESS;
    }
    else if (contain_string(aidlLine, "{"))
    {
        /* ignore, not handle */
        logd("%s: aidl module begin with '{', not handle", __func__);
        return CONVERT_RESULT_SUCCESS;
    }
    else if (contain_string(aidlLine, "}"))
    {
        logd("%s: aidl module end with '}', handle", __func__);
        handle_aidl_module_terminator(protoFileType);
        return CONVERT_RESULT_SUCCESS;
    }

    if (aidlLine.find_last_of(";") != aidlLine.length() - 1 &&
        aidlLine.find_last_of(",") != aidlLine.length() - 1)
    {
        return CONVERT_RESULT_NOT_HANDLE;
    }

    return create_proto_message(aidlLine, protoFileType);
}

static void generate_proto_file(const string& fileName)
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    logd("%s: file name: %s", __func__, fileName.c_str());
    create_new_file(fileName, inst->pss);

    clear_proto_file_instance();
}

static void init_conversion(uint32_t aidlFileIndex, const AidlFile *aidlFile, const ProtoFile& protoFile)
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    inst->aidlFileIndex = aidlFileIndex;
    clear_proto_file_instance();
    inst->messageType[0] = protoFile.fileType;

    logd("%s: init proto header", __func__);
    init_proto_header(aidlFile);
}

static void init_aidl_type_map(map<string, string>& aidlTypeMap)
{
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_ARRAY,        PROTO_TYPE_ARRAY));
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_BOOLEAN,      PROTO_TYPE_BOOL));
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_BYTE,         PROTO_TYPE_UINT32));
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_CHAR,         PROTO_TYPE_INT32));
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_LONG,         PROTO_TYPE_INT64));
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_BYTE_ARRAY,   PROTO_TYPE_BYTES));
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_INT,          PROTO_TYPE_INT32));
    aidlTypeMap.insert(pair<string, string>(AIDL_TYPE_STRING,       PROTO_TYPE_STRING));
}

static bool get_proto_import_file(const string& line, string& importFileName)
{
    string str;

    /* e.g. "IHal.proto" */
    if (!get_key_value(line, PROTO_IMPORT, str))
        return false;

    /* e.g. "IHal.proto" -> IHal.proto */
    importFileName = get_sub_string(str, '"');
    return true;
}

static bool get_proto_message_name(const string& line, string& msgName)
{
    return get_key_value(line, PROTO_MESSAGE, msgName);
}

static bool get_proto_enum_name(const string& line, string& enumName)
{
    return get_key_value(line, PROTO_ENUM, enumName);
}

static void init_proto_node(ProtoNode& pn, ProtoNodeType type, const string& name)
{
    pn.type = type;
    pn.name = name;
    pn.field = vector<ProtoField>();
}

static void add_proto_message_node(const string& msgName, uint32_t depth, ProtoFile& protoFile)
{
    ProtoNode pn;
    vector<vector<ProtoNode>>& protoNode = protoFile.protoNode;

    init_proto_node(pn, PROTO_NODE_MESSAGE, msgName);

    if (protoNode.size() < depth)
    {
        protoNode.push_back(vector<ProtoNode>());
    }

    vector<ProtoNode>& node = protoNode[depth - 1];
    node.push_back(pn);
}

static void add_proto_enum_node(const string& enumName, uint32_t depth, ProtoFile& protoFile)
{
    ProtoNode pn;
    vector<vector<ProtoNode>>& protoNode = protoFile.protoNode;

    init_proto_node(pn, PROTO_NODE_ENUM, enumName);

    if (protoNode.size() < depth)
    {
        protoNode.push_back(vector<ProtoNode>());
    }

    vector<ProtoNode>& node = protoNode[depth - 1];
    node.push_back(pn);
}

static bool get_proto_field(const string& line, ProtoNodeType protoNodeType, string& fieldType, string& fieldName, int32_t& fieldValue)
{
    /* e.g. int32 param = 1; */
    if (contain_char(line, '=') &&
        contain_char(line, ';'))
    {
        vector<string> result;
        vector<string>::size_type size;
        string str;

        if (!split_string(line, ';', result))
            return false;

        str = result[0];

        result.clear();

        if (!split_string(str, '=', result))
            return false;

        size = result.size();
        if (size == 2)
        {
            str = result[0];
            fieldValue = (int32_t) atoi(result[1].c_str());

            result.clear();

            if (!split_string(str, ' ', result))
                return false;

            size = result.size();
            if (size == 3)
            {
                if (!result[0].compare(PROTO_OPTIONAL))
                {
                    /* Ignore "optional" */
                    fieldType = result[1];
                }
                else
                {
                    fieldType = get_proto_type(result[0], result[1]);
                }
                fieldName = result[2];
                return true;
            }
            else if (size == 2)
            {
                /* message field */
                fieldType = result[0];
                fieldName = result[1];
                return true;
            }
            else if (size == 1)
            {
                /* enum field */
                fieldType = PROTO_TYPE_INT32;
                fieldName = result[0];
                return true;
            }
            else
            {
                loge("%s: invalid proto field, size: %d, node type: %d, line: %s",
                    __func__, size, protoNodeType, line.c_str());
                return false;
            }
        }
        else
        {
            loge("%s: invalid proto field size: %d, proto node type: %d, line: %s",
                __func__, size, protoNodeType, line.c_str());
            return false;
        }
    }
    return false;
}

static void init_proto_node_field(ProtoField& pf, const string& type, const string& name, int32_t value)
{
    pf.type = type;
    pf.name = name;
    pf.value = value;
}

static void add_proto_node_field(ProtoNodeType protoNodeType, const string& protoNodeName, uint32_t depth,
    const string& fieldType, const string& fieldName, int32_t fieldValue, ProtoFile& protoFile)
{
    ProtoField pf;
    vector<vector<ProtoNode>>& protoNode = protoFile.protoNode;

    init_proto_node_field(pf, fieldType, fieldName, fieldValue);

    if (protoNode.size() < depth)
    {
        /* error. proto node isn't constructed yet. */
        return;
    }

    vector<ProtoNode>& node = protoNode[depth - 1];
    if (node.empty())
    {
        /* error. proto node is empty. */
        return;
    }

    for (vector<ProtoNode>::size_type index = 0; index < node.size(); index++)
    {
        ProtoNode& pn = node[index];
        if ((pn.type == protoNodeType) &&
            !pn.name.compare(protoNodeName))
        {
            /* found. store proto field */
            pn.field.push_back(pf);
            break;
        }
    }
}

static const ProtoFile& get_proto_file(const string& protoFileName)
{
    const string fileName = concat_string(protoFileName, PROTO_FILE_SUFFIX);
    const ProtoInfo& protoInfo = proto_get_info();
    static ProtoFile sEmptyProtoFile;

    for (vector<ProtoFile>::size_type index = 0; index < protoInfo.protoFile.size(); index++)
    {
        if (!fileName.compare(protoInfo.protoFile[index].fileName))
            return protoInfo.protoFile[index];
    }
    return sEmptyProtoFile;
}

static bool get_proto_node_name(const string& aidlIntfName, ProtoNodeType nodeType, vector<string>& nodeName)
{
    const ProtoFile& protoFile = get_proto_file(aidlIntfName);
    // logi("%s: aidlIntfName: %s, proto node type: %x, proto file name: %s",
    //      __func__, aidlIntfName.c_str(), nodeType, protoFile.fileName.c_str());
    if (protoFile.fileName.empty())
        return false;

    nodeName.clear();

    // logd("%s: protoNode size: %d", __func__, protoFile.protoNode.size());
    for (vector<vector<ProtoNode>>::size_type index = 0; index < protoFile.protoNode.size(); index++)
    {
        const vector<ProtoNode>& node = protoFile.protoNode[index];
        // logd("%s: index: %d", __func__, index);
        for (vector<ProtoNode>::size_type pn_index = 0; pn_index < node.size(); pn_index++)
        {
            const ProtoNode& pn = node[pn_index];
            // logd("%s: pn_index: %d, pn name: %s", __func__, pn_index, pn.name.c_str());
            if (pn.type == nodeType)
            {
                // logd("%s: store proto node: %s", __func__, pn.name.c_str());
                nodeName.push_back(pn.name);
            }
        }
    }
    return true;
}

static bool get_proto_message(const string& aidlIntfName, const string& msgSuffix, vector<string>& msg)
{
    vector<string> nodeName;
    if (get_proto_node_name(aidlIntfName, PROTO_NODE_MESSAGE, nodeName))
    {
        for (auto it: nodeName)
        {
            if (contain_suffix(it, msgSuffix))
                msg.push_back(it);
        }
        return !msg.empty();
    }
    return false;
}

static bool get_proto_message(const string& aidlIntfName, vector<string>& msg)
{
    vector<string> nodeName;
    if (get_proto_node_name(aidlIntfName, PROTO_NODE_MESSAGE, nodeName))
    {
        for (auto it: nodeName)
        {
            if (is_common_message(it))
                msg.push_back(it);
        }
        return !msg.empty();
    }
    return false;
}

static bool get_proto_enum_type(const string& aidlIntfName, vector<string>& enumType)
{
    return get_proto_node_name(aidlIntfName, PROTO_NODE_ENUM, enumType);
}

static bool get_proto_enum_val(const string& str, string& name, int32_t& val)
{
    vector<string> result;

    if (contain_char(str, '=') &&
        contain_char(str, ';'))
    {
        string s = str.substr(0, str.find_first_of(';'));
        if (split_string(s, '=', result))
        {
            if (result.size() == 2)
            {
                name = result[0];
                trim(name);
                val = (int32_t) atoi(result[1].c_str());
                // logd("%s: enum name: %s, val: %d", __func__, name.c_str(), val);
                return true;
            }
        }
    }
    return false;
}

static bool get_proto_enum_val(const string& enumType, vector<string>& enumVal)
{
    const ProtoInfo& protoInfo = proto_get_info();
    string protoFileName = get_full_file_name(protoInfo.protoPath, concat_string(enumType, PROTO_FILE_SUFFIX));
    ifstream fs;
    string line, enumName, name;
    int32_t val = 0;
    bool is_comment_begin_found = false;
    bool enum_type_found = false;

    /* 1. open proto file */
    if (!open_file(protoFileName, fs))
        return false;

    /* 2. read line from proto file */
    while (getline(fs, line))
    {
        trim(line);

        if (is_line_skipped(line, is_comment_begin_found))
            continue;

        if (!enum_type_found)
        {
            if (get_key_value(line, PROTO_ENUM, enumName))
            {
                // logd("%s: enumName: %s", __func__, enumName.c_str());
                enum_type_found = true;
                continue;
            }
        }
        else
        {
            if (get_proto_enum_val(line, name, val))
            {
                // logd("%s: insert name: %s, val: %d", __func__, name.c_str(), val);
                enumVal.push_back(name);
            }
        }
    }
    // logd("%s: size: %d", __func__, enumVal.size());
    return !enumVal.empty();
}

static string get_proto_file_name_stem(const string& protoFileName)
{
    return get_file_name_stem(protoFileName, PROTO_FILE_SUFFIX);
}

static bool get_proto_type(const string& str, ProtoNodeType& type, string& name)
{
    if (get_proto_message_name(str, name))
    {
        type = PROTO_NODE_MESSAGE;
    }
    else if (get_proto_enum_name(str, name))
    {
        type = PROTO_NODE_ENUM;
    }
    return false;
}

static bool is_intf_callback(const string& str)
{
    return valid_str(str) &&
            (str[0] == 'I') &&
            contain_suffix(str, PROTO_CALLBACK_SUFFIX);
}

static string get_intf_callback_name(const string& aidlIntfName)
{
    const ProtoFile& protoFile = get_proto_file(aidlIntfName);

    if (protoFile.fileName.empty())
        return "";

    for (vector<string>::size_type index = 0; index < protoFile.importFile.size(); index++)
    {
        /* e.g. IHalCallback.proto -> IHalCallback */
        const string importFileName = protoFile.importFile[index];
        const string fileNameStem = get_proto_file_name_stem(importFileName);
        if (is_intf_callback(fileNameStem))
            return fileNameStem;
    }

    /* not found */
    return "";
}

static bool is_proto_line_ignored(const string& line)
{
    return is_empty_string(line) ||
        contain_string(line, PROTO_SYNTAX) ||
        contain_string(line, PROTO_PACKAGE) ||
        contain_string(line, PROTO_IMPORT) ||
        contain_string(line, PROTO_OPTION);
}

static string get_hal_name_lower(const string& aidlPackageName)
{
    logd("%s: aidl package name: %s", __func__, aidlPackageName.c_str());
    if (!valid_str(aidlPackageName))
    {
        loge("%s: empty aidl package name", __func__);
        return "";
    }

    string protoPackageName = proto_get_package_name(aidlPackageName);
    logd("%s: proto package name: %s", __func__, protoPackageName.c_str());

    replace_string(protoPackageName, ".", "-");
    logd("%s: hal name lower: %s", __func__, protoPackageName.c_str());
    return protoPackageName;
}

static string get_hal_name_lower()
{
    const Aidl2ProtoParam& param = proto_get_param();
    if (param.packageName != "")
    {
        logd("%s: hal name: %s", __func__, param.packageName.c_str());
        return param.packageName;
    }

    string aidlIntfRoot = aidl_get_intf_root();
    if (is_empty_string(aidlIntfRoot))
    {
        /* empty aidl intf root */
        return "";
    }
    logd("%s: aidl intf root name: %s", __func__, aidlIntfRoot.c_str());

    string aidlPackageName = aidl_get_package_name(aidlIntfRoot);
    logd("%s: aidl package name: %s", __func__, aidlPackageName.c_str());

    string hal_name_lower = contain_string(aidlPackageName, AIDL_HAL_PACKAGE_PREFIX) ?
                            get_hal_name_lower(aidlPackageName) :
                            get_format_string_in_lower(aidl_get_interface_short_name(aidlIntfRoot), '-');
    logd("%s: hal name: %s", __func__, hal_name_lower.c_str());
    return hal_name_lower;
}

static string get_aidl_package_name()
{
    string aidlIntfRoot = aidl_get_intf_root();
    if (is_empty_string(aidlIntfRoot))
    {
        /* empty aidl intf root */
        return "";
    }
    logd("%s: aidl intf root name: %s", __func__, aidlIntfRoot.c_str());

    return aidl_get_package_name(aidlIntfRoot);
}

static string get_proto_package_name()
{
    string aidlPackageName = get_aidl_package_name();
    return !is_empty_string(aidlPackageName) ?
            proto_get_package_name(aidlPackageName) :
            "";
}

static string get_proto_package_line()
{
    string protoPackageName = get_proto_package_name();
    stringstream ss;
    ss << PROTO_PACKAGE << " " << protoPackageName << ';';
    return ss.str();
}

/* ------------------------------------------------------ */

void proto_util_init(const string& protoPath)
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    inst->protoPath = protoPath;
    init_aidl_type_map(inst->aidlTypeMap);
}

void proto_util_deinit()
{
    ProtoFileInstance *inst = &sProtoFileInstance;
    inst->aidlTypeMap.clear();
}

string proto_get_file_name(const string& aidlFileName)
{
    /* e.g. "IHal.aidl" -> "IHal.proto" */
    return concat_string(aidl_get_file_name_stem(aidlFileName), PROTO_FILE_SUFFIX);
}

string proto_get_package_name(const string& aidlPackage)
{
    string prefix = AIDL_HAL_PACKAGE_PREFIX;
    /* e.g. "android.hardware.hal" -> "hal" */
    return valid_str(aidlPackage) && contain_string(aidlPackage, prefix) ?
        aidlPackage.substr(aidlPackage.find(prefix) + prefix.length(), string::npos) :
        get_format_string_in_lower(aidl_get_interface_short_name(aidl_get_intf_root()), '.');
}

bool proto_get_package_name(const string& protoFileName, string& packageName)
{
    const ProtoFile& protoFile = get_proto_file(protoFileName);
    packageName = protoFile.packageName;
    return valid_str(packageName);
}

bool proto_get_namespace(const string& protoFileName, string& protoNamespace)
{
    string packageName;
    if (proto_get_package_name(protoFileName, packageName))
    {
        protoNamespace = new_string(packageName, '.', "::");
        return true;
    }
    return false;
}

ProtoType proto_get_file_type(AidlType aidlFileType)
{
    switch (aidlFileType)
    {
        case AIDL_TYPE_INTERFACE:
            return PROTO_TYPE_REQ;
        case AIDL_TYPE_CALLBACK:
            return PROTO_TYPE_IND;
        case AIDL_TYPE_ENUM:
            return PROTO_TYPE_ENUM;
        case AIDL_TYPE_PARCELABLE:
            return PROTO_TYPE_COMMON;
        case AIDL_TYPE_UNION:
            return PROTO_TYPE_ONEOF;
        default:
            return PROTO_TYPE_UNKNOW;
    }
}

bool proto_create_file(const string& aidlPath, uint32_t aidlFileIndex, const AidlFile *aidlFile,
    const string& protoPath, const ProtoFile& protoFile, bool removeFile)
{
    ConvertResult res = 0;
    string aidlFileName = get_full_file_name(aidlPath, aidlFile->fileName);
    string protoFileName = get_full_file_name(protoPath, protoFile.fileName);
    string line, preLine;
    stringstream ss(aidlFile->ss.str());

    if (!removeFile &&
        exist_file(protoFileName))
    {
        /* proto file already generated, return */
        logd("%s: exist proto file: %s", __func__, protoFileName.c_str());
        return true;
    }

    init_conversion(aidlFileIndex, aidlFile, protoFile);

    /* 2. read line from aidl file */
    while (getline(ss, line))
    {
        trim(line);

        if (!preLine.empty())
        {
            line = preLine + line;
            preLine = "";
        }

        res = convert_aidl_2_proto_line(line, aidlFile->packageName);
        if (IS_CONVERT_FAIL(res))
        {
            loge("%s: fail to convert aidl into proto line, %d(%s)", __func__, errno, strerror(errno));
            break;
        }
        else if (IS_CONVERT_NOT_HANDLE(res))
        {
            preLine = line;
            logd("%s: not handle %s", __func__, preLine.c_str());
        }
    }

    if (IS_CONVERT_SUCCESS(res))
    {
        generate_proto_file(protoFileName);
    }

    return IS_CONVERT_SUCCESS(res);
}

const char *proto_map_file_type_2_string(ProtoType fileType)
{
    switch (fileType)
    {
        case PROTO_TYPE_COMMON:
            return "common message";
        case PROTO_TYPE_ONEOF:
            return "oneof";
        case PROTO_TYPE_REQ:
            return "req";
        case PROTO_TYPE_IND:
            return "ind";
        case PROTO_TYPE_CFM:
            return "cfm";
        case PROTO_TYPE_ENUM:
            return "enum";
        default:
            return "unknown";
    }
}

void proto_read_file(const string& protoPath, ProtoFile& protoFile)
{
    ifstream fs;
    string line, str, fieldType, fieldName;
    int32_t fieldValue = 0;
    bool is_comment_begin_found = false;
    uint32_t depth = 0;
    string protoFileName = get_full_file_name(protoPath, protoFile.fileName);
    ProtoNodeHeader currentNode;

    /* 1. open proto file */
    if (!open_file(protoFileName, fs))
        return;

    /* 2. read line from proto file */
    while (getline(fs, line))
    {
        trim(line);

        if (is_line_skipped(line, is_comment_begin_found))
            continue;

        /* ignore syntax, package, option */
        if (get_proto_import_file(line, str))
        {
            /* store import file */
            protoFile.importFile.push_back(str);
            continue;
        }
        else if (contain_char(line, '{'))
        {
            /* proto module begin */
            ++depth;
            // logd("%s: find '{', module begin, depth: %d", __func__, depth);
            if (currentNode.type == PROTO_NODE_MESSAGE)
            {
                add_proto_message_node(currentNode.name, depth, protoFile);
            }
            else if (currentNode.type == PROTO_NODE_ENUM)
            {
                add_proto_enum_node(currentNode.name, depth, protoFile);
            }
            else
            {
                loge("%s: unknown proto node type", __func__, currentNode.type);
            }
            continue;
        }
        else if (contain_char(line, '}'))
        {
            /* proto module end */
            if (depth > 0)
            {
                --depth;
            }
            // logd("%s: find '}', module end, depth: %d", __func__, depth);
            continue;
        }
        else if (is_proto_line_ignored(line))
        {
            continue;
        }
        else if (get_proto_type(line, currentNode.type, currentNode.name))
        {
            continue;
        }
        else if (get_proto_field(line, currentNode.type, fieldType, fieldName, fieldValue))
        {
            // logd("%s: >> current nodeType: %x, fieldType: %s, fieldName: %s, fieldValue: %d", __func__,
            //     currentNode.type, fieldType.c_str(), fieldName.c_str(), fieldValue);
            if ((currentNode.type == PROTO_NODE_MESSAGE) || (currentNode.type == PROTO_NODE_ENUM))
            {
                add_proto_node_field(currentNode.type, currentNode.name, depth, fieldType, fieldName, fieldValue, protoFile);
            }
            continue;
        }
    }
}

bool proto_get_message_req(const string& aidlIntfName, vector<string>& msg)
{
    return valid_str(aidlIntfName) && get_proto_message(aidlIntfName, PROTO_MESSAGE_REQ, msg);
}

bool proto_get_message_ind(const string& aidlIntfName, vector<string>& msg)
{
    return valid_str(aidlIntfName) && get_proto_message(aidlIntfName, PROTO_MESSAGE_IND, msg);
}

bool proto_get_message_cfm(const string& aidlIntfName, vector<string>& msg, bool excludeLastCommonCfm)
{
    vector<string> cfm;
    if (!valid_str(aidlIntfName))
        return false;

    if (get_proto_message(aidlIntfName, PROTO_MESSAGE_CFM, cfm))
    {
        vector<string>::size_type size = cfm.size();
        if (excludeLastCommonCfm)
            --size;

        for (vector<string>::size_type index = 0; index < size; index++)
            msg.push_back(cfm[index]);

        return true;
    }
    return false;
}

bool proto_get_common_message(const string& aidlIntfName, vector<string>& msg)
{
    return valid_str(aidlIntfName) && get_proto_message(aidlIntfName, msg);
}

bool proto_get_enum_type(const string& aidlIntfName, vector<string>& enumType)
{
    return valid_str(aidlIntfName) && get_proto_enum_type(aidlIntfName, enumType);
}

bool proto_get_enum_val(const string& enumType, vector<string>& enumVal)
{
    return valid_str(enumType) && get_proto_enum_val(enumType, enumVal);
}

string proto_get_intf_callback_name(const string& aidlIntfName)
{
    return valid_str(aidlIntfName) ?
            get_intf_callback_name(aidlIntfName) :
            "";
}

string proto_get_header_file_name(const string& aidlIntfName)
{
    return valid_str(aidlIntfName) ?
            concat_string(aidlIntfName, PROTO_HEADER_SUFFIX) :
            "";
}

string proto_get_hal_name_lower(const string& aidlPackageName)
{
    return get_hal_name_lower(aidlPackageName);
}

bool proto_create_hal_status_file(const string& protoPath, bool removeFile)
{
    const ProtoInfo& protoInfo = proto_get_info();
    string protoPackageLine = get_proto_package_line();
    string hal_status_file = get_full_file_name(protoPath, HAL_STATUS_PROTO_FILE);
    string str = PROTO_HAL_STATUS;

    if (!protoInfo.genHalStatus)
    {
        logd("%s: not gen %s", __func__, HAL_STATUS_PROTO_FILE);
        return false;
    }

    if (is_empty_string(protoPackageLine))
    {
        return false;
    }

    if (!removeFile &&
        exist_file(hal_status_file))
    {
        /* exist, return */
        return true;
    }

    logd("%s: proto package line: %s", __func__, protoPackageLine.c_str());
    replace_string(str, PROTO_PACKAGE_HAL, protoPackageLine);

    logi("a2p: create %s", HAL_STATUS_PROTO_FILE);
    return create_file(hal_status_file, str);
}

bool proto_create_android_bp(const string& protoPath, bool removeFile)
{
    return proto_create_makefile(protoPath, removeFile, "Android.bp", PROTO_ANDROID_BP);
}

bool proto_create_linux_makefile(const string& protoPath, bool removeFile)
{
    return proto_create_makefile(protoPath, removeFile, "Makefile", PROTO_LINUX_MAKEFILE);
}

bool proto_create_makefile(const string& protoPath, bool removeFile, const string& name, const string& prototype)
{
    string makefile = get_full_file_name(protoPath, name);
    string str = prototype;
    string aidlPackageName = get_aidl_package_name();
    string hal_name_lower = get_hal_name_lower();

    if (is_empty_string(aidlPackageName) ||
        is_empty_string(hal_name_lower))
    {
        return false;
    }

    if (!removeFile &&
        exist_file(makefile))
    {
        /* exist, return */
        return true;
    }

    logd("%s: makefile: %s", __func__, makefile.c_str());
    if (contain_string(hal_name_lower, "qti"))
        replace_string(str, "qti-hal", hal_name_lower);
    else
        replace_string(str, "hal", hal_name_lower);

    return create_file(makefile, str);
}
