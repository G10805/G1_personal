/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#ifndef EXCLUDE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <fstream>
#include <iostream>
#include <string>

#include "aidl.h"
#include "aidl_util.h"
#include "log.h"
#include "util.h"

using std::endl;
using std::ifstream;
using std::ofstream;

#ifndef EXCLUDE_EXPERIMENTAL_FILESYSTEM
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

#define IS_AIDL_ENUM(type)      ((type) == AIDL_TYPE_ENUM)

typedef struct
{
    bool initialized;
    AidlInfo aidlInfo;
} AidlInstance;

static AidlInstance sAidlInstance;

static EnumStatsInfo sEnumInfo;

#ifdef ENABLE_DUMP
static void dump_aidl_file(const AidlFile* aidlFile)
{
    if (aidlFile != NULL)
    {
        logd("%s: fileName: %s", __func__, aidlFile->fileName.c_str());
        logd("%s: packageName: %s", __func__, aidlFile->packageName.c_str());
        logd("%s: messageName: %s", __func__, aidlFile->messageName.c_str());
        logd("%s: fileType: %x", __func__, aidlFile->fileType);
        logd("%s: typeName: %s", __func__, aidlFile->typeName.c_str());
        dump_string("import file", aidlFile->importFile);
    }
}

static void dump_aidl_file(const vector<AidlFile*>& aidlFile)
{
    for (auto it: aidlFile)
        dump_aidl_file(it);
}
#endif

static bool is_aidl_file(const fs::path& p)
{
    return !p.extension().string().compare(AIDL_FILE_SUFFIX);
}

static bool calculate_aidl_file(string& path, vector<string>& files)
{
    fs::path aidlPath = path;

    files.clear();

    logd("%s: aidl path: %s", __func__, path.c_str());

    if (!fs::exists(aidlPath))
    {
        loge("%s: aidl path not exist, err: %d (%s)", __func__, errno, strerror(errno));
        return false;
    }

    if (fs::is_regular_file(aidlPath))
    {
        if (!is_aidl_file(aidlPath))
        {
            loge("%s: not aidl file, err: %d (%s)", __func__, errno, strerror(errno));
            return false;
        }
        /* store parent path */
        path = aidlPath.parent_path().string();
        string aidlFile = aidlPath.filename().string();
        logd("%s: aidl file: %s, aidl path: %s", __func__, aidlFile.c_str(), path.c_str());

        files.push_back(aidlFile);
    }
    else if (fs::is_directory(aidlPath))
    {
        for (const auto& entry : fs::directory_iterator(aidlPath))
        {
            // if (entry.is_regular_file())
            if (fs::is_regular_file(entry.path()))
            {
                /* aidl file */
                if (is_aidl_file(entry.path()))
                {
                    string fileName = entry.path().filename().string();
                    files.push_back(fileName);
                }
            }
        }
    }
    else
    {
        loge("%s: special file, not aidl file, err: %d (%s)", __func__, errno, strerror(errno));
        return false;
    }
    return true;
}

static bool aidl_retrieve_file_info(stringstream& ss, string& packageName, vector<string>& importFile, string& messageName, AidlType& fileType)
{
    string line, str;
    bool is_comment_begin_found = false;
    bool res = false;

    while (getline(ss, line))
    {
        trim(line);

        if (is_line_skipped(line, is_comment_begin_found))
            continue;

        if (aidl_get_package_name(line, str))
        {
            packageName = str;
        }
        else if (aidl_get_import_file(line, str))
        {
            importFile.push_back(str);
        }
        else if (aidl_get_file_type(line, fileType))
        {
            aidl_get_message_name_and_type(line, messageName);
            res = true;
            break;
        }
    }
    return res;
}

static bool is_enum_all_in_one_line(string& str)
{
    return contain_string(str, "{") &&
        contain_string(str, "}") &&
        contain_string(str, "enum");
}

static bool is_function_all_in_one_line(string& str)
{
    return contain_string(str, "(") &&
        contain_string(str, ")") &&
        has_terminate_char(str);
}

static bool is_parcelable_all_in_one_line(string& str)
{
    return has_terminate_char(str);
}

static void enum_pre_processing(stringstream& ss, string& str)
{
    char space_delim = ' ';
    char common_delim = ',';
    vector<string> results, enumFieldResults;

    if (split_string(str, space_delim, results))
    {
        for(vector<string>::size_type i = 0; i < results.size(); i++)
        {
            if (contain_string(results[i], AIDL_AT)||
                contain_string(results[i], "{") ||
                contain_string(results[i], "}"))
            {
                ss << results[i] << endl;
            }
            else if (contain_string(results[i], AIDL_ENUM))
            {
                /* combine message type and message name, e.g. enum NanPairingAkm */
                ss << results[i] << " " << results[i+1] << endl;
                i++;
            }
            else
            {
                string tmp_str;
                /* combine enum fileds from results to one string */
                while(!contain_string(results[i], "}") && i < results.size())
                {
                    tmp_str += " " + results[i];
                    i ++;
                }
                i--;

                /* split one string to multiple enum strings */
                split_string(tmp_str, common_delim, enumFieldResults);
                for(vector<string>::size_type j = 0; j < enumFieldResults.size(); j++)
                {
                    ss << enumFieldResults[j] << "," << endl;
                }
            }
        }
    }
}

