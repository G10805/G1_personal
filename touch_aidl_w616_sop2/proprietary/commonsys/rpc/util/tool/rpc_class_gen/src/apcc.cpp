/*
 * Copyright (c) Qualcomm Technologies, Inc.  and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "aidl.h"
#include "aidl_util.h"

#include "apcc.h"
#include "apcc_file.h"
#include "apcc_gen.h"

#include "proto.h"
#include "aidl_2_proto.h"

#include "log.h"
#include "util.h"

typedef struct
{
    bool initialized;
    ApccInfo apccInfo;
} ApccInstance;

static ApccInstance sApccInstance;

/* ------------------------------------------------------------------------------------ */

static bool apcc_proto_init(const ApccParam& param)
{
    Aidl2ProtoParam a2pParam;
    a2pParam.aidlPath = param.aidlPath;
    a2pParam.protoPath = param.protoPath;
    a2pParam.genHalStatus = true;

    if (!proto_init(a2pParam))
        return false;

    return true;
}

bool apcc_init(const ApccParam& param)
{
    ApccInstance *inst = &sApccInstance;

    if (!apcc_proto_init(param))
        return false;

    if (!create_dir(param.genPath))
        return false;

    const ProtoInfo& protoInfo = proto_read_files();
    logi("apcc: read %d proto file", protoInfo.totalFileNumber);

    apcc_init_gen_info();

    apcc_init_info(inst->apccInfo);

    inst->initialized = true;
    return true;
}

const ApccInfo& apcc_get_info()
{
    ApccInstance *inst = &sApccInstance;
    return inst->apccInfo;
}

ApccParam& apcc_get_param()
{
    ApccInstance *inst = &sApccInstance;
    return inst->apccInfo.param;
}

bool apcc_generate_files()
{
    ApccInstance *inst = &sApccInstance;

    if (!inst->initialized)
        return false;

    return apcc_create_files(inst->apccInfo);
}

void apcc_deinit()
{
    ApccInstance *inst = &sApccInstance;
    inst->initialized = false;
    proto_deinit();
}
