/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <vector>

#include "aidl.h"
#include "aidl_util.h"
#include "log.h"
#include "util.h"

using std::istringstream;
using std::size_t;
using std::vector;

/* ------------------------------------------------------ */

static bool is_interface(const AidlFile *aidlFile)
{
    return (aidlFile != NULL) &&
            ((aidlFile->fileType == AIDL_TYPE_INTERFACE) ||
            (aidlFile->fileType == AIDL_TYPE_CALLBACK));
}

static void dump_aidl_param(const AidlParameter& param)
{
    /* only dump aidl param out */
    if (IS_AIDL_PARAM_OUT(param.dir))
        logd("%s: %s %s %s", __func__, IS_AIDL_PARAM_OUT(param.dir) ? "out" : "in",
            param.type.c_str(), param.name.c_str());
}

static void dump_aidl_param_list(const vector<AidlParameter>& paramList)
{
    for (auto it: paramList)
        dump_aidl_param(it);
}

static string get_aidl_type_name(const string& importFile)
{
    size_t pos = importFile.find_last_of('.');
    return (pos != string::npos) ? importFile.substr(pos + 1, string::npos) : importFile;
}

static void set_param_qualifier(const string& str, AidlParameter& param)
{
    if (is_same_string(str, AIDL_IN))
    {
        param.dir = AIDL_PARAM_IN;
    }
    else if (is_same_string(str, AIDL_OUT))
    {
        param.dir = AIDL_PARAM_OUT;
    }
    else if (is_same_string(str, AIDL_AT_NULLABLE))
    {
        logd("%s: param nullable", __func__);
        param.nullable = true;
    }
}

static bool get_single_param(const string& str, AidlParameter& param)
{
    vector<string> result;
    const char delim = ' ';
    vector<string>::size_type size;

    if (!split_string(str, delim, result))
    {
        return false;
    }

    size = result.size();

    /* default param dir: in */
    param.dir = AIDL_PARAM_IN;
    param.nullable = false;

    if (size == 2)
    {
        param.type = result[0];
        param.name = result[1];
    }
    else if (size == 3)
    {
        set_param_qualifier(result[0], param);
        param.type = result[1];
        param.name = result[2];
    }
    else if (size >= 4)
    {
        if (is_same_string(result[2], "="))
        {
            param.type = result[0];
            param.name = result[1];
        }
        else
        {
            for (size_t index = 0; index < size - 2; index++)
            {
                set_param_qualifier(result[index], param);
            }
            param.type = result[size-2];
            param.name = result[size-1];
        }
    }
    else
    {
        /* invalid param */
        logw("%s invalid param: %s ", __func__, str.c_str());
        return false;
    }
    return true;
}

static bool add_single_param(const string& str, vector<AidlParameter>& paramList)
{
    AidlParameter param;
    if (!get_single_param(str, param))
    {
        /* fail to parse param */
        loge("%s failed to parse '%s' ", __func__, str.c_str());
        return false;
    }
    paramList.push_back(param);
    return true;
}

static bool get_func_param(const string& str, vector<AidlParameter>& paramList)
{
    vector<string> result;
    const char delim = ',';

    paramList.clear();

    /* case1. empty param */
    if (is_empty_string(str))
    {
        return true;
    }

    /* case2. one param */
    if (!contain_char(str, delim))
    {
        return add_single_param(str, paramList);
    }

    /* case3. multi param */
    if (!split_string(str, delim, result))
    {
        return false;
    }

    for (auto val : result)
    {
        /* remove space ahead of paramters */
        /* e.g. ' in bool arg2' */
        trim(val);

        if (!add_single_param(val, paramList))
        {
            return false;
        }
    }
    return true;
}