static void common_pre_processing(stringstream& ss, string& str)
{
    char space_delim = ' ';
    vector<string> results;
    if (split_string(str, space_delim, results))
    {
        vector<string>::size_type size = results.size();
        for (vector<string>::size_type i = 0; i < size; i++)
        {
            if (contain_string(results[i], AIDL_AT))
            {
                ss << results[i] << endl;
            }
            else
            {
                string tmp_str;
                /* combine results to function in one string */
                while (!contain_string(results[i], "}"))
                {
                    tmp_str += " " + results[i];
                    if (++i >= size)
                        break;
                }
                ss << tmp_str << endl;
            }
        }
    }
}

static void aidl_file_pre_processing(ifstream& fs, stringstream& ss)
{
    string line;
    string line_pending_process;
    bool is_comment_begin_found = false;
    while (getline(fs, line))
    {
        trim(line);

        if (is_line_skipped(line, is_comment_begin_found))
            continue;

        if (contain_string(line, "/*") && contain_string(line, "*/"))
        {
            // e.g.
            // before: byte[/* 6 */] bssid;
            // after: byte[] bssid;
            remove_comment_in_line(line);
        }

        if (!contain_string(line, AIDL_AT))
        {
            ss << line << endl;
            continue;
        }

        if (is_enum_all_in_one_line(line))
        {
            enum_pre_processing(ss, line);
        }
        else if (is_function_all_in_one_line(line) ||
            is_parcelable_all_in_one_line(line))
        {
            logd("%s: line: %s", __func__, line.c_str());
            common_pre_processing(ss, line);
        }
        else
        {
            ss << line << endl;
        }
    }
}

static void update_enum_class_info()
{
    EnumStatsInfo* enumInfo = aidl_get_enum_stats();
    map<string, string>::iterator iter;
    string str;
    char delim = '#';
    vector<string> results;
    for(iter = enumInfo->enumFieldName.begin(); iter != enumInfo->enumFieldName.end(); iter++)
    {
        string str = iter->second;
        if (split_string(str, delim, results))
        {
            int index = 0;
            int size = results.size();
            /* no duplicate enums */
            if (size < 2)
            {
                continue;
            }
            while (index < size)
            {
                /* e.g.
                 *elem0: <RttStatus, true>
                 *elem1: <NanStatusCode, true>
                 *elem2: <WifiDebugRxPacketFate, true>
                 */
                enumInfo->enumClassName[results[index]] = true;
                index ++;
            }
        }
    }
}

static void update_enum_field_info(string& messageName, string& enumKey)
{
    EnumStatsInfo* enumInfo = aidl_get_enum_stats();
    string messageNameSplit = "#";
    map<string, string>::iterator iter;
    iter = enumInfo->enumFieldName.find(enumKey);
    if (iter != enumInfo->enumFieldName.end())
    {
        /* e.g
         * elem0: <SUCCESS, RttStatus#NanStatusCode#WifiDebugRxPacketFate>
         */
        enumInfo->enumFieldName[enumKey] += messageNameSplit + messageName;
    }
    else
    {
        enumInfo->enumFieldName[enumKey] = messageName;
    }
}

static void retrieve_enum_stats(stringstream& ss, string& messageName)
{
    string line, enumKey;
    bool messageBody = false;
    while (getline(ss, line))
    {
        if (contain_string(line, AIDL_ENUM))
        {
            if (!aidl_get_message_name_and_type(line, messageName))
            {
                logw("%s message name not retrieved", __func__);
                break;
            }
            messageBody = true;
        }
        else if (contain_string(line, "{"))
        {
            continue;
        }
        else if (messageBody)
        {
            if (contain_string(line, "}"))
            {
                break;
            }
            if (!aidl_get_enum_key(line, enumKey))
            {
                continue;
            }
            update_enum_field_info(messageName, enumKey);
        }
    }
}

