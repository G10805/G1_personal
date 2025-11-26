/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "aidl_util.h"
#include "proto_util.h"

bool proto_create_dir(const string& aidlPath, string& protoPath);

void proto_init_info(const AidlInfo& aidlInfo, const string& protoPath, bool genHalStatus, ProtoInfo& protoInfo);

bool proto_create_files(const AidlInfo& aidlInfo);

void proto_read_files(ProtoInfo& protoInfo);