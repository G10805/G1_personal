/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "apcc_main.h"
#include "log.h"

int apcc_main(const ApccParam& param)
{
    logd("apcc init");
    if (!apcc_init(param))
        return -1;

    logd("apcc generate files");
    if (!apcc_generate_files())
        return -1;

    apcc_deinit();
    logi("apcc done");
    return 0;
}
