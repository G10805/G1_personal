/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#include "aidl.h"
#include "aidl_2_proto.h"
#include "proto.h"
#include "proto_file.h"
#include "proto_util.h"
#include "util.h"

typedef struct
{
    bool initialized;
    ProtoInfo protoInfo;
    Aidl2ProtoParam param;
} ProtoInstance;

static ProtoInstance sProtoInstance;

bool proto_init(const Aidl2ProtoParam& param)
{
    ProtoInstance *inst = &sProtoInstance;

    inst->param = param;

    if (inst->initialized)
        return true;

    if (!aidl_init(inst->param.aidlPath))
        return false;

    if (!create_dir(inst->param.protoPath))
        return false;

    proto_init_info(aidl_get_info(), inst->param.protoPath, inst->param.genHalStatus, inst->protoInfo);

    proto_util_init(inst->param.protoPath);

    inst->initialized = true;
    return true;
}

const ProtoInfo& proto_get_info()
{
    ProtoInstance *inst = &sProtoInstance;
    return inst->protoInfo;
}

const Aidl2ProtoParam& proto_get_param()
{
    ProtoInstance *inst = &sProtoInstance;
    return inst->param;
}

bool proto_generate_files()
{
    ProtoInstance *inst = &sProtoInstance;

    if (!inst->initialized)
        return false;

    return proto_create_files(aidl_get_info());
}

void proto_deinit()
{
    ProtoInstance *inst = &sProtoInstance;
    inst->initialized = false;
    proto_util_deinit();
    aidl_deinit();
}

const ProtoInfo& proto_read_files()
{
    ProtoInstance *inst = &sProtoInstance;
    proto_read_files(inst->protoInfo);
    return inst->protoInfo;
}
