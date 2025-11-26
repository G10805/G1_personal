/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef EXCLUDE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <iostream>

#include "aidl_2_proto.h"
#include "proto.h"
#include "proto_file.h"
#include "proto_util.h"

#include "log.h"
#include "util.h"

using std::ifstream;
using std::ofstream;


static void set_proto_file(const AidlFile *aidlFile, ProtoFile& protoFile)
{
    /* 1. fileName */
    protoFile.fileName = proto_get_file_name(aidlFile->fileName);

    /* 2. packageName */
    protoFile.packageName = proto_get_package_name(aidlFile->packageName);

    /* 3. fileType */
    protoFile.fileType = proto_get_file_type(aidlFile->fileType);
}

static void init_proto_files(const vector<AidlFile *>& aidlFile, vector<ProtoFile>& protoFile)
{
    vector<AidlFile>::size_type size = aidlFile.size();

    protoFile.resize(size);

    for (vector<AidlFile>::size_type index = 0; index < size; index++)
    {
        set_proto_file(aidlFile[index], protoFile[index]);
    }
}

/* ------------------------------------------------------------------------------------ */

bool proto_create_dir(const string& aidlPath, string& protoPath)
{
    return create_dir(aidlPath, PROTO_RELATIVE_PATH, protoPath);
}

void proto_init_info(const AidlInfo& aidlInfo, const string& protoPath, bool genHalStatus, ProtoInfo& protoInfo)
{
    protoInfo.protoPath = protoPath;
    logi("a2p: proto path: %s", protoPath.c_str());

    protoInfo.totalFileNumber = aidlInfo.totalFileNumber;
    logd("%s: total proto file number %d", __func__, aidlInfo.totalFileNumber);

    protoInfo.genHalStatus = genHalStatus;
    logd("%s: gen hal status: %x", __func__, genHalStatus);

    init_proto_files(aidlInfo.aidlFile, protoInfo.protoFile);
}

bool proto_create_files(const AidlInfo& aidlInfo)
{
    uint32_t totalFileNumber = aidlInfo.totalFileNumber;
    const ProtoInfo& protoInfo = proto_get_info();
    const Aidl2ProtoParam& param = proto_get_param();

    for (uint32_t index = 0; index < totalFileNumber; index++)
    {
        if (!proto_create_file(aidlInfo.aidlPath,
            index,
            aidlInfo.aidlFile[index],
            protoInfo.protoPath,
            protoInfo.protoFile[index],
            param.removeFile))
        {
            return false;
        }
    }
    logi("a2p: create %d proto file from aidl file", totalFileNumber);

    proto_create_hal_status_file(protoInfo.protoPath, param.removeFile);

    if (param.makefileType == MakefileType::ANDROID_BP)
    {
        if (proto_create_android_bp(protoInfo.protoPath, param.removeFile))
        {
            logi("a2p: create Android.bp");
            return true;
        }
        else
        {
            loge("%s: fail to create Android.bp", __func__);
            return false;
        }
    }
    else
    {
        if (proto_create_linux_makefile(protoInfo.protoPath, param.removeFile))
        {
            logi("a2p: create linux makefile");
            return true;
        }
        else
        {
            loge("%s: fail to linux makefile", __func__);
            return false;
        }
    }
}

void proto_read_files(ProtoInfo& protoInfo)
{
    uint32_t totalFileNumber = protoInfo.totalFileNumber;
    for (uint32_t index = 0; index < totalFileNumber; index++)
    {
        proto_read_file(protoInfo.protoPath, protoInfo.protoFile[index]);
    }
}