static void retrieve_intf_info(AidlFile* &aidlFile)
{
    AidlInterfaceFile *intfFile = reinterpret_cast<AidlInterfaceFile *>(aidlFile);
    stringstream& ss = aidlFile->ss;
    vector<AidlInterface>& intfList = intfFile->intf;
    AidlInterface intf;
    string line;
    string line_stored;

    // logd("%s: ss: %s", __func__, ss.str().c_str());

    while (getline(ss, line))
    {
        trim(line);

        if (contain_string(line, AIDL_PACKAGE) ||
            contain_string(line, AIDL_IMPORT) ||
            contain_string(line, AIDL_INTERFACE) ||
            contain_string(line, AIDL_ENUM) ||
            contain_string(line, AIDL_PARCELABLE) ||
            contain_string(line, AIDL_UNION))
        {
            /* ignore */
            continue;
        }

        if (aidl_is_at_line(line) &&
            !contain_char(line, ' '))
        {
            /* ignore @ line */
            logd("%s: ignore line: %s", __func__, line.c_str());
            continue;
        }

        /* remove comment in line */
        remove_comment(line);

        if (valid_str(line_stored))
        {
            // logd("%s: + line: %s", __func__, line.c_str());
            line = line_stored + line;
            // logd("%s: - new line: %s", __func__, line.c_str());
        }

        if (aidl_get_interface(line, intf))
        {
            // logd("%s: add intf: %s", __func__, line.c_str());
            intfList.push_back(intf);
            line_stored.clear();
        }
        else
        {
            if (contain_char(line, '('))
            {
                /* store then concat with next line */
                line_stored = line;
                // logd("%s: line_stored: %s", __func__, line_stored.c_str());
            }
        }
    }
}