static bool get_func_name_and_return_type(const string& str, string& returnType, string& funcName)
{
    vector<string> result;
    const char delim = ' ';

    returnType = "";
    funcName = "";

    /* e.g. "void foo" */
    if (!split_string(str, delim, result))
    {
        return false;
    }

    for (auto val : result)
    {
        trim(val);

        if (aidl_is_qualifier(val))
        {
            /* ignore */
            continue;
        }

        /* 1. return type. e.g. "void" | "int[]" | "IHal" */
        if (is_empty_string(returnType))
        {
            returnType = val;
            continue;
        }
        else if (is_same_string(val, "["))
        {
            returnType += "[";
            continue;
        }
        else if (is_same_string(val, "]"))
        {
            returnType += "]";
            continue;
        }

        /* 2. func name */
        if (is_empty_string(funcName))
        {
            funcName = val;
            break;
        }
    }
    return true;
}

static bool is_aidl_interface(const string& str)
{
    return contain_char(str, '(') && contain_char(str, ')');
}

static const AidlFile *get_aidl_file(const string& aidlIntfName, AidlType fileType = AIDL_TYPE_INTERFACE)
{
    const AidlInfo& aidlInfo = aidl_get_info();

    for (vector<AidlFile*>::size_type index = 0; index < aidlInfo.aidlFile.size(); index++)
    {
        AidlFile *aidlFile = aidlInfo.aidlFile[index];
        if (aidlFile != nullptr)
        {
            if ((aidlFile->fileType == fileType) &&
                is_same_string(aidlFile->typeName, aidlIntfName))
            {
                return aidlFile;
            }
        }
    }
    return nullptr;
}

static const AidlFile *get_aidl_intf_file(const string& aidlIntfName)
{
    return aidl_is_callback(aidlIntfName) ?
            get_aidl_file(aidlIntfName, AIDL_TYPE_CALLBACK) :
            get_aidl_file(aidlIntfName, AIDL_TYPE_INTERFACE);
}

static void copy_enum_type(vector<string>& dstEnumType, const vector<string>& srcEnumType)
{
    for (vector<string>::size_type index = 0; index < srcEnumType.size(); index++)
    {
        dstEnumType.push_back(srcEnumType[index]);
    }
}

