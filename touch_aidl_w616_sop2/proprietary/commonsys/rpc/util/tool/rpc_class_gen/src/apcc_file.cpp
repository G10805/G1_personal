/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "aidl.h"
#include "apcc_file.h"
#include "apcc_gen.h"
#include "proto_util.h"

#include "log.h"
#include "util.h"

// #define TEST_RPC

#ifdef TEST_RPC
#define TEST_ONLY
#ifdef TEST_ONLY
#define TEST_AIDL_INTF      "IBroadcastRadio"
#define TEST_SINGLE_FILE
#endif
#endif

static uint32_t get_apcc_file_number(const vector<ApccFile>& apccFile)
{
    uint32_t file_number = 0;
    for (auto it: apccFile)
    {
        if (it.gen)
            ++file_number;
    }
    return file_number;
}

static uint32_t get_apcc_file_number(const ApccInfo& apccInfo)
{
    uint32_t file_number = 0;
    for (uint32_t intfIndex = 0; intfIndex < apccInfo.aidlIntfNumber; intfIndex++)
    {
        file_number += get_apcc_file_number(apccInfo.srcFile[intfIndex]);
    }
    return file_number;
}

static void set_apcc_file(ApccType type, const string& name, const string& path, bool gen, ApccFile& apccFile)
{
    // logd("%s: file {type: %d, name: %s, path: %s}, gen: %x", __func__, type, name.c_str(), path.c_str(), gen);
    /* 1. file type */
    apccFile.type = type;
    /* 2. file name */
    apccFile.name = name;
    /* 3. file path */
    apccFile.path = path;
    /* 4. set flag for code generation or not */
    apccFile.gen = gen;
}

static void set_apcc_file(const string& genPath, const string& aidlIntfName, vector<ApccFile>& apccFile)
{
    vector<AidlFile>::size_type size = APCC_TYPE_COUNT;

    apccFile.resize(size);

    for (vector<AidlFile>::size_type index = 0; index < size; index++)
    {
        ApccType type = APCC_TYPE_BASE + index;
        string name = apcc_get_gen_file_name(aidlIntfName, type);
        string relativePath = apcc_get_gen_folder(type);
        string fullPath;
        bool gen = apcc_get_gen_flag(aidlIntfName, type);
        if (gen)
        {
            create_dir(genPath, relativePath);
            fullPath = get_path(genPath, relativePath);
        }
        else
        {
            fullPath = "";
        }
        set_apcc_file(type, name, fullPath, gen, apccFile[index]);
    }
}

static void init_apcc_files(ApccInfo& apccInfo, vector<vector<ApccFile>>& apccFiles)
{
    vector<vector<AidlFile>>::size_type size = apccInfo.aidlIntfName.size();
    apccFiles.resize(size);

    for (vector<vector<AidlFile>>::size_type index = 0; index < size; index++)
    {
        set_apcc_file(apccInfo.param.genPath, apccInfo.aidlIntfName[index], apccFiles[index]);
    }
}

static void dump_aidl_intf_name(const vector<string>& aidlIntfName)
{
    vector<string>::size_type size = aidlIntfName.size();
    logd("%s: total aidl intf number %d", __func__, size);
    for (vector<string>::size_type index = 0; index < size; index++)
    {
        logd("%s: aidl intf (%d/%d) name: %s", __func__, index + 1, size, aidlIntfName[index].c_str());
    }
}

/* ------------------------------------------------------------------------------------ */

void apcc_init_info(ApccInfo& apccInfo)
{
    /* get aidl interface name */
    aidl_get_type_name(aidl_get_info(), AIDL_TYPE_INTERFACE, apccInfo.aidlIntfName);

    dump_aidl_intf_name(apccInfo.aidlIntfName);

    apccInfo.aidlIntfNumber = apccInfo.aidlIntfName.size();
    logd("%s: apccInfo.aidlIntfNumber %d", __func__, apccInfo.aidlIntfNumber);

    init_apcc_files(apccInfo, apccInfo.srcFile);
}

bool apcc_create_files(const ApccInfo& apccInfo)
{
    uint32_t totalFile = 0;
    uint32_t aidlIntfNumber = apccInfo.aidlIntfNumber;
    OUTPUT_FUNC();
    for (uint32_t intfIndex = 0; intfIndex < aidlIntfNumber; intfIndex++)
    {
        AidlCallbackName aidlCallbackName;
        const string& aidlIntfName = apccInfo.aidlIntfName[intfIndex];
        const vector<ApccFile>& apccFile = apccInfo.srcFile[intfIndex];
        vector<ApccFile>::size_type fileNumber = apccFile.size();

        aidl_get_callback_name(aidlIntfName, aidlCallbackName);

#ifdef TEST_ONLY
        if (!is_same_string(aidlIntfName, TEST_AIDL_INTF))
        {
            /* FIXME: Test only */
            continue;
        }
#endif

        logd("%s: aidl intf: %s (%d/%d), callback: %s, file number: %d", __func__,
            aidlIntfName.c_str(), intfIndex + 1, aidlIntfNumber, aidlCallbackName.callbackName.c_str(),
            get_apcc_file_number(apccFile));

        for (vector<ApccFile>::size_type fileIndex = 0; fileIndex < fileNumber; fileIndex++)
        {
            // logd("%s: file: %s (%d/%d)", __func__, apccFile[fileIndex].name.c_str(), fileIndex + 1, fileNumber);

#ifdef TEST_SINGLE_FILE
            if (fileIndex != APCC_TYPE_HAL_IMPL_SOURCE)
            {
                continue;
            }
#endif

            if (!apcc_create_file(aidlIntfName, aidlCallbackName, apccFile[fileIndex]))
                return false;

            if (apccFile[fileIndex].gen)
            {
                ++totalFile;
            }

#ifdef TEST_SINGLE_FILE
            /* FIXME: Test only */
            loge("%s: file: %s, test break", __func__, apccFile[fileIndex].name.c_str());
#endif
        }
    }
    logi("apcc: generate file %d/%d", totalFile, get_apcc_file_number(apccInfo));
    return true;
}
