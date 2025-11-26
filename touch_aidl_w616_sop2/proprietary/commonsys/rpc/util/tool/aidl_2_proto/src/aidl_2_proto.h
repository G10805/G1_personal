/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <string>

using std::string;

enum class MakefileType
{
    ANDROID_BP,
    LINUX_MAKEFILE,
};

typedef struct
{
    string aidlPath;
    string protoPath;
    bool genHalStatus;
    MakefileType makefileType;
    bool removeFile;
    string packageName;
} Aidl2ProtoParam;


int aidl_2_proto_main(const Aidl2ProtoParam& param);