static void get_enum_list(vector<string>& enumType)
{
    const AidlInfo& aidlInfo = aidl_get_info();

    for (vector<AidlFile*>::size_type index = 0; index < aidlInfo.aidlFile.size(); index++)
    {
        AidlFile *aidlFile = aidlInfo.aidlFile[index];
        if (aidlFile != nullptr)
        {
            AidlType fileType = aidlFile->fileType;
            switch (fileType)
            {
                case AIDL_TYPE_INTERFACE:
                    /* fall-through */
                case AIDL_TYPE_CALLBACK:
                {
                    AidlInterfaceFile *intfFile = reinterpret_cast<AidlInterfaceFile *>(aidlFile);
                    copy_enum_type(enumType, intfFile->enumType);
                    break;
                }
                case AIDL_TYPE_PARCELABLE:
                {
                    AidlParcelableFile *parcelableFile = reinterpret_cast<AidlParcelableFile *>(aidlFile);
                    copy_enum_type(enumType, parcelableFile->enumType);
                    break;
                }
                case AIDL_TYPE_ENUM:
                {
                    enumType.push_back(aidlFile->typeName);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

static bool get_import_type_name(const string& typeName, vector<string>& importTypeName)
{
    const AidlFile *aidlFile = get_aidl_file(typeName);
    importTypeName.clear();
    if (is_interface(aidlFile))
    {
        for (auto importFile: aidlFile->importFile)
        {
            importTypeName.push_back(get_aidl_type_name(importFile));
        }
        return true;
    }
    return false;
}

static void init_aidl_file(AidlFile *aidlFile, const string& fileName, const string& packageName,
        const vector<string>& importFile, const string& messageName, AidlType fileType)
{
    aidlFile->fileName = fileName;
    aidlFile->packageName = packageName;
    aidlFile->messageName = messageName;
    aidlFile->fileType = fileType;
    aidlFile->typeName = aidl_get_file_name_stem(fileName);
    aidlFile->importFile = importFile;
    aidlFile->ss.str("");
    aidlFile->ss.clear();
}

static bool contain_operator(const string& str)
{
    /* TODO: Handle other operator */
    return contain_string(str, "+");
}

static bool retrieve_enum_val(const map<string, int32_t>& enumKeyValMap, const string& key, int32_t& val)
{
    logd("%s: map size: %d, key: %s", __func__, enumKeyValMap.size(), key.c_str());
    auto it = enumKeyValMap.find(key);
    if (it != enumKeyValMap.end())
    {
        val = it->second;
        return true;
    }
    return false;
}

static bool get_enum_val(const string& str, const map<string, int32_t>& enumKeyValMap, char op, int32_t& val)
{
    vector<string> result;
    if (contain_char(str, op) &&
        split_string(str, op, result) &&
        (result.size() >= 2))
    {
        val = 0;
        for (auto it: result)
        {
            int32_t v = 0;
            string s = it;
            trim(s);
            if (!get_number_val(s, v))
            {
                /* find another enum's key */
                if (!retrieve_enum_val(enumKeyValMap, s, v))
                {
                    loge("%s: can not find val for another enum key: %s", __func__, s.c_str());
                    return false;
                }
                logd("%s: another enum key: %s, val: %d", __func__, s.c_str(), v);
            }
            /* FIXME: Handle according to 'op' */
            val += v;
        }
        return true;
    }
    return false;
}

static bool get_enum_val(const string& str, const map<string, int32_t>& enumKeyValMap, int32_t& val)
{
    return get_enum_val(str, enumKeyValMap, '+', val);
}

/* ------------------------------------------------------ */

bool aidl_get_file_type(const string& line, AidlType& fileType)
{
    if (contain_string(line, AIDL_INTERFACE))
    {
        string val;
        string s = line.substr(line.find(AIDL_INTERFACE));

        get_key_value(s, AIDL_INTERFACE, val, '{');
        trim(val);
        fileType = aidl_contain_callback_suffix(val) ?
                    AIDL_TYPE_CALLBACK :
                    AIDL_TYPE_INTERFACE;
        // logd("%s: line: %s, str: %s, file type: %x", __func__, line.c_str(), val.c_str(), fileType);
    }
    else if (contain_string(line, AIDL_PARCELABLE))
    {
        fileType = AIDL_TYPE_PARCELABLE;
    }
    else if (contain_string(line, AIDL_UNION))
    {
        fileType = AIDL_TYPE_UNION;
    }
    else if (contain_string(line, AIDL_ENUM))
    {
        fileType = AIDL_TYPE_ENUM;
    }
    else
    {
        return false;
    }
    return true;
}

bool aidl_contain_callback_suffix(const string& str)
{
    return contain_suffix(str, AIDL_CALLBACK_SUFFIX_CALLBACK) ||
            contain_suffix(str, AIDL_CALLBACK_SUFFIX_CALLBACKS) ||
            contain_suffix(str, AIDL_CALLBACK_SUFFIX_LISTENER);
}

bool aidl_is_callback(const string& str)
{
    return (!str.empty()) &&
            (str[0] == 'I') &&
            aidl_contain_callback_suffix(str);
}

bool aidl_is_qualifier(const string& str)
{
    return is_same_string(str, AIDL_AT) ||
        is_same_string(str, AIDL_IN) ||
        is_same_string(str, AIDL_ONEWAY) ||
        is_same_string(str, AIDL_OUT);
}

bool aidl_get_package_name(const string& line, string& packageName)
{
    return get_key_value(line, AIDL_PACKAGE, packageName);
}

bool aidl_get_import_file(const string& line, string& fileName)
{
    return get_key_value(line, AIDL_IMPORT, fileName);
}

bool aidl_get_interface(const string& line, AidlInterface& intf)
{
    size_t pos;
    size_t end_pos;

    if (!is_aidl_interface(line))
    {
        return false;
    }

    pos = line.find('(');
    end_pos = line.find(')');

    if (end_pos <= pos)
    {
        return false;
    }

    /* 1. get func name and return type. e.g. "void foo" */
    get_func_name_and_return_type(line.substr(0, pos), intf.returnType, intf.funcName);

    /* 2. get func parameter. e.g. "int i, int j" */
    get_func_param(((pos + 1) == end_pos) ? "" : line.substr(pos + 1, end_pos - pos - 1), intf.param);
    dump_aidl_param_list(intf.param);
    return true;
}

bool aidl_get_common_field(const string& line, AidlParameter& param)
{
    const char delim = ';';
    /* e.g. "String ifaceName;" */
    if (contain_char(line, delim))
    {
        vector<string> result;

        if (!split_string(line, delim, result))
        {
            return false;
        }

        return get_single_param(result[0], param);
    }
    return false;
}

bool aidl_get_message_name_and_type(const string& aidlLine, string& name)
{
    string type = "";
    return aidl_get_message_name_and_type(aidlLine, name, type);
}

bool aidl_get_message_name_and_type(const string& aidlLine, string& name, string& type)
{
    const char delim = ' ';
    vector<string> result;
    if (split_string(aidlLine, delim, result))
    {
        type = result[0];
        if (result.size() > 1)
        {
            name = result[1];
        }
        return true;
    }
    return false;
}

bool aidl_is_enum_field(const string& line)
{
    return contain_char(line, ',');
}

bool aidl_get_enum_field(const string& line, string& str)
{
    vector<string> result;
    const char delim = ',';

    if (!split_string(line, delim, result))
    {
        return false;
    }

    str = result[0];
    return true;
}

bool aidl_get_enum_key(const string& aidlString, string& enumKey)
{
    vector<string> result;
    string enumField;

    if (!aidl_get_enum_field(aidlString, enumField))
        return false;

    if (contain_char(aidlString, '='))
    {
        const char delim = '=';

        /* enumField does not include ',' */
        if (!split_string(enumField, delim, result))
            return false;

        /* get enum key */
        trim(result[0]);
        enumKey = result[0];
    }
    else
    {
        trim(enumField);
        enumKey = enumField;
    }
    return true;
}

bool aidl_get_enum_key_val(const string& aidlString, const map<string, int32_t>& enumKeyValMap,
    string& enumKey, int32_t& enumVal, int32_t& enumValNew)
{
    vector<string> result;
    string enumField;

    if (!aidl_get_enum_field(aidlString, enumField))
        return false;

    if (contain_char(aidlString, '='))
    {
        const char delim = '=';

        /* enumField does not include ',' */
        if (!split_string(enumField, delim, result))
            return false;

        /* get enum key */
        trim(result[0]);
        enumKey = result[0];

        /* get enum value */
        aidl_get_enum_val(result[1], enumKeyValMap, enumValNew);
    }
    else
    {
        trim(enumField);
        enumKey = enumField;
        enumValNew = enumVal;
    }
    return true;
}

bool aidl_get_enum_val(const string& enumValStr, const map<string, int32_t>& enumKeyValMap, int32_t& enumVal)
{
    vector<string> result;
    string str = enumValStr;
    const string delim = "<<";
    size_t pos;
    stringstream ss;
    int32_t val = 0;
    int32_t shift = 0;

    trim(str);
    enumVal = 0;

    if (contain_operator(str))
    {
        return get_enum_val(str, enumKeyValMap, enumVal);
    }
    else if (!contain_string(str, "<<"))
    {
        return get_number_val(str, enumVal);
    }

    /* e.g. "1 << 0" -> 1 */
    pos = str.find(delim);
    if (pos != string::npos)
    {
        ss << str.substr(0, pos);
        ss >> val;

        ss.clear();
        ss.str("");

        ss << str.substr(pos + delim.length(), string::npos);
        ss >> shift;

        enumVal = (val << shift);
        return true;
    }
    return false;
}

string aidl_get_file_name_stem(const string& aidlFileName)
{
    return get_file_name_stem(aidlFileName, AIDL_FILE_SUFFIX);
}

string aidl_get_interface_short_name(const string& interfaceName)
{
    /* e.g. "IHalEventCallback" => "Hal" */
    if (contain_suffix(interfaceName, AIDL_CALLBACK_SUFFIX_CALLBACKS))
        return get_string(interfaceName, "I", "Event", AIDL_CALLBACK_SUFFIX_CALLBACKS);
    else if (contain_suffix(interfaceName, AIDL_CALLBACK_SUFFIX_LISTENER))
        return get_string(interfaceName, "I", "Event", AIDL_CALLBACK_SUFFIX_LISTENER);
    else
        return get_string(interfaceName, "I", "Event", AIDL_CALLBACK_SUFFIX_CALLBACK);
}

const char *aidl_map_file_type_2_string(AidlType fileType)
{
    switch (fileType)
    {
        case AIDL_TYPE_INTERFACE:
            return "interface";
        case AIDL_TYPE_CALLBACK:
            return "callback";
        case AIDL_TYPE_PARCELABLE:
            return "parcelable";
        case AIDL_TYPE_ENUM:
            return "enum";
        default:
            return "unknown";
    }
}

void aidl_dump_param(const AidlParameter& param, size_t index)
{
    logd("aidl param[%d]: %s %s", index, param.type.c_str(), param.name.c_str());
}

void aidl_dump_param_list(const vector<AidlParameter>& param)
{
    size_t size = param.size();
    logd("aidl param size %d", size);
    for (size_t index = 0; index < size; index++)
    {
        aidl_dump_param(param[index], index);
    }
}

void aidl_dump_interface(const AidlInterface& intf)
{
    logd("aidl interface (return type: %s, func name: %s)",
        intf.returnType.c_str(), intf.funcName.c_str());

    aidl_dump_param_list(intf.param);
}

void aidl_get_type_name(const AidlInfo& aidlInfo, AidlType type, vector<string>& aidlTypeName)
{
    const vector<AidlFile *>& aidlFile = aidlInfo.aidlFile;

    for (vector<string>::size_type index = 0; index < aidlFile.size(); index++)
    {
        if (aidlFile[index]->fileType == type)
        {
            string name = aidl_get_file_name_stem(aidlFile[index]->fileName);
            aidlTypeName.push_back(name);
        }
    }
}

const string aidl_get_package_name(const string& aidlIntfName)
{
    const AidlFile *aidlFile = get_aidl_intf_file(aidlIntfName);
    return (aidlFile != nullptr) ? aidlFile->packageName : "";
}

bool aidl_is_interface(const string& typeName)
{
    const AidlInfo& aidlInfo = aidl_get_info();

    if (aidlInfo.aidlFile.empty())
        return false;

    for (vector<AidlFile*>::size_type index = 0; index < aidlInfo.aidlFile.size(); index++)
    {
        AidlFile* aidlFile = aidlInfo.aidlFile[index];
        if (is_interface(aidlFile))
        {
            string aidlFileName = aidl_get_file_name_stem(aidlFile->fileName);
            if (is_same_string(aidlFileName, typeName))
                return true;
        }
    }
    return false;
}

const vector<AidlInterface>& aidl_get_interface_list(const string& aidlIntfName)
{
    const AidlFile *aidlFile = get_aidl_intf_file(aidlIntfName);
    static const vector<AidlInterface> sEmptyIntf;
    if (aidlFile != nullptr)
    {
        const AidlInterfaceFile *intfFile = reinterpret_cast<const AidlInterfaceFile *>(aidlFile);
        return intfFile->intf;
    }
    return sEmptyIntf;
}

const vector<string>& aidl_get_import_list(const string& aidlIntfName)
{
    const AidlFile *aidlFile = get_aidl_intf_file(aidlIntfName);
    const static vector<string> sEmptyImportFile;
    return aidlFile != nullptr ? aidlFile->importFile : sEmptyImportFile;
}

void aidl_get_enum_list(vector<string>& enumType)
{
    get_enum_list(enumType);
}

bool aidl_is_at_line(const string& line)
{
    return line.find(AIDL_AT) == 0;
}

bool aidl_is_array(const string& aidlType)
{
    /* e.g. "int[]" */
    return contain_char(aidlType, '[') && contain_char(aidlType, ']');
}

string aidl_get_array_size(const string& aidlType)
{
    if (aidl_is_array(aidlType))
    {
        size_t pos = aidlType.find_first_of('[');
        size_t end_pos = aidlType.find_last_of(']');
        if ((pos != string::npos) &&
            (end_pos != string::npos) &&
            (end_pos > pos + 1))
        {
            string str = aidlType.substr(pos + 1, end_pos - pos - 1);
            return valid_str(str) ? str : "";
        }
    }
    return "";
}

string aidl_get_array_item_type(const string& aidlType)
{
    /* e.g. "int[]" -> "int" */
    return aidlType.substr(0, aidlType.find('['));
}

string aidl_get_include_header(const string& importFile)
{
    stringstream ss;
    string str = new_string(importFile, '.', "/");
    /* e.g. "android.hardware.hal.IHal" -> "aidl/android/hardware/hal/IHal.h" */
    ss << AIDL_NAME << '/' << str << ".h";
    return ss.str();
}

string aidl_get_include_header(const string& packageName, const string& typeName)
{
    stringstream ss;
    string str = new_string(packageName, '.', "/");
    /* e.g. "android.hardware.hal" + "IHal" -> "aidl/android/hardware/hal/IHal.h" */
    ss << AIDL_NAME << '/' << str << '/' << typeName << ".h";
    return ss.str();
}

string aidl_get_using_type(const string& importFile)
{
    stringstream ss;
    string str = new_string(importFile, '.', "::");
    /* e.g. "android.hardware.hal.IHal" -> "aidl::android::hardware::hal::IHal" */
    ss << AIDL_NAME << "::" << str;
    return ss.str();
}

string aidl_get_using_type(const string& packageName, const string& typeName)
{
    stringstream ss;
    string str = new_string(packageName, '.', "::");
    /* e.g. "android.hardware.hal" + "IHal" -> "aidl::android::hardware::hal::IHal" */
    ss << AIDL_NAME << "::" << str << "::" << typeName;
    return ss.str();
}

string aidl_get_using_type(const string& packageName, const string& rootTypeName, const string& internalTypeName)
{
    stringstream ss;
    string str = new_string(packageName, '.', "::");
    /* e.g. "android.hardware.hal" + "IHal" + "InternalType" -> "aidl::android::hardware::hal::IHal::InternalType" */
    ss << AIDL_NAME << "::" << str << "::" << rootTypeName << "::" << internalTypeName;
    return ss.str();
}

void aidl_get_hal_namespace(const string& packageName, vector<string>& halNamespace)
{
    char delim = '.';
    halNamespace.clear();
    halNamespace.push_back(AIDL_NAME);
    split_string(packageName, delim, halNamespace, false);
}

AidlFile *aidl_new_file(AidlType fileType)
{
    AidlFile *aidlFile;
    switch (fileType)
    {
        case AIDL_TYPE_INTERFACE:
            /* fall-through */
        case AIDL_TYPE_CALLBACK:
        {
            aidlFile = new AidlInterfaceFile;
            break;
        }
        case AIDL_TYPE_PARCELABLE:
        {
            aidlFile = new AidlParcelableFile;
            break;
        }
        default:
        {
            aidlFile = new AidlFile;
            break;
        }
    }
    return aidlFile;
}

AidlFile *aidl_new_file(const string& fileName, const string& packageName, const vector<string>& importFile,
        const string& messageName, AidlType fileType)
{
    AidlFile *aidlFile = aidl_new_file(fileType);
    init_aidl_file(aidlFile, fileName, packageName, importFile, messageName, fileType);
    return aidlFile;
}

const AidlFile *aidl_get_file(uint32_t aidlFileIndex)
{
    const AidlInfo& aidlInfo = aidl_get_info();
    return (aidlFileIndex < aidlInfo.aidlFile.size()) ?
        aidlInfo.aidlFile[aidlFileIndex] :
        NULL;
}

const vector<string>& aidl_get_enum_type()
{
    const AidlInfo& aidlInfo = aidl_get_info();
    return aidlInfo.enumType;
}

bool aidl_is_enum_type(const string& typeName)
{
    const vector<string>& enumType = aidl_get_enum_type();
    return contain_string(enumType, typeName);
}

bool aidl_is_intf_root(const string& aidlIntfName)
{
    const AidlInfo& aidlInfo = aidl_get_info();
    return is_same_string(aidlInfo.aidlIntfRoot, aidlIntfName);
}

const string& aidl_get_intf_root()
{
    const AidlInfo& aidlInfo = aidl_get_info();
    return aidlInfo.aidlIntfRoot;
}

const string& aidl_get_hal_status_type()
{
    const AidlInfo& aidlInfo = aidl_get_info();
    return aidlInfo.halStatusType;
}

bool aidl_get_callback_name(const string& aidlIntfName, AidlCallbackName& aidlCallbackName)
{
    vector<string> importTypeName;
    aidlCallbackName.callbackName = "";
    aidlCallbackName.callback2Name = "";
    aidlCallbackName.callbackResultName = "";
    // logd("%s: ", __func__);
    if (get_import_type_name(aidlIntfName, importTypeName))
    {
        // dump_string("import type", importTypeName);
        for (auto rit = importTypeName.crbegin(); rit != importTypeName.crend(); ++rit)
        {
            string name = *rit;
            bool found_callback_result = false;
            if (aidl_is_callback(name))
            {
                // logd("%s: %s", __func__, it.c_str());
                const AidlFile *aidlCallbackFile = get_aidl_file(name, AIDL_TYPE_CALLBACK);
                if (is_interface(aidlCallbackFile))
                {
                    // logd("%s: aidl callback is intf", __func__);
                    const AidlInterfaceFile *intfFile = reinterpret_cast<const AidlInterfaceFile *>(aidlCallbackFile);
                    for (auto intf: intfFile->intf)
                    {
                        // logd("%s: %s, return type: %s", __func__, it.c_str(), intf.returnType.c_str());
                        if (!is_same_string(intf.returnType, AIDL_TYPE_VOID))
                        {
                            /* found */
                            found_callback_result = true;
                            // logd("%s: %s, found callback result", __func__, it.c_str());
                            break;
                        }
                    }
                    if (found_callback_result)
                    {
                        aidlCallbackName.callbackResultName = *rit;
                        // logd("%s: found, callbackResultName: %s", __func__, aidlCallbackName.callbackResultName.c_str());
                    }
                    else
                    {
                        if (is_empty_string(aidlCallbackName.callbackName))
                        {
                            aidlCallbackName.callbackName = name;
                            // logd("%s: callbackName: %s", __func__, aidlCallbackName.callbackName.c_str());
                        }
                        else
                        {
                            aidlCallbackName.callback2Name = name;
                            // logd("%s: callback2Name: %s", __func__, aidlCallbackName.callback2Name.c_str());
                        }
                    }
                }
            }
        }
        return true;
    }
    return false;
}

string aidl_get_out_param_name(const string& paramName)
{
    stringstream ss;
    ss << AIDL_PARAM_OUT_PREFIX << paramName;
    return ss.str();
}