static bool set_aidl_file(const string& aidlPath, const string& fileName, AidlFile* &aidlFile)
{
    bool res = false;
    ifstream fs;
    string line, packageName;
    vector<string> importFile;
    string messageName;
    AidlType fileType;
    string fullFileName = get_full_file_name(aidlPath, fileName);
    stringstream ss;

    if (!open_file(fullFileName, fs))
        return false;

    aidl_file_pre_processing(fs, ss);
    fs.close();

    res = aidl_retrieve_file_info(ss, packageName, importFile, messageName, fileType);
    if (res)
    {
        aidlFile = aidl_new_file(fileName, packageName, importFile, messageName, fileType);
        aidlFile->ss.str(ss.str());

        switch (aidlFile->fileType)
        {
            case AIDL_TYPE_INTERFACE:
                /* fall-through */
            case AIDL_TYPE_CALLBACK:
            {
                retrieve_intf_info(aidlFile);
                break;
            }
            case AIDL_TYPE_ENUM:
            {
                retrieve_enum_stats(aidlFile->ss, aidlFile->messageName);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {
        loge("%s fail to retrieve aidl file info", __func__);
        return false;
    }
    return res;
}

static string get_package_name(string& packageName, string& messageName)
{
    stringstream ss;
    ss << packageName << '.' << messageName;
    return ss.str();
}

static void update_aidl_file(AidlFile *aidlFile)
{
    if (aidlFile->fileType != AIDL_TYPE_ENUM)
    {
        return;
    }

    string messageName = aidlFile->messageName;
    EnumStatsInfo *enumStats = aidl_get_enum_stats();
    map<string, bool>::iterator iter = enumStats->enumClassName.find(messageName);
    if (iter != enumStats->enumClassName.end())
    {
        if (iter->second)
        {
            string str = get_string_upper_letters(messageName);
            transform_string_to_lower(str);
            aidlFile->packageName = get_package_name(aidlFile->packageName, str);
        }
    }
}

static bool init_aidl_files(AidlInfo& aidlInfo, vector<string> files)
{
    vector<string>::size_type size = files.size();

    aidlInfo.aidlFile.resize(size);

    for (vector<string>::size_type index = 0; index < size; index++)
    {
        if (!set_aidl_file(aidlInfo.aidlPath, files[index], aidlInfo.aidlFile[index]))
        {
            loge("%s: fail to set aidl file", __func__);
            return false;
        }
    }

    update_enum_class_info();

    for (vector<string>::size_type index = 0; index < size; index++)
    {
        update_aidl_file(aidlInfo.aidlFile[index]);
    }
    return true;
}

static uint32_t get_aidl_intf_num(const vector<AidlFile*>& aidlFile)
{
    uint32_t aidl_intf_num = 0;
    for (auto it: aidlFile)
    {
        if (it->fileType == AIDL_TYPE_INTERFACE)
            ++aidl_intf_num;
    }
    return aidl_intf_num;
}

static void get_aidl_enum_type(const vector<AidlFile*>& aidlFile, vector<string>& enumType)
{
    for (auto it: aidlFile)
    {
        if (it->fileType == AIDL_TYPE_ENUM)
            enumType.push_back(it->typeName);
    }    
}

static string get_aidl_intf_root(const vector<AidlFile*>& aidlFile)
{
    vector<AidlFile*> aidlIntf;
    for (auto it: aidlFile)
    {
        if (it->fileType == AIDL_TYPE_INTERFACE)
            aidlIntf.push_back(it);
    }
    if (aidlIntf.empty())
    {
        return "";
    }
    else if (aidlIntf.size() == 1)
    {
        return aidlIntf[0]->typeName;
    }
    else
    {
        for (auto it: aidlIntf)
        {
            const string& typeName = it->typeName;
            bool existIntfInImport = false;
            for (auto it2: aidlIntf)
            {
                if (!is_same_string(typeName, it2->typeName))
                {
                    for (auto importFile: it2->importFile)
                    {
                        if (contain_suffix(importFile, typeName))
                        {
                            existIntfInImport = true;
                            break;
                        }
                    }
                }
                if (existIntfInImport)
                    break;
            }
            if (!existIntfInImport)
            {
                /* found */
                logd("%s: found aidl intf root: %s", __func__, typeName.c_str());
                return typeName;
            }
        }
        logw("%s: not foud aidl intf root", __func__);
        return "";
    }
}

static string get_hal_status_type(const string& aidlIntfRoot, const vector<AidlFile*>& aidlFile)
{
    const string halRoot = aidl_get_interface_short_name(aidlIntfRoot);
    const string statusType = "Status";
    const string halStatusType = concat_string(halRoot, statusType);
    const string halStatusCodeType = concat_string(halStatusType, "Code");

    for (auto it: aidlFile)
    {
        if (it->fileType == AIDL_TYPE_ENUM)
        {
            if (is_same_string(it->typeName, halStatusCodeType) ||
                is_same_string(it->typeName, halStatusType) ||
                is_same_string(it->typeName, statusType))
                return it->typeName;
        }
    }
    return "";
}

static bool init_aidl_info(const string& path, AidlInfo& aidlInfo)
{
    vector<string> files;

    aidlInfo.aidlPath = path;

    logi("a2p: aidl path:  %s", aidlInfo.aidlPath.c_str());

    if (!calculate_aidl_file(aidlInfo.aidlPath, files))
        return false;

    if (files.empty())
    {
        loge("%s: none aidl file found", __func__);
        return false;
    }

    aidlInfo.totalFileNumber = (uint32_t) files.size();
    logi("a2p: found %d aidl file", aidlInfo.totalFileNumber);

    if (!init_aidl_files(aidlInfo, files))
    {
        loge("%s: fail to init aidl files", __func__);
        return false;
    }

    aidlInfo.aidlIntfNum = get_aidl_intf_num(aidlInfo.aidlFile);
    logd("%s: aidl intf num: %d", __func__, aidlInfo.aidlIntfNum);
    get_aidl_enum_type(aidlInfo.aidlFile, aidlInfo.enumType);
    logd("%s: aidl enum type num: %d", __func__, aidlInfo.enumType.size());
    aidlInfo.aidlIntfRoot = get_aidl_intf_root(aidlInfo.aidlFile);
    logd("%s: aidlIntfRoot: %s", __func__, aidlInfo.aidlIntfRoot.c_str());
    aidlInfo.halStatusType = get_hal_status_type(aidlInfo.aidlIntfRoot, aidlInfo.aidlFile);
    logd("%s: halStatusType: %s", __func__, aidlInfo.halStatusType.c_str());
    return true;
}

/* ------------------------------------------------------------------------------------ */

bool aidl_init(const string& aidlPath)
{
    AidlInstance *inst = &sAidlInstance;
    OUTPUT_FUNC();
    if (inst->initialized)
        return true;

    inst->initialized = init_aidl_info(aidlPath, inst->aidlInfo);
    return inst->initialized;
}

const AidlInfo& aidl_get_info()
{
    AidlInstance *inst = &sAidlInstance;
    return inst->aidlInfo;
}

EnumStatsInfo* aidl_get_enum_stats()
{
    EnumStatsInfo *inst = &sEnumInfo;
    return inst;
}

string aidl_get_package_from_message_name(const string& str)
{
    const AidlInfo& aidlInfo = aidl_get_info();
    vector<string>::size_type size = aidlInfo.aidlFile.size();
    for (vector<string>::size_type index = 0; index < size; index++)
    {
        if (!aidlInfo.aidlFile[index]->messageName.compare(str))
        {
            return aidlInfo.aidlFile[index]->packageName;
        }
    }
    return "";
}

void aidl_deinit()
{
    AidlInstance *inst = &sAidlInstance;
    OUTPUT_FUNC();
    for (vector<AidlFile*>::size_type index = 0; index < inst->aidlInfo.aidlFile.size(); index++)
    {
        delete inst->aidlInfo.aidlFile[index];
    }
    inst->aidlInfo.aidlFile.clear();
    inst->initialized = false;
}
