/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "aidl_2_proto.h"
#include "proto_util.h"

bool proto_init(const Aidl2ProtoParam& param);

const ProtoInfo& proto_get_info();

const Aidl2ProtoParam& proto_get_param();

bool proto_generate_files();

void proto_deinit();

const ProtoInfo& proto_read_files();