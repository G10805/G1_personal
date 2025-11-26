/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "aidl_2_proto.h"
#include "log.h"
#include "proto.h"
#include "util.h"

int aidl_2_proto_main(const Aidl2ProtoParam& param)
{
    if (!proto_init(param))
        return -1;

    if (!proto_generate_files())
        return -1;

    proto_deinit();
    logd("a2p succeed");
    return 0;
}